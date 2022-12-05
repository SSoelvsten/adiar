#include "../../../test.h"
#include <adiar/internal/algorithms/pred.h>

go_bandit([]() {
  describe("adiar/internal/algoritms/pred.h", []() {
    describe("is_isomorphic", []() {
      describe("Trivial cases", []() {
        shared_levelized_file<dd::node_t>
        terminal_F, x21, x42, x21_and_x22, x21_and_x22_and_x42, x21_and_x42, x21_xor_x42;

        { // Garbage collect writers to free write-lock
          node_writer w_F(terminal_F);
          w_F << node(false);

          node_writer w_21(x21);
          w_21 << node(21,0, ptr_uint64(false), ptr_uint64(true));

          node_writer w_42(x42);
          w_42 << node(42,0, ptr_uint64(false), ptr_uint64(true));

          node_writer w_21_and_22(x21_and_x22);
          w_21_and_22 << node(22,0, ptr_uint64(false), ptr_uint64(true))
                      << node(21,0, ptr_uint64(false), ptr_uint64(22,0));

          node_writer w_21_and_22_and_42(x21_and_x22_and_x42);
          w_21_and_22_and_42 << node(42,0, ptr_uint64(false), ptr_uint64(true))
                             << node(22,0, ptr_uint64(false), ptr_uint64(42,0))
                             << node(21,0, ptr_uint64(false), ptr_uint64(22,0));

          node_writer w_21_and_42(x21_and_x42);
          w_21_and_42 << node(42,0, ptr_uint64(false), ptr_uint64(true))
                      << node(21,0, ptr_uint64(false), ptr_uint64(42,0));

          node_writer w_21_xor_42(x21_xor_x42);
          w_21_xor_42 << node(42,1, ptr_uint64(true), ptr_uint64(false))
                      << node(42,0, ptr_uint64(false), ptr_uint64(true))
                      << node(21,0, ptr_uint64(42,0), ptr_uint64(42,1));
        }

        it("accepts same file with same negation flag", [&]() {
          AssertThat(is_isomorphic(terminal_F, terminal_F, false, false), Is().True());
          AssertThat(is_isomorphic(terminal_F, terminal_F, true, true), Is().True());
        });

        it("rejects same file with differing negation falgs", [&]() {
          AssertThat(is_isomorphic(terminal_F, terminal_F, false, true), Is().False());
          AssertThat(is_isomorphic(terminal_F, terminal_F, true, false), Is().False());
        });

        it("rejects on different number of nodes [1]", [&]() {
          AssertThat(is_isomorphic(x21, x21_and_x42, false, false), Is().False());
          AssertThat(is_isomorphic(x21, x21_and_x42, false, true), Is().False());
          AssertThat(is_isomorphic(x21_and_x42, x21, true, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x42, x21, true, true), Is().False());
        });

        it("rejects on different number of nodes [2]", [&]() {
          AssertThat(is_isomorphic(x21_xor_x42, x21_and_x42, false, false), Is().False());
          AssertThat(is_isomorphic(x21_xor_x42, x21_and_x42, false, true), Is().False());
          AssertThat(is_isomorphic(x21_and_x42, x21_xor_x42, true, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x42, x21_xor_x42, true, true), Is().False());
        });

        it("rejects on different number of levels [1]", [&]() {
          AssertThat(is_isomorphic(terminal_F, x42, false, false), Is().False());
          AssertThat(is_isomorphic(terminal_F, x42, false, true), Is().False());
          AssertThat(is_isomorphic(x42, terminal_F, true, false), Is().False());
          AssertThat(is_isomorphic(x42, terminal_F, true, true), Is().False());
        });

        it("rejects on different number of levels [2]", [&]() {
          AssertThat(is_isomorphic(x21_and_x22_and_x42, x21_and_x22, false, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x22_and_x42, x21_and_x22, false, true), Is().False());
          AssertThat(is_isomorphic(x21_and_x22, x21_and_x22_and_x42, true, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x22, x21_and_x22_and_x42, true, true), Is().False());
        });

        it("rejects on different labels in level [1]", [&]() {
          AssertThat(is_isomorphic(x21, x42, false, false), Is().False());
          AssertThat(is_isomorphic(x21, x42, false, true), Is().False());
          AssertThat(is_isomorphic(x42, x21, true, false), Is().False());
          AssertThat(is_isomorphic(x42, x21, true, true), Is().False());
        });

        it("rejects on different labels in level [2]", [&]() {
          AssertThat(is_isomorphic(x21_and_x42, x21_and_x22, false, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x42, x21_and_x22, false, true), Is().False());
          AssertThat(is_isomorphic(x21_and_x22, x21_and_x42, true, false), Is().False());
          AssertThat(is_isomorphic(x21_and_x22, x21_and_x42, true, true), Is().False());
        });

        it("rejects on different size of level", [&]() {
          // Create two BDDs with 7 nodes each on variables x0, x1, x2,
          // and x3 (i.e. the same number of nodes and levels). One with 3
          // nodes for x2 and one node for x3 and one with 2 nodes for x2 and x3.

          shared_levelized_file<dd::node_t> seven_a, seven_b;

          { // Garbage collect writers to free write-lock
            node_writer w_seven_a(seven_a);
            w_seven_a << node(3,0, ptr_uint64(false), ptr_uint64(true))
                    << node(2,2, ptr_uint64(false), ptr_uint64(true))
                    << node(2,1, ptr_uint64(true), ptr_uint64(3,0))
                    << node(2,0, ptr_uint64(false), ptr_uint64(3,0))
                    << node(1,1, ptr_uint64(2,0), ptr_uint64(2,1))
                    << node(1,0, ptr_uint64(2,1), ptr_uint64(2,2))
                    << node(0,0, ptr_uint64(1,0), ptr_uint64(1,1));

            node_writer w_seven_b(seven_b);
            w_seven_b << node(3,1, ptr_uint64(true), ptr_uint64(false))
                      << node(3,0, ptr_uint64(false), ptr_uint64(true))
                      << node(2,1, ptr_uint64(3,1), ptr_uint64(true))
                      << node(2,0, ptr_uint64(false), ptr_uint64(3,0))
                      << node(1,1, ptr_uint64(2,1), ptr_uint64(2,0))
                      << node(1,0, ptr_uint64(2,0), ptr_uint64(2,1))
                      << node(0,0, ptr_uint64(1,0), ptr_uint64(1,1));
          }

          AssertThat(is_isomorphic(seven_a, seven_b, false, false), Is().False());
          AssertThat(is_isomorphic(seven_a, seven_b, false, true), Is().False());
          AssertThat(is_isomorphic(seven_b, seven_a, true, false), Is().False());
          AssertThat(is_isomorphic(seven_b, seven_a, true, true), Is().False());
        });
      });


      //////////////////////
      // Sink-only cases
      shared_levelized_file<dd::node_t> F_a, F_b, T_a;

      { // Garbage collect writers to free write-lock
        node_writer nw_F_a(F_a);
        nw_F_a << node(false);

        node_writer nw_F_b(F_b);
        nw_F_b << node(false);

        node_writer nw_T_a(T_a);
        nw_T_a << node(true);
      }

      //////////////////////
      // One-node cases
      /*
           1     ---- x42
          / \
          F T
      */
      shared_levelized_file<dd::node_t> x42_a, x42_b, not_x42;

      { // Garbage collect writers to free write-lock
        node_writer wa(x42_a);
        wa << node(42, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));

        node_writer wb(x42_b);
        wb << node(42, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));

        node_writer wn(not_x42);
        wn << node(42, node::MAX_ID, ptr_uint64(true), ptr_uint64(false));
      }

      /*
           1    or   1   ---- x69
          / \       / \
          T T       F F
      */
      shared_levelized_file<dd::node_t> x69_T, x69_F;

      { // Garbage collect writers to free write-lock
        node_writer wT(x69_T);
        wT << node(69, node::MAX_ID, ptr_uint64(true), ptr_uint64(true));

        node_writer wF(x69_F);
        wF << node(69, node::MAX_ID, ptr_uint64(false), ptr_uint64(false));
      }


      //////////////////////
      // Traversal cases
      shared_levelized_file<node> bdd_1;
      /*
            _1_     ---- x0
           /   \
           2   3    ---- x1
          / \ / \
          T  4  F   ---- x2
            / \
            F T
      */
      { node_writer w(bdd_1);
        w << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(false))
          << node(1, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_1n;
      /* bdd_1 negated */
      { node_writer w(bdd_1n);
        w << node(2, node::MAX_ID,   ptr_uint64(true),       ptr_uint64(false))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(true))
          << node(1, node::MAX_ID-1, ptr_uint64(false),      ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      // bdd_1 with child of (2) and low child of (4) flipped in truth value
      shared_levelized_file<node> bdd_1_low_leaf;
      { node_writer w(bdd_1_low_leaf);
        w << node(2, node::MAX_ID,   ptr_uint64(true),       ptr_uint64(true))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(false))
          << node(1, node::MAX_ID-1, ptr_uint64(false),      ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      // bdd_1 with child of (3) and high child of (4) flipped in truth value
      shared_levelized_file<node> bdd_1_high_leaf;
      { node_writer w(bdd_1_high_leaf);
        w << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(false))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(true))
          << node(1, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_2;
      /*
             1      ---- x0
            / \
           2   3    ---- x1
          /  X  \
         F  / \  F
           4   5     ---- x2
          / \ / \
         T   F   T
      */
      { node_writer w(bdd_2);
        w << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
          << node(2, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(false))
          << node(1, node::MAX_ID-1, ptr_uint64(false),      ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_2n;
      /* bdd_2 negated */
      { node_writer w(bdd_2n);
        w << node(2, node::MAX_ID,   ptr_uint64(true),       ptr_uint64(false))
          << node(2, node::MAX_ID-1, ptr_uint64(false),      ptr_uint64(true))
          << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(true))
          << node(1, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_2_low_child;
      /*
             1       ---- x0
            / \
           2   3     ---- x1
          / \  |\
         4   5 | F   ---- x2
        / \ / \|
       T   F   T

          which in traversal look similar to bdd_2 until (3) on level x1
      */
      { node_writer w(bdd_2_low_child);
        w << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
          << node(2, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))
          << node(1, node::MAX_ID,   ptr_uint64(true),       ptr_uint64(false))
          << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_2_high_child;
      /*
             1     ---- x0
            / \
           2   3   ---- x1
          /  X  \
         4  | 5 |  ---- x2
        /  \|/ \|
        T   F   T

          which in traversal look similar to bdd_2 until (2) on level x1
      */
      { node_writer w(bdd_2_high_child);
        w << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
          << node(2, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))
          << node(1, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
          << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(1, ptr_uint64::MAX_ID-1));
      }

      shared_levelized_file<node> bdd_3;
      /*
           1      ---- x0
          / \
          |  2    ---- x1
          \ / \
           3  F   ---- x2
          / \
          T 4     ---- x3
           / \
           F T
      */
      { node_writer w(bdd_3);
        w << node(3, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
          << node(2, node::MAX_ID, ptr_uint64(true),     ptr_uint64(3, ptr_uint64::MAX_ID))
          << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(false))
          << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_4;
      /*
             1     ---- x0
            / \
           2  |    ---- x1
          / \ /
          F  3     ---- x2
            / \
            4 T    ---- x3
           / \
           F T

          The same as bdd_3 but mirrored horisontally
      */
      { node_writer w(bdd_4);
        w << node(3, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
          << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(true))
          << node(1, node::MAX_ID, ptr_uint64(false),    ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_5;
      /*
             1     ---- x0
            / \
           2   |   ---- x1
          / \  |
          F T  3   ---- x2
              / \
              4 T  ---- x3
             / \
             F T

          The same as bdd_4 but (2) goes to a terminal instead of (3)
      */
      { node_writer w(bdd_5);
        w << node(3, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
          << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(true))
          << node(1, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      shared_levelized_file<node> bdd_6;
      /*
             1     ---- x0
            / \
           2   |   ---- x1
          / \  |
          F T  3   ---- x2
              / \
              4 F  ---- x3
             / \
             F T

          The same as bdd_5 but (3) goes to a false terminal instead of true
      */
      { node_writer w(bdd_5);
        w << node(3, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
          << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(false))
          << node(1, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      describe("Fast 2N/B check", [&]() {
        // The fast checks require the BDDs to be on 'canonical' form, which
        // makes two isomorphic BDDs truly identical.

        //////////////////////
        // Sink-only cases
        it("accepts two different F terminals ", [&]() {
          AssertThat(is_isomorphic(F_a, F_b), Is().True());
        });

        it("rejects an F and a T terminal ", [&]() {
          AssertThat(is_isomorphic(T_a, F_a), Is().False());
          AssertThat(is_isomorphic(F_b, T_a), Is().False());
        });

        it("rejects an F with a T terminal, where both are negated", [&]() {
          AssertThat(is_isomorphic(T_a, F_a, true, true), Is().False());
          AssertThat(is_isomorphic(T_a, F_a, true, true), Is().False());
        });

        //////////////////////
        // One-node cases
        it("accepts two different x42", [&]() {
          AssertThat(is_isomorphic(x42_a, x42_b, false, false), Is().True());
          AssertThat(is_isomorphic(x42_a, x42_b, true, true), Is().True());
        });

        it("rejects x42 and ~x42", [&]() {
          AssertThat(is_isomorphic(x42_a, not_x42, false, false), Is().False());
          AssertThat(is_isomorphic(x42_a, not_x42, true, true), Is().False());
        });

        it("rejects x69_T and x69_F", [&]() {
          AssertThat(is_isomorphic(x69_T, x69_F, false, false), Is().False());
          AssertThat(is_isomorphic(x69_T, x69_F, true, true), Is().False());
        });

        //////////////////////
        // Traversal cases
        it("rejects its negation [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1, bdd_1n, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_1, bdd_1n, true, true), Is().False());
        });

        it("rejects its negation [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2, bdd_2n, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_2, bdd_2n, true, true), Is().False());
        });

        it("rejects on low child mismatch (leaf value) [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1, bdd_1_low_leaf, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_1, bdd_1_low_leaf, true, true), Is().False());
        });

        it("rejects on low child mismatch (internal node vs. leaf) [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2, bdd_2_low_child, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_2, bdd_2_low_child, true, true), Is().False());
        });

        it("rejects on low child mismatch (internal node labels) [3]", [&]() {
          shared_levelized_file<node> bdd_3_b;
          /* Same as bdd_3 but with (2) directly going to (4) on the low */
          { // Garbage collect writers to free write-lock
            node_writer w(bdd_3_b);
            w << node(3, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
              << node(2, node::MAX_ID, ptr_uint64(true),     ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(1, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(false))
              << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_3, bdd_3_b, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_3, bdd_3_b, true, true), Is().False());
        });

        it("rejects on high child mismatch (leaf value) [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1, bdd_1_high_leaf, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_1, bdd_1_high_leaf, true, true), Is().False());
        });

        it("rejects on high child mismatch (internal node vs. leaf) [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2, bdd_2_high_child, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_2, bdd_2_high_child, true, true), Is().False());
        });

        it("rejects on high child mismatch (internal node labels) [4]", [&]() {
          shared_levelized_file<node> bdd_4_b;
          /* Same as bdd_4 but with (2) directly going to (4) on the high */
          { node_writer w(bdd_4_b);
            w << node(3, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
              << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(true))
              << node(1, node::MAX_ID, ptr_uint64(false),    ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_4, bdd_4_b, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_4, bdd_4_b, true, true), Is().False());
        });

        it("rejects on mismatch of total number of terminals", [&]() {
          AssertThat(is_isomorphic(bdd_4, bdd_5, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_4, bdd_5, true, true), Is().False());
          AssertThat(is_isomorphic(bdd_4, bdd_5, false, true), Is().False());
          AssertThat(is_isomorphic(bdd_4, bdd_5, true, false), Is().False());
        });

        it("rejects on mismatch of number of one type of terminal", [&]() {
          AssertThat(is_isomorphic(bdd_5, bdd_6, false, false), Is().False());
          AssertThat(is_isomorphic(bdd_5, bdd_6, true, true), Is().False());
        });
      });

      describe("Slow O(sort(N)) check", [&]() {
        //////////////////////
        // Sink-only cases
        it("rejects two F terminals, where one is negated", [&]() {
          AssertThat(is_isomorphic(F_a, F_b, true, false), Is().False());
          AssertThat(is_isomorphic(F_a, F_b, false, true), Is().False());
        });

        it("accepts an F with a T terminal, where one is negated", [&]() {
          AssertThat(is_isomorphic(T_a, F_a, true, false), Is().True());
          AssertThat(is_isomorphic(T_a, F_a, false, true), Is().True());
          AssertThat(is_isomorphic(F_b, T_a, true, false), Is().True());
          AssertThat(is_isomorphic(F_b, T_a, false, true), Is().True());
        });

        //////////////////////
        // One-node cases
        it("rejects two x42, where one is negated", [&]() {
          AssertThat(is_isomorphic(x42_a, x42_b, false, true), Is().False());
          AssertThat(is_isomorphic(x42_a, x42_b, true, false), Is().False());
        });

        it("accepts x42 with negated ~x42", [&]() {
          AssertThat(is_isomorphic(x42_a, not_x42, false, true), Is().True());
          AssertThat(is_isomorphic(x42_a, not_x42, true, false), Is().True());
        });

        //////////////////////
        // Traversal cases
        it("accepts with negation of negated [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1, bdd_1n, false, true), Is().True());
          AssertThat(is_isomorphic(bdd_1, bdd_1n, true, false), Is().True());
        });

        it("accepts with negation of negated [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2, bdd_2n, false, true), Is().True());
          AssertThat(is_isomorphic(bdd_2, bdd_2n, true, false), Is().True());
        });

        it("accepts with nodes swapped [1]", [&]() {
          // bdd_1 with nodes (2) and (3) swapped (and hence non-canonical)
          shared_levelized_file<node> bdd_1b;
          { // Garbage collect writers to free write-lock
            node_writer w(bdd_1b);
            w << node(2, node::MAX_ID,   ptr_uint64(false),    ptr_uint64(true))
              << node(1, node::MAX_ID,   ptr_uint64(true),     ptr_uint64(2, ptr_uint64::MAX_ID))
              << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(false))
              << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID-1));
          }

          AssertThat(is_isomorphic(bdd_1, bdd_1b, false, false), Is().True());
          AssertThat(is_isomorphic(bdd_1n, bdd_1b, true, false), Is().True());
          AssertThat(is_isomorphic(bdd_1n, bdd_1b, false, true), Is().True());
          AssertThat(is_isomorphic(bdd_1, bdd_1b, true, true), Is().True());
        });

        it("accepts with nodes swapped [2]", [&]() {
          // bdd_2 with nodes (4) and (5) swapped (and hence non-canonical)
          shared_levelized_file<node> bdd_2b;
          { // Garbage collect writers to free write-lock
            node_writer w(bdd_2b);
            w << node(2, node::MAX_ID,   ptr_uint64(true),       ptr_uint64(false))
              << node(2, node::MAX_ID-1, ptr_uint64(false),      ptr_uint64(true))
              << node(1, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(2, ptr_uint64::MAX_ID-1))
              << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(false))
              << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID-1));
          }

          AssertThat(is_isomorphic(bdd_2, bdd_2b, false, false), Is().True());
          AssertThat(is_isomorphic(bdd_2n, bdd_2b, true, false), Is().True());
          AssertThat(is_isomorphic(bdd_2n, bdd_2b, false, true), Is().True());
          AssertThat(is_isomorphic(bdd_2, bdd_2b, true, true), Is().True());
        });

        it("rejects on low child mismatch (leaf value) [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1n, bdd_1_low_leaf, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_1n, bdd_1_low_leaf, false, true), Is().False());
        });

        it("rejects on low child mismatch (internal node vs. leaf) [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2n, bdd_2_low_child, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_2n, bdd_2_low_child, false, true), Is().False());
        });

        it("rejects on low child mismatch (internal node labels) [3]", [&]() {
          shared_levelized_file<node> bdd_3_b;
          /* Same as bdd_3 negated but with (2) directly going to (4) on the low */
          { node_writer w(bdd_3_b);
            w << node(3, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
              << node(2, node::MAX_ID, ptr_uint64(false),    ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(1, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(true))
              << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_3, bdd_3_b, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_3, bdd_3_b, false, true), Is().False());
        });

        it("rejects on low child mismatch on root", [&]() {
          shared_levelized_file<node> bdd_a;
          { node_writer w(bdd_a);
            w << node(1, node::MAX_ID, ptr_uint64(false), ptr_uint64(true))
              << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(false));
          }

          shared_levelized_file<node> bdd_b;
          { node_writer w(bdd_b);
            w << node(1, node::MAX_ID, ptr_uint64(true), ptr_uint64(false))
              << node(0, node::MAX_ID, ptr_uint64(true), ptr_uint64(1, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_a, bdd_b, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_a, bdd_b, false, true), Is().False());
        });

        it("rejects on high child mismatch (leaf value) [1]", [&]() {
          AssertThat(is_isomorphic(bdd_1n, bdd_1_high_leaf, false, true), Is().False());
          AssertThat(is_isomorphic(bdd_1n, bdd_1_high_leaf, true, false), Is().False());
        });

        it("rejects on high child mismatch (internal node vs. leaf) [2]", [&]() {
          AssertThat(is_isomorphic(bdd_2n, bdd_2_high_child, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_2n, bdd_2_high_child, false, true), Is().False());
        });

        it("rejects on high child mismatch (internal node labels) [4]", [&]() {
          shared_levelized_file<node> bdd_4_b;
          /* Same as bdd_4 negated but with (2) directly going to (4) on
             the high */
          { node_writer w(bdd_4_b);
            w << node(3, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
              << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(false))
              << node(1, node::MAX_ID, ptr_uint64(true),     ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_4, bdd_4_b, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_4, bdd_4_b, false, true), Is().False());
        });

        it("rejects on high child mismatch on root", [&]() {
          shared_levelized_file<node> bdd_a;
          { node_writer w(bdd_a);
            w << node(2, node::MAX_ID, ptr_uint64(false),    ptr_uint64(true))
              << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(false))
              << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(true));
          }

          shared_levelized_file<node> bdd_b;
          { node_writer w(bdd_b);
            w << node(2, node::MAX_ID, ptr_uint64(true),     ptr_uint64(false))
              << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(false))
              << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(true));
          }

          AssertThat(is_isomorphic(bdd_a, bdd_b, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_a, bdd_b, false, true), Is().False());
        });

        it("rejects when resolving more requests on a level than nodes in original BDDs", [&]() {
          /*
                       1          ---- x0
                      / \
                      | 2         ---- x1
                      |/ \
                      3   4       ---- x2
                     / \ / \
                     .  .  .

                     (x3 level omitted from drawing)

                   We can then create another version where the low and high of
                   (2) has been swapped. This can be seen on level x2 (despite
                   all the children are to something with x3) since the unique
                   requests are:

                   [ (3,3), (3,4), (4,3) ]

                   which means that (3) in the first one has been related to
                   both (3) and (4) in the other one. Hence, they cannot be
                   isomorphic.
          */
          shared_levelized_file<node> bdd_5_a;
          { // Garbage collect writers to free write-lock
            node_writer w(bdd_5_a);
            w << node(3, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true))
              << node(3, node::MAX_ID-1, ptr_uint64(true), ptr_uint64(false))
              << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID-1), ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID-1))
              << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
              << node(0, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
          }

          shared_levelized_file<node> bdd_5_b;
          { // Garbage collect writers to free write-lock
            node_writer w(bdd_5_b);
            w << node(3, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))
              << node(3, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))
              << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID-1), ptr_uint64(3, ptr_uint64::MAX_ID))
              << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(3, ptr_uint64::MAX_ID-1))
              // This one has its children flipped
              << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(2, ptr_uint64::MAX_ID-1))
              << node(0, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
          }

          AssertThat(is_isomorphic(bdd_5_a, bdd_5_b, true, false), Is().False());
          AssertThat(is_isomorphic(bdd_5_b, bdd_5_a, false, true), Is().False());
        });
      });
    });
  });
 });
