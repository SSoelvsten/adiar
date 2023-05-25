#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/ptr.h", []() {
    describe("ptr_uint64", []() {
      it("should recognise NIL (unflagged)", [&]() {
        const ptr_uint64 some_value = ptr_uint64::NIL();
        AssertThat(some_value.is_nil(), Is().True());
      });

      describe("NIL", [&](){
        it("should recognise NIL (unflagged)", [&]() {
          const ptr_uint64 some_value = ptr_uint64::NIL();
          AssertThat(some_value.is_nil(), Is().True());
        });

        it("should recognise NIL (flagged)", [&]() {
          const ptr_uint64 some_value = flag(ptr_uint64::NIL());
          AssertThat(some_value.is_nil(), Is().True());
        });

        it("can see whether the flag is set", [&]() {
          AssertThat(flag(ptr_uint64::NIL()).is_flagged(), Is().True());
        });

        it("can see whether the flag is not set", [&]() {
          AssertThat(ptr_uint64::NIL().is_flagged(), Is().False());
        });
      });

      describe("terminals", [&](){
        it("should take up 8 bytes of memory", [&]() {
          const ptr_uint64 terminal = ptr_uint64(false);
          AssertThat(sizeof(terminal), Is().EqualTo(8u));
        });

        describe("flag, unflag, is_flagged", []() {
          it("is unflagged by default", [&]() {
            ptr_uint64 p = ptr_uint64(true);
            AssertThat(p.is_flagged(), Is().False());

            p = ptr_uint64(true);
            AssertThat(p.is_flagged(), Is().False());
          });

          it("can set the flag", [&]() {
            ptr_uint64 p = flag(ptr_uint64(false));
            AssertThat(p.is_flagged(), Is().True());

            p = flag(ptr_uint64(true));
            AssertThat(p.is_flagged(), Is().True());
          });

          it("can unset the flag", [&]() {
            ptr_uint64 p = flag(ptr_uint64(false));
            p = unflag(p);
            AssertThat(p.is_flagged(), Is().False());

            p = flag(ptr_uint64(true));
            p = unflag(p);
            AssertThat(p.is_flagged(), Is().False());
          });
        });

        describe("is_terminal", []() {
          it("should recognise Sinks as such", [&]() {
            const ptr_uint64 terminal_F = ptr_uint64(false);
            const ptr_uint64 terminal_T = ptr_uint64(true);

            AssertThat(terminal_F.is_terminal(), Is().True());
            AssertThat(terminal_T.is_terminal(), Is().True());
          });

          it("should not be confused with Node Ptr (unflagged)", [&]() {
            ptr_uint64 arc_node_max = ptr_uint64(ptr_uint64::MAX_LABEL,
                                                 ptr_uint64::MAX_ID);

            AssertThat(arc_node_max.is_terminal(), Is().False());
            AssertThat(arc_node_max.is_false(), Is().False());
            AssertThat(arc_node_max.is_true(), Is().False());

            ptr_uint64 arc_node_min = ptr_uint64(0,0);
            AssertThat(arc_node_min.is_terminal(), Is().False());
            AssertThat(arc_node_min.is_false(), Is().False());
            AssertThat(arc_node_min.is_true(), Is().False());

            ptr_uint64 arc_node = ptr_uint64(42,18);
            AssertThat(arc_node.is_terminal(), Is().False());
            AssertThat(arc_node.is_false(), Is().False());
            AssertThat(arc_node.is_true(), Is().False());
          });

          it("should not be confused with Node Ptr (flagged)", [&]() {
            ptr_uint64 arc_node_max = flag(ptr_uint64(ptr_uint64::MAX_LABEL,
                                                      ptr_uint64::MAX_ID));

            AssertThat(arc_node_max.is_terminal(), Is().False());
            AssertThat(arc_node_max.is_false(), Is().False());
            AssertThat(arc_node_max.is_true(), Is().False());

            ptr_uint64 arc_node_min = flag(ptr_uint64(0,0));
            AssertThat(arc_node_min.is_terminal(), Is().False());
            AssertThat(arc_node_min.is_false(), Is().False());
            AssertThat(arc_node_min.is_true(), Is().False());

            ptr_uint64 arc_node = flag(ptr_uint64(42,18));
            AssertThat(arc_node.is_terminal(), Is().False());
            AssertThat(arc_node.is_false(), Is().False());
            AssertThat(arc_node.is_true(), Is().False());
          });

          it("should not be confused with Nil (unflagged)", [&]() {
            AssertThat(ptr_uint64::NIL().is_terminal(), Is().False());
          });

          it("should not be confused with Nil (flagged)", [&]() {
            AssertThat(flag(ptr_uint64::NIL()).is_terminal(), Is().False());
          });
        });

        describe("value", []() {
          it("retrieves value from false terminal", [&]() {
            ptr_uint64 p = ptr_uint64(false);
            AssertThat(p.value(), Is().False());
          });

          it("retrieves value from true terminal", [&]() {
            ptr_uint64 p = ptr_uint64(true);
            AssertThat(p.value(), Is().True());
          });
        });

        describe("is_false", []() {
          it("should accept false terminal", [&]() {
            ptr_uint64 terminal_F = ptr_uint64(false);
            AssertThat(terminal_F.is_false(), Is().True());
          });

          it("should accept false terminal (flagged)", [&]() {
            ptr_uint64 terminal_F = flag(ptr_uint64(false));
            AssertThat(terminal_F.is_false(), Is().True());
          });

          it("should reject terminal", [&]() {
            ptr_uint64 terminal_T = ptr_uint64(true);
            AssertThat(terminal_T.is_false(), Is().False());
          });

          it("should reject non-terminal", [&]() {
            ptr_uint64 p = ptr_uint64(0,0);
            AssertThat(p.is_false(), Is().False());
          });

          it("should reject NIL", [&]() {
            ptr_uint64 p = ptr_uint64::NIL();
            AssertThat(p.is_false(), Is().False());
          });
        });

        describe("is_true", []() {
          it("should reject false terminal", [&]() {
            ptr_uint64 terminal_F = ptr_uint64(false);
            AssertThat(terminal_F.is_true(), Is().False());
          });

          it("should accept true terminal", [&]() {
            ptr_uint64 terminal_T = ptr_uint64(true);
            AssertThat(terminal_T.is_true(), Is().True());
          });

          it("should accept true terminal (flagged)", [&]() {
            ptr_uint64 terminal_T = flag(ptr_uint64(true));
            AssertThat(terminal_T.is_true(), Is().True());
          });

          it("should reject non-terminal", [&]() {
            ptr_uint64 p = ptr_uint64(0,1);
            AssertThat(p.is_true(), Is().False());
          });

          it("should reject NIL", [&]() {
            ptr_uint64 p = ptr_uint64::NIL();
            AssertThat(p.is_true(), Is().False());
          });
        });

        describe("negate", []() {
          it("should negate terminal (unflagged)", [&]() {
            ptr_uint64 p1 = ptr_uint64(false);
            AssertThat(negate(p1), Is().EqualTo(ptr_uint64(true)));

            ptr_uint64 p2 = ptr_uint64(true);
            AssertThat(negate(p2), Is().EqualTo(ptr_uint64(false)));
          });

          it("should negate terminal into terminal (flagged)", [&]() {
            ptr_uint64 p1 = flag(ptr_uint64(false));
            AssertThat(negate(p1), Is().EqualTo(flag(ptr_uint64(true))));

            ptr_uint64 p2 = flag(ptr_uint64(true));
            AssertThat(negate(p2), Is().EqualTo(flag(ptr_uint64(false))));
          });
        });
      });

      describe("internal nodes", [&]() {
        const ptr_uint64 terminal_F = ptr_uint64(false);
        const ptr_uint64 terminal_T = ptr_uint64(true);

        it("should take up 8 bytes of memory", [&]() {
          const ptr_uint64 node_ptr = ptr_uint64(42,2);
          AssertThat(sizeof(node_ptr), Is().EqualTo(8u));
        });

        describe("flag, unflag, is_flagged", [&]() {
          it("can recognise the flag is set", [&]() {
            ptr_uint64 p = flag(ptr_uint64(42,2));
            AssertThat(p.is_flagged(), Is().True());

            p = flag(ptr_uint64(17,3));
            AssertThat(p.is_flagged(), Is().True());
          });

          it("can recognise the flag is not set", [&]() {
            ptr_uint64 p = ptr_uint64(42,2);
            AssertThat(p.is_flagged(), Is().False());

            p = ptr_uint64(17,3);
            AssertThat(p.is_flagged(), Is().False());
          });
        });

        describe("is_node", [&]() {
          it("should recognise Node Ptr (unflagged)", [&]() {
            const ptr_uint64 p_node_max = ptr_uint64(ptr_uint64::MAX_LABEL,
                                                     ptr_uint64::MAX_ID);

            AssertThat(p_node_max.is_node(), Is().True());

            const ptr_uint64 p_node_min = ptr_uint64(0,0);
            AssertThat(p_node_min.is_node(), Is().True());

            const ptr_uint64 p_node_min_max = ptr_uint64(0, ptr_uint64::MAX_ID);
            AssertThat(p_node_min_max.is_node(), Is().True());

            const ptr_uint64 p_node = ptr_uint64(42,18);
            AssertThat(p_node.is_node(), Is().True());
          });

          it("should recognise Node Ptr (flagged)", [&]() {
            const ptr_uint64 p_node_max = flag(ptr_uint64(ptr_uint64::MAX_LABEL,
                                                          ptr_uint64::MAX_ID));

            AssertThat(p_node_max.is_node(), Is().True());

            const ptr_uint64 p_node_min = flag(ptr_uint64(0,0));
            AssertThat(p_node_min.is_node(), Is().True());

            const ptr_uint64 p_node_min_max = ptr_uint64(0, ptr_uint64::MAX_ID);
            AssertThat(p_node_min_max.is_node(), Is().True());

            const ptr_uint64 p_node = flag(ptr_uint64(42,18));
            AssertThat(p_node.is_node(), Is().True());
          });

          it("should not be confused with Terminals", [&]() {
            const ptr_uint64 terminal_F = ptr_uint64(false);
            const ptr_uint64 terminal_T = ptr_uint64(true);

            AssertThat(terminal_F.is_node(), Is().False());
            AssertThat(terminal_T.is_node(), Is().False());
          });

          it("should not be confused with Nil (unflagged)", [&]() {
            AssertThat(ptr_uint64::NIL().is_node(), Is().False());
          });

          it("should not be confused with Nil (flagged)", [&]() {
            AssertThat(flag(ptr_uint64::NIL()).is_node(), Is().False());
          });
        });

        describe("label", [&]() {
          it("should store and retrieve label for Ptr with maximal id (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(12, ptr_uint64::MAX_ID);
            AssertThat(p.label(), Is().EqualTo(12u));
          });

          it("should store and retrieve 42 label Ptr (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(42,2);
            AssertThat(p.label(), Is().EqualTo(42u));
          });

          it("should store and retrieve 21 label Ptr (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(21,2);
            AssertThat(p.label(), Is().EqualTo(21u));
          });

          it("should store and retrieve MAX label Ptr (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::MAX_LABEL,
                                            ptr_uint64::MAX_ID);

            AssertThat(p.label(), Is().EqualTo(ptr_uint64::MAX_LABEL));
          });

          it("should store and retrieve label for Ptr with maximal id (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(12, ptr_uint64::MAX_ID));
            AssertThat(p.label(), Is().EqualTo(12u));
          });

          it("should store and retrieve 42 label Ptr (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(42,2));
            AssertThat(p.label(), Is().EqualTo(42u));
          });

          it("should store and retrieve 21 label Ptr (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(21,2));
            AssertThat(p.label(), Is().EqualTo(21u));
          });

          it("should store and retrieve MAX label Ptr (flagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::MAX_LABEL,
                                            ptr_uint64::MAX_ID);

            AssertThat(p.label(), Is().EqualTo(ptr_uint64::MAX_LABEL));
          });
        });

        describe("id", [&]() {
          it("should store and retrieve 42 id (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(2,42);
            AssertThat(p.id(), Is().EqualTo(42u));
          });

          it("should store and retrieve 21 id (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(2,21);
            AssertThat(p.id(), Is().EqualTo(21u));
          });

          it("should store and retrieve MAX id (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::MAX_LABEL,
                                            ptr_uint64::MAX_ID);

            AssertThat(p.id(), Is().EqualTo(ptr_uint64::MAX_ID));
          });

          it("should store and retrieve 42 id (flagged)", [&]() {
            const  ptr_uint64 p = flag(ptr_uint64(2,42));
            AssertThat(p.id(), Is().EqualTo(42u));
          });

          it("should store and retrieve 21 id (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(2,21));
            AssertThat(p.id(), Is().EqualTo(21u));
          });

          it("should store and retrieve MAX id (flagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::MAX_LABEL,
                                            ptr_uint64::MAX_ID);

            AssertThat(p.id(), Is().EqualTo(ptr_uint64::MAX_ID));
          });
        });

        describe("out-index", [&]() {
          it("has default value of 0 (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(2,42);
            AssertThat(p.out_idx(), Is().EqualTo(0u));
          });

          it("has default value of 0 (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(2,42));
            AssertThat(p.out_idx(), Is().EqualTo(0u));
          });

          it("has default value of 0 (max-id)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(2,ptr_uint64::MAX_ID));
            AssertThat(p.out_idx(), Is().EqualTo(0u));
          });

          it("has maximum out-index be 1", [&]() {
            AssertThat(ptr_uint64::MAX_OUT_IDX, Is().EqualTo(1u));
          });

          it("should store and retrieve 'false' out-index (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(2,42,false);
            AssertThat(p.out_idx(), Is().EqualTo(0u));
          });

          it("should store and retrieve 'false' out-index (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(2,42,false));
            AssertThat(p.out_idx(), Is().EqualTo(0u));
          });

          it("should store and retrieve 'true' out-index (unflagged)", [&]() {
            const ptr_uint64 p = ptr_uint64(2,42,true);
            AssertThat(p.out_idx(), Is().EqualTo(1u));
          });

          it("should store and retrieve 'true' out-index (flagged)", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(2,42,true));
            AssertThat(p.out_idx(), Is().EqualTo(1u));
          });
        });

        describe("ordering ( < )", [&]() {
          it("should sort by label, then by id", [&]() {
            const ptr_uint64 node_1_2 = ptr_uint64(1,2);
            const ptr_uint64 node_2_1 = ptr_uint64(2,1);
            const ptr_uint64 node_2_2 = ptr_uint64(2,2);

            AssertThat(node_1_2 < node_2_1, Is().True());
            AssertThat(node_2_1 < node_2_2, Is().True());
          });

          it("should sort by label and id independent of the flag", [&]() {
            const ptr_uint64 node_1_2 = ptr_uint64(1,2);
            const ptr_uint64 node_2_1 = ptr_uint64(2,1);

            AssertThat(node_1_2 < flag(node_2_1), Is().True());
            AssertThat(flag(node_1_2) < node_2_1, Is().True());
            AssertThat(flag(node_1_2) < flag(node_2_1), Is().True());
          });

          it("should sort by id, then by out-index", [&]() {
            const ptr_uint64 node_1_T = ptr_uint64(0,1,true);
            const ptr_uint64 node_2_F = ptr_uint64(0,2,false);
            const ptr_uint64 node_2_T = ptr_uint64(0,2,true);

            AssertThat(node_1_T < node_2_F, Is().True());
            AssertThat(node_2_F < node_2_T, Is().True());
          });

          it("should sort by id and out-index independent of the flag", [&]() {
            const ptr_uint64 node_1_T = ptr_uint64(0,1,true);
            const ptr_uint64 node_2_F = ptr_uint64(0,2,false);

            AssertThat(node_1_T < flag(node_2_F), Is().True());
            AssertThat(flag(node_1_T) < node_2_F, Is().True());
            AssertThat(flag(node_1_T) < flag(node_2_F), Is().True());
          });

          it("should sort by label, then id, and finally out-index", [&]() {
            const ptr_uint64 node_a = ptr_uint64(0,3,true);
            const ptr_uint64 node_b = ptr_uint64(1,2,false);

            AssertThat(node_a < node_b, Is().True());

            const ptr_uint64 node_c = ptr_uint64(2,0,false);
            const ptr_uint64 node_d = ptr_uint64(2,0,true);

            AssertThat(node_c < node_d, Is().True());
          });

          it("should sort by label id, and out-index independent of the flag", [&]() {
            const ptr_uint64 node_a = ptr_uint64(0,3,true);
            const ptr_uint64 node_b = ptr_uint64(1,2,false);

            AssertThat(node_a < flag(node_b), Is().True());
            AssertThat(flag(node_a) < node_b, Is().True());
            AssertThat(flag(node_a) < flag(node_b), Is().True());

            const ptr_uint64 node_c = ptr_uint64(2,0,false);
            const ptr_uint64 node_d = ptr_uint64(2,0,true);

            AssertThat(flag(node_c) < node_d, Is().True());
            AssertThat(node_c < flag(node_d), Is().True());
            AssertThat(flag(node_c) < flag(node_d), Is().True());
          });

          it("should sort F terminal after (largest) internal node", [&]() {
            // Create a node pointer with the highest possible raw value
            const ptr_uint64 p_node = flag(ptr_uint64(ptr_uint64::MAX_LABEL,
                                                      ptr_uint64::MAX_ID,
                                                      ptr_uint64::MAX_OUT_IDX));

            AssertThat(p_node < terminal_F, Is().True());
            AssertThat(flag(p_node) < terminal_F, Is().True());
            AssertThat(p_node < flag(terminal_F), Is().True());
            AssertThat(flag(p_node) < flag(terminal_F), Is().True());
          });

          it("should sort T terminal after (largest) internal node", [&]() {
            // Create a node pointer with the highest possible raw value
            const ptr_uint64 p_node = ptr_uint64(ptr_uint64::MAX_LABEL,
                                                 ptr_uint64::MAX_ID,
                                                 ptr_uint64::MAX_OUT_IDX);

            AssertThat(p_node < terminal_T, Is().True());
            AssertThat(flag(p_node) < terminal_T, Is().True());
            AssertThat(p_node < flag(terminal_T), Is().True());
            AssertThat(flag(p_node) < flag(terminal_T), Is().True());
          });
        });
      });
    });
  });
 });
