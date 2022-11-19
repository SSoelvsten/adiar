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

            describe("create_node, label_of, id_of", [&]() {
                it("should create node [label_t, id_t, ptr_uint64, ptr_uint64] [1]", [&]() {
                    const node n1 = node(3u,12u, terminal_F, terminal_T);
                    AssertThat(n1.uid(), Is().EqualTo(ptr_uint64(3,12)));
                    AssertThat(n1.label(), Is().EqualTo(3u));
                    AssertThat(n1.id(), Is().EqualTo(12u));

                    AssertThat(n1.low(), Is().EqualTo(terminal_F));
                    AssertThat(n1.high(), Is().EqualTo(terminal_T));
                  });

                it("should create node [label_t, id_t, ptr_uint64, ptr_uint64] [2]", [&]() {
                    const node n2 = node(3u,42u, terminal_T, terminal_F);
                    AssertThat(n2.uid(), Is().EqualTo(ptr_uint64(3,42)));
                    AssertThat(n2.label(), Is().EqualTo(3u));
                    AssertThat(n2.id(), Is().EqualTo(42u));

                    AssertThat(n2.low(), Is().EqualTo(terminal_T));
                    AssertThat(n2.high(), Is().EqualTo(terminal_F));
                  });

                it("should create node [label_t, id_t, node&, node&]", [&]() {
                    const node n_child1 = node(3u,12u, terminal_F, terminal_T);
                    const node n_child2 = node(3u,42u, terminal_T, terminal_F);

                    const node n = node(2,2, n_child1, n_child2);
                    AssertThat(n.uid(), Is().EqualTo(ptr_uint64(2,2)));
                    AssertThat(n.label(), Is().EqualTo(2u));
                    AssertThat(n.id(), Is().EqualTo(2u));

                    AssertThat(n.low(), Is().EqualTo(n_child1.uid()));
                    AssertThat(n.high(), Is().EqualTo(n_child2.uid()));
                  });

                it("should create node [label_t, id_t, node&, ptr_uint64]", [&]() {
                    const node n_child = node(2u,2u, terminal_F, terminal_T);

                    const node n = node(1u,7u,terminal_T,n_child);
                    AssertThat(n.uid(), Is().EqualTo(ptr_uint64(1,7)));
                    AssertThat(n.label(), Is().EqualTo(1u));
                    AssertThat(n.id(), Is().EqualTo(7u));

                    AssertThat(n.low(), Is().EqualTo(terminal_T));
                    AssertThat(n.high(), Is().EqualTo(n_child.uid()));
                  });

                it("should create node [label_t, id_t, ptr_uint64, node&]", [&]() {
                    const node n_child = node(2u,2u, terminal_F,terminal_T);

                    const node n = node(0u,3u, terminal_T,n_child);
                    AssertThat(n.uid(), Is().EqualTo(ptr_uint64(0,3)));
                    AssertThat(n.label(), Is().EqualTo(0u));
                    AssertThat(n.id(), Is().EqualTo(3u));

                    AssertThat(n.low(), Is().EqualTo(terminal_T));
                    AssertThat(n.high(), Is().EqualTo(n_child.uid()));
                  });
              });

            it("should sort by label, then by id", [&]() {
                const node node_1_2 = node(1u,2u, terminal_F, terminal_T);
                const node node_2_1 = node(2u,1u, terminal_T, terminal_F);

                AssertThat(node_1_2 < node_2_1, Is().True());
                AssertThat(node_2_1 > node_1_2, Is().True());

                const node node_2_2 = node(2u,2u, terminal_F, terminal_F);

                AssertThat(node_2_1 < node_2_2, Is().True());
                AssertThat(node_2_2 > node_2_1, Is().True());
              });

            it("should be equal by their content", [&]() {
                const node node_1_v1 = node(42u,2u, terminal_F, terminal_T);
                const node node_1_v2 = node(42u,2u, terminal_F, terminal_T);

                AssertThat(node_1_v1 == node_1_v2, Is().True());
                AssertThat(node_1_v1 != node_1_v2, Is().False());
              });

            it("should be unequal by their content", [&]() {
                const node node_1 = node(42u,2u, terminal_F, terminal_T);
                const node node_2 = node(42u,2u, terminal_F, terminal_F);
                const node node_3 = node(42u,3u, terminal_F, terminal_T);
                const node node_4 = node(21u,2u, terminal_F, terminal_T);

                AssertThat(node_1 == node_2, Is().False());
                AssertThat(node_1 != node_2, Is().True());

                AssertThat(node_1 == node_3, Is().False());
                AssertThat(node_1 != node_3, Is().True());

                AssertThat(node_1 == node_4, Is().False());
                AssertThat(node_1 != node_4, Is().True());
              });

            describe("terminal nodes", [&]() {
                const node terminal_node = node(true);
                const node terminal_node_F = node(false);

                describe("is_terminal", [&]() {
                    it("accepts true terminal", [&]() {
                        AssertThat(terminal_node.is_terminal(), Is().True());
                      });

                    it("accepts false terminal", [&]() {
                        AssertThat(terminal_node_F.is_terminal(), Is().True());
                      });

                    it("rejects non-terminal nodes [1]", [&]() {
                        const node node_1 = node(42u,2u, terminal_F, terminal_T);
                        AssertThat(node_1.is_terminal(), Is().False());
                      });

                    it("rejects non-terminal nodes [2]", [&]() {
                        const node almost_F_terminal = node(0u,0u, terminal_T, ptr_uint64(42,2));
                        AssertThat(almost_F_terminal.is_terminal(), Is().False());
                      });

                    it("rejects non-terminal nodes [2]", [&]() {
                        const node almost_T_terminal = node(0u,1u, terminal_T, ptr_uint64(42,2));
                        AssertThat(almost_T_terminal.is_terminal(), Is().False());
                      });
                  });

                describe("value_of", [&]() {
                    it("retrieves value of a true terminal node", [&]() {
                        AssertThat(terminal_node.value(), Is().True());
                      });

                    it("retrieves value of a false terminal node", [&]() {
                        AssertThat(terminal_node_F.value(), Is().False());
                      });
                  });

                describe("is_false", [&]() {
                    it("rejects true terminal", [&]() {
                        AssertThat(terminal_node.is_false(), Is().False());
                      });

                    it("accepts false terminal", [&]() {
                        AssertThat(terminal_node_F.is_false(), Is().True());
                      });

                    it("rejects non-terminal nodes", [&]() {
                        const node n = node(0,0, ptr_uint64(42,2), terminal_F);
                        AssertThat(n.is_false(), Is().False());
                      });
                  });

                describe("is_true", [&]() {
                    it("accepts true terminal", [&]() {
                        AssertThat(terminal_node.is_true(), Is().True());
                      });

                    it("rejects false terminal", [&]() {
                        AssertThat(terminal_node_F.is_true(), Is().False());
                      });

                    it("rejects non-terminal nodes", [&]() {
                        const node n = node(0,1, terminal_T, ptr_uint64(2,3));
                        AssertThat(n.is_true(), Is().False());
                      });
                  });
              });

            describe("negate (!)", [&]() {
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

                it("should negate terminal node", [&]() {
                    AssertThat(!node(true), Is().EqualTo(node(false)));
                    AssertThat(!node(false), Is().EqualTo(node(true)));
                  });
              });
          });
      });
  });
