#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/node.h", []() {
    describe("node", [&]() {
      const ptr_uint64 terminal_F = ptr_uint64(false);
      const ptr_uint64 terminal_T = ptr_uint64(true);

      it("should be a POD", [&]() {
        AssertThat(std::is_pod<node>::value, Is().True());
      });

      it("should take up 24 bytes of memory", [&]() {
        const ptr_uint64 node_ptr = ptr_uint64(42,2);
        const node n = node(1u,8u, node_ptr, terminal_F);

        AssertThat(sizeof(n), Is().EqualTo(3u * 8u));
      });

      describe("node(...), .label(), .id(), .low(), .high()", [&]() {
        it("should create node [label_type, id_type, ptr_uint64, ptr_uint64] [1]", [&]() {
          const node n = node(3u,12u, terminal_F, terminal_T);
          AssertThat(n.uid(), Is().EqualTo(ptr_uint64(3,12)));
          AssertThat(n.label(), Is().EqualTo(3u));
          AssertThat(n.id(), Is().EqualTo(12u));

          AssertThat(n.child(false), Is().EqualTo(n.low()));
          AssertThat(n.low(), Is().EqualTo(terminal_F));
          AssertThat(n.child(true), Is().EqualTo(n.high()));
          AssertThat(n.high(), Is().EqualTo(terminal_T));
        });

        it("should create node [label_type, id_type, ptr_uint64, ptr_uint64] [2]", [&]() {
          const node n = node(3u,42u, terminal_T, terminal_F);
          AssertThat(n.uid(), Is().EqualTo(ptr_uint64(3,42)));
          AssertThat(n.label(), Is().EqualTo(3u));
          AssertThat(n.id(), Is().EqualTo(42u));

          AssertThat(n.child(false), Is().EqualTo(n.low()));
          AssertThat(n.low(), Is().EqualTo(terminal_T));
          AssertThat(n.child(true), Is().EqualTo(n.high()));
          AssertThat(n.high(), Is().EqualTo(terminal_F));
        });

        it("should create node [label_type, id_type, node&, node&]", [&]() {
          const node n_child1 = node(3u,12u, terminal_F, terminal_T);
          const node n_child2 = node(3u,42u, terminal_T, terminal_F);

          const node n = node(2,2, n_child1, n_child2);
          AssertThat(n.uid(), Is().EqualTo(ptr_uint64(2,2)));
          AssertThat(n.label(), Is().EqualTo(2u));
          AssertThat(n.id(), Is().EqualTo(2u));

          AssertThat(n.child(false), Is().EqualTo(n.low()));
          AssertThat(n.low(), Is().EqualTo(n_child1.uid()));
          AssertThat(n.child(true), Is().EqualTo(n.high()));
          AssertThat(n.high(), Is().EqualTo(n_child2.uid()));
        });

        it("should create node [label_type, id_type, node&, ptr_uint64]", [&]() {
          const node n_child = node(2u,2u, terminal_F, terminal_T);

          const node n = node(1u,7u,terminal_T,n_child);
          AssertThat(n.uid(), Is().EqualTo(ptr_uint64(1,7)));
          AssertThat(n.label(), Is().EqualTo(1u));
          AssertThat(n.id(), Is().EqualTo(7u));

          AssertThat(n.child(false), Is().EqualTo(n.low()));
          AssertThat(n.low(), Is().EqualTo(terminal_T));
          AssertThat(n.child(true), Is().EqualTo(n.high()));
          AssertThat(n.high(), Is().EqualTo(n_child.uid()));
        });

        it("should create node [label_type, id_type, ptr_uint64, node&]", [&]() {
          const node n_child = node(2u,2u, terminal_F,terminal_T);

          const node n = node(0u,3u, terminal_T,n_child);
          AssertThat(n.uid(), Is().EqualTo(ptr_uint64(0,3)));
          AssertThat(n.label(), Is().EqualTo(0u));
          AssertThat(n.id(), Is().EqualTo(3u));

          AssertThat(n.child(false), Is().EqualTo(n.low()));
          AssertThat(n.low(), Is().EqualTo(terminal_T));
          AssertThat(n.child(true), Is().EqualTo(n.high()));
          AssertThat(n.high(), Is().EqualTo(n_child.uid()));
        });
      });

      describe("terminal nodes", [&]() {
        const node terminal_node_T = node(true);
        const node terminal_node_F = node(false);

        it("has nil() children [F]", [&]() {
          AssertThat(terminal_node_F.low(), Is().EqualTo(node::pointer_type::nil()));
          AssertThat(terminal_node_F.high(), Is().EqualTo(node::pointer_type::nil()));
        });

        it("has nil() children [T]", [&]() {
          AssertThat(terminal_node_T.low(), Is().EqualTo(node::pointer_type::nil()));
          AssertThat(terminal_node_T.high(), Is().EqualTo(node::pointer_type::nil()));
        });

        describe("is_terminal", [&]() {
          it("accepts terminal [F]", [&]() {
            AssertThat(terminal_node_F.is_terminal(), Is().True());
          });

          it("accepts terminal [T]", [&]() {
            AssertThat(terminal_node_T.is_terminal(), Is().True());
          });

          it("rejects non-terminal nodes [1]", [&]() {
            const node node_1 = node(42u,2u, terminal_F, terminal_T);
            AssertThat(node_1.is_terminal(), Is().False());
          });

          it("rejects non-terminal nodes [2]", [&]() {
            const node almost_F_terminal = node(0u,0u, terminal_T, ptr_uint64(42,2));
            AssertThat(almost_F_terminal.is_terminal(), Is().False());
          });

          it("rejects non-terminal nodes [3]", [&]() {
            const node almost_T_terminal = node(0u,1u, terminal_T, ptr_uint64(42,2));
            AssertThat(almost_T_terminal.is_terminal(), Is().False());
          });
        });

        describe("value_of", [&]() {
          it("retrieves value of terminal node [T]", [&]() {
            AssertThat(terminal_node_T.value(), Is().True());
          });

          it("retrieves value of a terminal node [F]", [&]() {
            AssertThat(terminal_node_F.value(), Is().False());
          });
        });

        describe("is_false", [&]() {
          it("accepts terminal [F]", [&]() {
            AssertThat(terminal_node_F.is_false(), Is().True());
          });

          it("rejects terminal [T]", [&]() {
            AssertThat(terminal_node_T.is_false(), Is().False());
          });

          it("rejects non-terminal nodes", [&]() {
            const node n = node(0,0, ptr_uint64(42,2), terminal_F);
            AssertThat(n.is_false(), Is().False());
          });
        });

        describe("is_true", [&]() {
          it("rejects terminal [F]", [&]() {
            AssertThat(terminal_node_F.is_true(), Is().False());
          });

          it("accepts terminal [T]", [&]() {
            AssertThat(terminal_node_T.is_true(), Is().True());
          });

          it("rejects non-terminal nodes", [&]() {
            const node n = node(0,1, terminal_T, ptr_uint64(2,3));
            AssertThat(n.is_true(), Is().False());
          });
        });
      });

      describe("comparators [node]", [&]() {
        it("should primarily sort by label", [&]() {
          const node node_1 = node(1u,2u, terminal_F, terminal_T);
          const node node_2 = node(2u,1u, terminal_T, terminal_F);

          AssertThat(node_1, Is().LessThan(node_2));
          AssertThat(node_1, Is().LessThanOrEqualTo(node_2));
          AssertThat(node_2, Is().GreaterThan(node_1));
          AssertThat(node_2, Is().GreaterThanOrEqualTo(node_1));
        });

        it("should secondly sort by id", [&]() {
          const node node_1 = node(2u,1u, terminal_F, terminal_T);
          const node node_2 = node(2u,2u, terminal_T, terminal_F);

          AssertThat(node_1, Is().LessThan(node_2));
          AssertThat(node_2, Is().GreaterThan(node_1));
          AssertThat(node_1, Is().LessThanOrEqualTo(node_2));
          AssertThat(node_2, Is().GreaterThanOrEqualTo(node_1));
        });

        it("should thirdly sort by children [1]", [&]() {
          const node node_1 = node(2u,0u, terminal_F, terminal_T);
          const node node_2 = node(2u,0u, terminal_T, terminal_F);

          AssertThat(node_1, Is().LessThan(node_2));
          AssertThat(node_2, Is().GreaterThan(node_1));
          AssertThat(node_1, Is().LessThanOrEqualTo(node_2));
          AssertThat(node_2, Is().GreaterThanOrEqualTo(node_1));
        });

        it("should thirdly sort by children [2]", [&]() {
          const node node_1 = node(2u,0u, terminal_T, terminal_F);
          const node node_2 = node(2u,0u, terminal_T, terminal_T);

          AssertThat(node_1, Is().LessThan(node_2));
          AssertThat(node_2, Is().GreaterThan(node_1));
          AssertThat(node_1, Is().LessThanOrEqualTo(node_2));
          AssertThat(node_2, Is().GreaterThanOrEqualTo(node_1));
        });

        it("should (not) provide an ordering when equal", [&]() {
          const node node_1 = node(2u,1u, terminal_F, terminal_T);
          const node node_2 = node(2u,1u, terminal_F, terminal_T);

          AssertThat(node_1, Is().Not().LessThan(node_2));
          AssertThat(node_2, Is().Not().GreaterThan(node_1));

          AssertThat(node_1, Is().LessThanOrEqualTo(node_2));
          AssertThat(node_2, Is().GreaterThanOrEqualTo(node_1));
        });

        it("should be equal by if uid, low, and high agree [1]", [&]() {
          const node node_1_v1 = node(42u,2u, terminal_F, terminal_T);
          const node node_1_v2 = node(42u,2u, terminal_F, terminal_T);

          AssertThat(node_1_v1, Is().EqualTo(node_1_v2));
          AssertThat(node_1_v1 != node_1_v2, Is().False());
        });

        it("should be equal by if uid, low, and high agree [2]", [&]() {
          const node node_1_v1 = node(0u,0u, terminal_F, terminal_F);
          const node node_1_v2 = node(0u,0u, terminal_F, terminal_F);

          AssertThat(node_1_v1, Is().EqualTo(node_1_v2));
          AssertThat(node_1_v1 != node_1_v2, Is().False());
        });

        it("should be unequal if uid mismatches  [1]", [&]() {
          const node node_1 = node(42u,2u, terminal_F, terminal_T);
          const node node_2 = node(42u,1u, terminal_F, terminal_T);

          AssertThat(node_1, Is().Not().EqualTo(node_2));
          AssertThat(node_1 != node_2, Is().True());
        });

        it("should be unequal if uid mismatches  [2]", [&]() {
          const node node_1 = node(42u,2u, terminal_F, terminal_T);
          const node node_2 = node(21u,2u, terminal_F, terminal_T);

          AssertThat(node_1, Is().Not().EqualTo(node_2));
          AssertThat(node_1 != node_2, Is().True());
        });

        it("should be unequal if uid mismatches  [3]", [&]() {
          const node node_1 = node(42u,2u, terminal_F, terminal_T);
          const node node_2 = node(21u,8u, terminal_F, terminal_T);

          AssertThat(node_1, Is().Not().EqualTo(node_2));
          AssertThat(node_1 != node_2, Is().True());
        });

        it("should be unequal if low mismatches  [1]", [&]() {
          const node node_1 = node(42u,2u, terminal_F, terminal_T);
          const node node_2 = node(42u,2u, terminal_T, terminal_T);

          AssertThat(node_1 == node_2, Is().False());
          AssertThat(node_1 != node_2, Is().True());
        });

        it("should be unequal if high mismatches [1]", [&]() {
          const node node_1 = node(42u,2u, terminal_F, terminal_T);
          const node node_2 = node(42u,2u, terminal_F, terminal_F);

          AssertThat(node_1 == node_2, Is().False());
          AssertThat(node_1 != node_2, Is().True());
        });
      });

      describe("comparators [uid]", [&]() {
        it("should primarily sort by label [ 1]", [&]() {
          const node n = node(0u, 1u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(1u,0u);

          AssertThat(n, Is().LessThan(u));
          AssertThat(n, Is().LessThanOrEqualTo(u));
          AssertThat(u, Is().GreaterThan(n));
          AssertThat(u, Is().GreaterThanOrEqualTo(n));
        });

        it("should primarily sort by label [!1]", [&]() {
          const node n = node(0u, 1u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(1u,0u);

          AssertThat(u, Is().Not().LessThan(n));
          AssertThat(u, Is().Not().LessThanOrEqualTo(n));
          AssertThat(n, Is().Not().GreaterThan(u));
          AssertThat(n, Is().Not().GreaterThanOrEqualTo(u));
        });

        it("should primarily sort by label [ 2]", [&]() {
          const node n = node(21u, 8u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(42u,2u);

          AssertThat(n, Is().LessThan(u));
          AssertThat(n, Is().LessThanOrEqualTo(u));
          AssertThat(u, Is().GreaterThan(n));
          AssertThat(u, Is().GreaterThanOrEqualTo(n));
        });

        it("should primarily sort by label [ 3]", [&]() {
          const node n = node(1u, 1u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(0u,2u);

          AssertThat(u, Is().LessThan(n));
          AssertThat(u, Is().LessThanOrEqualTo(n));
          AssertThat(n, Is().GreaterThan(u));
          AssertThat(n, Is().GreaterThanOrEqualTo(u));
        });

        it("should primarily sort by label [ 4]", [&]() {
          const node n = node(42u, 0u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(21u,8u);

          AssertThat(u, Is().LessThan(n));
          AssertThat(u, Is().LessThanOrEqualTo(n));
          AssertThat(n, Is().GreaterThan(u));
          AssertThat(n, Is().GreaterThanOrEqualTo(u));
        });

        it("should primarily sort by label [!4]", [&]() {
          const node n = node(42u, 0u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(21u,8u);

          AssertThat(n, Is().Not().LessThan(u));
          AssertThat(n, Is().Not().LessThanOrEqualTo(u));
          AssertThat(u, Is().Not().GreaterThan(n));
          AssertThat(u, Is().Not().GreaterThanOrEqualTo(n));
        });

        it("should secondly sort by id     [ 1]", [&]() {
          const node n = node(42u, 0u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(42u, 1u);

          AssertThat(n, Is().LessThan(u));
          AssertThat(n, Is().LessThanOrEqualTo(u));
          AssertThat(u, Is().GreaterThan(n));
          AssertThat(u, Is().GreaterThanOrEqualTo(n));
        });

        it("should secondly sort by id     [ 2]", [&]() {
          const node n = node(42u, 1u, terminal_F, terminal_T);
          const node::uid_type u = node::uid_type(42u, 0u);

          AssertThat(u, Is().LessThan(n));
          AssertThat(u, Is().LessThanOrEqualTo(n));
          AssertThat(n, Is().GreaterThan(u));
          AssertThat(n, Is().GreaterThanOrEqualTo(u));
        });
      });

      describe("not ( ~ )", [&]() {
        it("should leave node_ptr children unchanged", [&]() {
          const node n = node(2u,2u, ptr_uint64(42,3), ptr_uint64(8,2));

          AssertThat(!n, Is().EqualTo(n));
        });

        it("should negate terminal_ptr child", [&]() {
          const node n = node(2u,2u, terminal_F, ptr_uint64(8,2));

          AssertThat(!n, Is().EqualTo(node(2,2, terminal_T, ptr_uint64(8,2))));
        });

        it("should negate terminal_ptr children while preserving flags", [&]() {
          const node n = node(2u,2u, terminal_F, flag(terminal_T));

          AssertThat(!n, Is().EqualTo(node(2,2, terminal_T, flag(terminal_F))));
        });

        it("should negate 'false' terminal node", [&]() {
          AssertThat(!node(false), Is().EqualTo(node(true)));
        });

        it("should negate 'true' terminal node", [&]() {
          AssertThat(!node(true),  Is().EqualTo(node(false)));
        });
      });
    });
  });
 });
