#include "../../../test.h"

#include <adiar/internal/io/node_random_access.h>

#include <filesystem>

go_bandit([]() {
  describe("adiar/internal/io/node_file.h , node_stream.h , node_writer.h", []() {
    describe("node_writer [ .unsafe_* ] + node_file::stats", []() {
      // TODO: does not update canonicity
      // TODO: does update number of terminals
      // TODO: does increase 1-level and 2-level cuts
    });

    describe("node_writer [ << ] + node_file::stats", []() {
      // -------------------------------------------------------------------
      // Canonical node files where the max 1-level cut and max 2-level cuts
      // are the same.
      /*
                    F
      */
      levelized_file<node> nf_F;
      {
        node_writer nw(nf_F);
        nw << node(false);
      }

      /*
                    T
      */
      levelized_file<node> nf_T;
      {
        node_writer nw(nf_T);
        nw << node(true);
      }

      /*
                    1           ---- 42
                   / \
                   F T
      */
      levelized_file<node> nf_42;
      {
        node_writer nw(nf_42);
        nw << node(42, node::MAX_ID, node::ptr_t(false), node::ptr_t(true));
      }

      /*
                    1           ---- 42
                   / \
                   T F
      */
      levelized_file<node> nf_not42;
      {
        node_writer nw(nf_not42);
        nw << node(42, node::MAX_ID, node::ptr_t(false), node::ptr_t(true));
      }

      /*
                    1           ---- 42
                   / \
                   T T
      */
      levelized_file<node> nf_42andnot42;
      {
        node_writer nw(nf_42andnot42);
        nw << node(42, node::MAX_ID, node::ptr_t(true), node::ptr_t(true));
      }


      /*
                    1            ---- x0
                   / \
                   F 2           ---- x1
                    / \
                    F T
      */
      levelized_file<node> nf_0and1;
      {
        node_writer nw(nf_0and1);
        nw << node(1, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(0, node::MAX_ID, node::ptr_t(false), node::ptr_t(1, node::ptr_t::MAX_ID));
      }

      /*
                     1          ---- x0
                    / \
                    F 2         ---- x1
                     / \
                     F 3        ---- x2
                      / \
                      F T
      */
      levelized_file<node> nf_0and1and2;
      {
        node_writer nw(nf_0and1and2);
        nw << node(2, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(1, node::MAX_ID, node::ptr_t(false), node::ptr_t(2, node::ptr_t::MAX_ID))
           << node(0, node::MAX_ID, node::ptr_t(false), node::ptr_t(1, node::ptr_t::MAX_ID));
      }

      /*
                       1          ---- x0
                      / \
                      | 2         ---- x1
                      |/ \
                      3  T        ---- x2
                     / \
                     F T
      */
      levelized_file<node> nf_0and1_or_2;
      {
        node_writer nw(nf_0and1_or_2);
        nw << node(2, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(1, node::MAX_ID, node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(true))
           << node(0, node::MAX_ID, node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(1, node::ptr_t::MAX_ID));
      }

      /*
                     1           ---- 21
                    / \
                    2 3          ---- 42
                    |X|
                    F T
      */
      levelized_file<node> nf_21xor42;
      {
        node_writer nw(nf_21xor42);
        nw << node(42, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(42, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(false))
           << node(21, node::MAX_ID, node::ptr_t(42, node::ptr_t::MAX_ID), node::ptr_t(42, node::ptr_t::MAX_ID-1));
      }

      /*
                      _1_        ---- x0
                     /   \
                     2   3       ---- x1
                    / \ / \
                    T  4  T      ---- x2
                      / \
                      F T
      */
      levelized_file<node> nf_0xnor1_or_2;
      {
        node_writer nw(nf_0xnor1_or_2);
        nw << node(2, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(1, node::MAX_ID, node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(true))
           << node(1, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(2, node::ptr_t::MAX_ID))
           << node(0, node::MAX_ID, node::ptr_t(1, node::ptr_t::MAX_ID-1), node::ptr_t(1, node::ptr_t::MAX_ID));
      }

      /*
                      1         ---- x0
                     _X_
                    2   3       ---- x1
                   / \ / \
                   T  4  T      ---- x2
                     / \
                     F T
      */
      levelized_file<node> nf_0xor1_or_2;
      {
        node_writer nw(nf_0xor1_or_2);
        nw << node(2, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
           << node(1, node::MAX_ID, node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(true))
           << node(1, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(2, node::ptr_t::MAX_ID))
           << node(0, node::MAX_ID, node::ptr_t(1, node::ptr_t::MAX_ID), node::ptr_t(1, node::ptr_t::MAX_ID-1));
      }

      /*
                      1    ---- x0
                      X
                     2 3   ---- x1
                     |X|
                     4 5   ---- x2
                     |X|
                     6 7   ---- x3
                     |X|
                     8 9   ---- x4
                     |X|
                     F T
      */
      levelized_file<node> nf_sum01234_mod2;
      {
        // In comments, we provide the sum (mod 2) before adding the
        // respective variable.
        node_writer nw(nf_sum01234_mod2);
        nw << node(4, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))                                            // 0
           << node(4, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(false))                                            // 1
           << node(3, node::MAX_ID,   node::ptr_t(4, node::ptr_t::MAX_ID-1), node::ptr_t(4, node::ptr_t::MAX_ID))    // 1
           << node(3, node::MAX_ID-1, node::ptr_t(4, node::ptr_t::MAX_ID),   node::ptr_t(4, node::ptr_t::MAX_ID-1))  // 0
           << node(2, node::MAX_ID,   node::ptr_t(3, node::ptr_t::MAX_ID-1), node::ptr_t(3, node::ptr_t::MAX_ID))    // 0
           << node(2, node::MAX_ID-1, node::ptr_t(3, node::ptr_t::MAX_ID),   node::ptr_t(3, node::ptr_t::MAX_ID-1))  // 1
           << node(1, node::MAX_ID,   node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(2, node::ptr_t::MAX_ID))    // 1
           << node(1, node::MAX_ID-1, node::ptr_t(2, node::ptr_t::MAX_ID),   node::ptr_t(2, node::ptr_t::MAX_ID-1))  // 0
           << node(0, node::MAX_ID,   node::ptr_t(1, node::ptr_t::MAX_ID-1), node::ptr_t(1, node::ptr_t::MAX_ID));   // 0
      }

      // -------------------------------------------------------------------
      // Canonical node file where the max 1-level cut and max 2-level cuts
      // are NOT the same.

      /*
                       _1_      ---- x0
                      /   \
                      2   3     ---- x1
                     / \ / \
                    4   5  T    ---- x2
                   / \ / \
                   6  7  T      ---- x3
                  / \/ \
                  T  F T

                When ignoring arcs to terminals:
                  The maximum 1-level cut: 3
                  The maximum 2-level cut: 4 (1 extra in-going edge to (5))
      */
      levelized_file<node> nf_larger_2level_cut_A;
      {
        node_writer nw(nf_larger_2level_cut_A);
        nw << node(3, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
           << node(3, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(false))
           << node(2, node::MAX_ID,   node::ptr_t(3, node::ptr_t::MAX_ID),   node::ptr_t(true))
           << node(2, node::MAX_ID-1, node::ptr_t(3, node::ptr_t::MAX_ID-1), node::ptr_t(3, node::ptr_t::MAX_ID))
           << node(1, node::MAX_ID,   node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(true))
           << node(1, node::MAX_ID-1, node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(2, node::ptr_t::MAX_ID))
           << node(0, node::MAX_ID,   node::ptr_t(1, node::ptr_t::MAX_ID-1), node::ptr_t(1, node::ptr_t::MAX_ID));
      }

      /*
                       1      ---- x0
                      / \
                    _2_  \    ---- x1
                   /   \ |
                   4   3 |    ---- x2
                  / \  \\|
                  F T   5     ---- x3
                       / \
                       F T

               When ignoring arcs to terminals:
                  The maximum 1-level cut: 3
                  The maximum 2-level cut: 4 (1 in-going edge to (4))
      */
      levelized_file<node> nf_larger_2level_cut_B;
      {
        node_writer nw(nf_larger_2level_cut_B);
        nw << node(3, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
           << node(2, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
           << node(2, node::MAX_ID-1, node::ptr_t(3, node::ptr_t::MAX_ID), node::ptr_t(3, node::ptr_t::MAX_ID))
           << node(1, node::MAX_ID,   node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(2, node::ptr_t::MAX_ID-1))
           << node(0, node::MAX_ID,   node::ptr_t(1, node::ptr_t::MAX_ID), node::ptr_t(3, node::ptr_t::MAX_ID));
      }

      describe("canonicity", [&]() {
        it("is true for False terminal", [&]() {
          AssertThat(nf_F.canonical, Is().True());
        });

        it("is true for True terminal", [&]() {
          AssertThat(nf_T.canonical, Is().True());
        });

        it("is true for single node [1]", [&]() {
          AssertThat(nf_42.canonical, Is().True());
        });

        it("is true for single node [2]", [&]() {
          AssertThat(nf_not42.canonical, Is().True());
        });

        it("is false for single node due to too small Id", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(21, 42, node::ptr_t(false), node::ptr_t(true));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is true for x21 + x42", [&]() {
          AssertThat(nf_21xor42.canonical, Is().True());
        });

        it("is false if child ordering with terminals are mismatching", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(42, node::MAX_ID, node::ptr_t(true), node::ptr_t(false))
               << node(42, node::MAX_ID-1, node::ptr_t(false), node::ptr_t(true))
               << node(21, node::MAX_ID, node::ptr_t(42, node::ptr_t::MAX_ID), node::ptr_t(42, node::ptr_t::MAX_ID-1));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is false if id is not reset per level", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(42, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
               << node(42, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(false))
               << node(21, node::MAX_ID-2, node::ptr_t(42, node::ptr_t::MAX_ID), node::ptr_t(42, node::ptr_t::MAX_ID-1));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is false if id is decremented by more than one", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(42, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
               << node(42, node::MAX_ID-2, node::ptr_t(true), node::ptr_t(false))
               << node(21, node::MAX_ID, node::ptr_t(42, node::ptr_t::MAX_ID), node::ptr_t(42, node::ptr_t::MAX_ID-2));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is true for ~(x0 + x1) \\/ x2", [&]() {
          AssertThat(nf_0xnor1_or_2.canonical, Is().True());
        });

        it("is true for (x0 + x1) \\/ x2", [&]() {
          AssertThat(nf_0xor1_or_2.canonical, Is().True());
        });

        it("is false due to internal uid child is out-of-order compared to a terminal child", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(2, node::MAX_ID, node::ptr_t(false), node::ptr_t(true))
               << node(1, node::MAX_ID, node::ptr_t(true), node::ptr_t(2, node::ptr_t::MAX_ID))
               << node(1, node::MAX_ID-1, node::ptr_t(2, node::ptr_t::MAX_ID), node::ptr_t(false))
               << node(0, node::MAX_ID, node::ptr_t(1, node::ptr_t::MAX_ID), node::ptr_t(1, node::ptr_t::MAX_ID-1));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is false due to internal uid low-children are out-of-order", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(3, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
               << node(2, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
               << node(2, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(3, node::ptr_t::MAX_ID))
               << node(1, node::MAX_ID,   node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(2, node::ptr_t::MAX_ID))
               << node(1, node::MAX_ID-1, node::ptr_t(2, node::ptr_t::MAX_ID),   node::ptr_t(2, node::ptr_t::MAX_ID))
               << node(0, node::MAX_ID,   node::ptr_t(1, node::ptr_t::MAX_ID),   node::ptr_t(1, node::ptr_t::MAX_ID-1));
          }

          AssertThat(nf.canonical, Is().False());
        });

        it("is false due to internal uid high-children are out-of-order", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(3, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
               << node(2, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))
               << node(2, node::MAX_ID-1, node::ptr_t(true), node::ptr_t(3, node::ptr_t::MAX_ID))
               << node(1, node::MAX_ID,   node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(2, node::ptr_t::MAX_ID-1))
               << node(1, node::MAX_ID-1, node::ptr_t(2, node::ptr_t::MAX_ID-1), node::ptr_t(2, node::ptr_t::MAX_ID))
               << node(0, node::MAX_ID,   node::ptr_t(1, node::ptr_t::MAX_ID),   node::ptr_t(1, node::ptr_t::MAX_ID-1));
          }

          AssertThat(nf.canonical, Is().False());
        });
      });

      describe("width", [&]() {
        it("is 0 for the False terminal", [&]() {
          AssertThat(nf_F.width, Is().EqualTo(0u));
        });

        it("is 0 for True terminal", [&]() {
          AssertThat(nf_T.width, Is().EqualTo(0u));
        });

        it("is 1 for single node [1]", [&]() {
          AssertThat(nf_42.width, Is().EqualTo(1u));
        });

        it("is 1 for single node [2]", [&]() {
          AssertThat(nf_not42.width, Is().EqualTo(1u));
        });

        it("is 1 for single node [2]", [&]() {
          AssertThat(nf_42andnot42.width, Is().EqualTo(1u));
        });

        it("is 2 for x21 + x42", [&]() {
          AssertThat(nf_21xor42.width, Is().EqualTo(2u));
        });

        it("is 2 for ~(x0 + x1) \\/ x2", [&]() {
          AssertThat(nf_0xnor1_or_2.width, Is().EqualTo(2u));
        });

        it("is 2 for (x0 + x1 + x2 + x3) mod 2", [&]() {
          AssertThat(nf_sum01234_mod2.width, Is().EqualTo(2u));
        });

        it("is 3 for (x1 -> !x3) \\/ (x2 /\\ (x1 \\/ x3))", []() {
          /*
          //               1            ---- x0
          //              / \
          //             _2_ \          ---- x1
          //            /   \ \
          //            3   4  5        ---- x2
          //           / \ / \/ \
          //           F 6 F T  F       ---- x3
          //            / \
          //            F T
           */
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(3, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true))  // n6
               << node(2, node::MAX_ID,   node::ptr_t(true),  node::ptr_t(false)) // n5 (non-canonical)
               << node(2, node::MAX_ID-1, node::ptr_t(false), node::ptr_t(true))  // n4 (non-canonical)
               << node(2, node::MAX_ID-2, node::ptr_t(false), node::ptr_t(3, node::MAX_ID)) // n3
               << node(1, node::MAX_ID,   node::ptr_t(2, node::MAX_ID-2), node::ptr_t(2, node::MAX_ID-1)) // n2
               << node(0, node::MAX_ID,   node::ptr_t(1, node::MAX_ID),   node::ptr_t(2, node::MAX_ID));  // n1
          }

          AssertThat(nf.width, Is().EqualTo(3u));
        });

      });

      describe("number of terminals", [&]() {
        it("is [1,0] for the False terminal", [&]() {
          AssertThat(nf_F.number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(nf_F.number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("is [0,1] for True terminal", [&]() {
          AssertThat(nf_T.number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(nf_T.number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("is [1,1] for single node [1]", [&]() {
          AssertThat(nf_42.number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(nf_42.number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("is [1,1] for single node [2]", [&]() {
          AssertThat(nf_not42.number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(nf_not42.number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("is [0,2] for single node [2]", [&]() {
          AssertThat(nf_42andnot42.number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(nf_42andnot42.number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("is [2,2] for x21 + x42", [&]() {
          AssertThat(nf_21xor42.number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(nf_21xor42.number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("is [1,3] for ~(x0 + x1) \\/ x2", [&]() {
          AssertThat(nf_0xnor1_or_2.number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(nf_0xnor1_or_2.number_of_terminals[true],  Is().EqualTo(3u));
        });
      });

      describe("max 1-level cut", [&]() {
        it("is exact for F", [&]() {
          AssertThat(nf_F.max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(nf_F.max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_F.max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(nf_F.max_1level_cut[cut_type::All], Is().EqualTo(1u));
        });

        it("is exact for T", [&]() {
          AssertThat(nf_T.max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(nf_T.max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(nf_T.max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_T.max_1level_cut[cut_type::All], Is().EqualTo(1u));
        });

        it("is exact for x42", [&]() {
          AssertThat(nf_42.max_1level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::All], Is().EqualTo(2u));
        });

        it("is exact for ~x42", [&]() {
          AssertThat(nf_not42.max_1level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::All], Is().EqualTo(2u));
        });

        it("is exact for x0 & x1", [&]() {
          AssertThat(nf_0and1.max_1level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::Internal_False], Is().EqualTo(2u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::All], Is().EqualTo(3u));
        });

        it("is exact for x0 & x1 & 2", [&]() {
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::Internal_False], Is().EqualTo(3u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::All], Is().EqualTo(4u));
        });

        it("is exact for x0 & x1 | c2", [&]() {
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::Internal], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::Internal_False], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::Internal_True], Is().EqualTo(3u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::All], Is().EqualTo(3u));
        });

        it("is exact for x21 ^ x42", [&]() {
          AssertThat(nf_21xor42.max_1level_cut[cut_type::Internal], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::Internal_False], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::Internal_True], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::All], Is().EqualTo(4u));
        });

        it("is exact for (x0 + x1 + x2 + x3) mod 2", [&]() {
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::Internal], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::Internal_False], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::Internal_True], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::All], Is().EqualTo(4u));
        });
      });

      describe("2-level cut", [&]() {
        it("is exact for F", [&]() {
          AssertThat(nf_F.max_2level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(nf_F.max_2level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_F.max_2level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(nf_F.max_2level_cut[cut_type::All], Is().EqualTo(1u));
        });

        it("is exact for T", [&]() {
          AssertThat(nf_T.max_2level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(nf_T.max_2level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(nf_T.max_2level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_T.max_2level_cut[cut_type::All], Is().EqualTo(1u));
        });

        it("is exact for x42", [&]() {
          AssertThat(nf_42.max_2level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::All], Is().EqualTo(2u));
        });

        it("is exact for ~x42", [&]() {
          AssertThat(nf_not42.max_2level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::All], Is().EqualTo(2u));
        });

        // The maximum 1-level and maximum 2-level cuts are the same.
        it("is copy of 1-level for x0 & x1", [&]() {
          AssertThat(nf_0and1.max_2level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::Internal_False], Is().EqualTo(2u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::All], Is().EqualTo(3u));
        });

        it("is copy of 1-level for x0 & x1 & 2", [&]() {
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::Internal], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::Internal_False], Is().EqualTo(3u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::All], Is().EqualTo(4u));
        });

        it("is copy of 1-level for x0 & x1 | c2", [&]() {
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::Internal], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::Internal_False], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::Internal_True], Is().EqualTo(3u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::All], Is().EqualTo(3u));
        });

        it("is soundly upper bounded for x21 ^ x42", [&]() {
          AssertThat(nf_21xor42.max_2level_cut[cut_type::Internal], Is().EqualTo(2u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(nf_21xor42.max_2level_cut[cut_type::Internal_False], Is().LessThanOrEqualTo(4u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(nf_21xor42.max_2level_cut[cut_type::Internal_True], Is().LessThanOrEqualTo(4u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::All], Is().EqualTo(4u));
        });

        it("is soundly upper bounded for (x0 + x1 + x2 + x3 + x4) mod 2", [&]() {
          // Uses upper bounds derived from 1-level cut.
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal_False], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal_False], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::Internal_True], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::All], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::All], Is().LessThanOrEqualTo(8u));
        });

        // The maximum 2-level cut greater than the maximum 1-level cut.
        it("is soundly upper bounded when 2-level cut > 1-level cut [A]", [&]() {
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal_False], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal_False], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal_True], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::Internal_True], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::All], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::All], Is().LessThanOrEqualTo(8u));
        });

        it("is soundly upper bounded when 2-level cut > 1-level cut [B]", [&]() {
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal_False], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal_False], Is().LessThanOrEqualTo(7u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::Internal_True], Is().LessThanOrEqualTo(7u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::All], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::All], Is().LessThanOrEqualTo(8u));
        });
      });
    });

    describe("node_file::stats", []() {
      it("Is no terminal when empty", [&]() {
        levelized_file<node> nf;
        AssertThat(nf.empty(), Is().True());

        AssertThat(nf.is_terminal(), Is().False());
        AssertThat(nf.is_false(),    Is().False());
        AssertThat(nf.is_true(),     Is().False());
      });

      it("Recognises being the False terminal", [&]() {
        levelized_file<node> nf;
        {
          node_writer nw(nf);
          nw << node(false);
        }

        AssertThat(nf.is_terminal(), Is().True());
        AssertThat(nf.value(),       Is().False());
        AssertThat(nf.is_false(),    Is().True());
        AssertThat(nf.is_true(),     Is().False());
      });

      it("Recognises being the True terminal", [&]() {
        levelized_file<node> nf;
        {
          node_writer nw(nf);
          nw << node(true);
        }

        AssertThat(nf.is_terminal(), Is().True());
        AssertThat(nf.value(),       Is().True());
        AssertThat(nf.is_false(),    Is().False());
        AssertThat(nf.is_true(),     Is().True());
      });

      it("Recognises being a non-terminal [x1]", [&]() {
        levelized_file<node> nf;
        {
          node_writer nw(nf);
          nw << node(1,0, node::ptr_t(false), node::ptr_t(true));
        }

        AssertThat(nf.is_terminal(), Is().False());
        AssertThat(nf.is_false(),    Is().False());
        AssertThat(nf.is_true(),     Is().False());
      });

      it("Recognises being a non-terminal [~x0 & x1]", [&]() {
        levelized_file<node> nf;
        {
          node_writer nw(nf);
          nw << node(1,0, node::ptr_t(false), node::ptr_t(true))
             << node(0,0, node::ptr_t(1,0),   node::ptr_t(false));
        }

        AssertThat(nf.is_terminal(), Is().False());
        AssertThat(nf.is_false(),    Is().False());
        AssertThat(nf.is_true(),     Is().False());
      });
    });

    describe("node_writer + node_stream", []() {
      // TODO: stream: reads backwards by default
      // TODO: stream: reads forwards if desired

      describe(".seek(const node::uid_t &u)", []() {
        it("can seek in False sink", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(false);
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(0,0)), Is().EqualTo(node(false)));
          AssertThat(ns.seek(node::uid_t(1,0)), Is().EqualTo(node(false)));
          AssertThat(ns.seek(node::uid_t(1,1)), Is().EqualTo(node(false)));
          AssertThat(ns.seek(node::uid_t(node::MAX_LABEL, node::MAX_ID)), Is().EqualTo(node(false)));
        });

        it("can seek in True sink", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(true);
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(0,0)), Is().EqualTo(node(true)));
          AssertThat(ns.seek(node::uid_t(1,0)), Is().EqualTo(node(true)));
          AssertThat(ns.seek(node::uid_t(1,1)), Is().EqualTo(node(true)));
          AssertThat(ns.seek(node::uid_t(node::MAX_LABEL, node::MAX_ID)), Is().EqualTo(node(true)));
        });

        it("can seek existing elements", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(1,1, node::ptr_t(false), node::ptr_t(true))
               << node(1,0, node::ptr_t(true), node::ptr_t(false))
               << node(0,0, node::uid_t(1,0), node::uid_t(1,1))
              ;
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(0,0)), Is().EqualTo(node(0,0, node::uid_t(1,0),  node::uid_t(1,1))));
          AssertThat(ns.seek(node::uid_t(1,1)), Is().EqualTo(node(1,1, node::ptr_t(false), node::ptr_t(true))));
        });

        it("can seek non-existing element in the middle ", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(2,1, node::ptr_t(false), node::ptr_t(true))
               << node(2,0, node::ptr_t(true),  node::ptr_t(false))
               << node(0,0, node::ptr_t(1,0),   node::ptr_t(1,1))
              ;
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(0,0)), Is().EqualTo(node(0,0, node::ptr_t(1,0),   node::ptr_t(1,1))));
          AssertThat(ns.seek(node::uid_t(1,1)), Is().EqualTo(node(2,0, node::ptr_t(true),  node::ptr_t(false))));
          AssertThat(ns.seek(node::uid_t(1,2)), Is().EqualTo(node(2,0, node::ptr_t(true),  node::ptr_t(false))));
          AssertThat(ns.seek(node::uid_t(2,0)), Is().EqualTo(node(2,0, node::ptr_t(true),  node::ptr_t(false))));
          AssertThat(ns.seek(node::uid_t(2,1)), Is().EqualTo(node(2,1, node::ptr_t(false), node::ptr_t(true))));
        });

        it("can seek past end [1]", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(1,1, node::ptr_t(false), node::ptr_t(true))
               << node(1,0, node::ptr_t(true),  node::ptr_t(false))
               << node(0,0, node::ptr_t(1,0),   node::ptr_t(1,1))
              ;
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(node::MAX_LABEL, node::MAX_ID)),
                     Is().EqualTo(node(1,1, node::ptr_t(false), node::ptr_t(true))));
        });

        it("can seek past end [2]", [&]() {
          levelized_file<node> nf;
          {
            node_writer nw(nf);
            nw << node(1,1, node::ptr_t(false), node::ptr_t(true))
               << node(1,0, node::ptr_t(true),  node::ptr_t(false))
               << node(0,0, node::ptr_t(1,0),   node::ptr_t(1,1))
              ;
          }

          node_stream<> ns(nf);

          AssertThat(ns.seek(node::uid_t(0,0)), Is().EqualTo(node(0,0, node::ptr_t(1,0),   node::ptr_t(1,1))));
          AssertThat(ns.seek(node::uid_t(1,1)), Is().EqualTo(node(1,1, node::ptr_t(false), node::ptr_t(true))));
          AssertThat(ns.seek(node::uid_t(2,0)), Is().EqualTo(node(1,1, node::ptr_t(false), node::ptr_t(true))));
        });

        // TODO: reversed
      });

      describe("node_writer + node_random_access", []() {
        /*
        //              1      ---- A
        //             / \
        //             F T
        */
        const node A_n1 = node(1, node::MAX_ID, node::ptr_t(false), node::ptr_t(true));

        levelized_file<node> nf_A;
        {
          node_writer nw(nf_A);
          nw << A_n1;
        }

        /*
        //        _1_            ---- x0
        //       /   \
        //      _2_   \          ---- x1
        //     /   \   \
        //     3   4   5         ---- x2
        //    / \ / \ / \
        //    F  6   7  T        ---- x4
        //      / \ / \
        //      T F F T
         */
        const node B_n7 = node(4, node::MAX_ID,   node::ptr_t(false), node::ptr_t(true));
        const node B_n6 = node(4, node::MAX_ID-1, node::ptr_t(true),  node::ptr_t(false));
        const node B_n5 = node(2, node::MAX_ID,   B_n7.uid(),         node::ptr_t(true));
        const node B_n4 = node(2, node::MAX_ID-1, B_n6.uid(),         B_n7.uid());
        const node B_n3 = node(2, node::MAX_ID-2, node::ptr_t(false), B_n6.uid());
        const node B_n2 = node(1, node::MAX_ID,   B_n3.uid(),         B_n4.uid());
        const node B_n1 = node(0, node::MAX_ID,   B_n2.uid(),         B_n5.uid());

        levelized_file<node> nf_B;
        {
          node_writer nw(nf_B);
          nw << B_n7 << B_n6 << B_n5 << B_n4 << B_n3 << B_n2 << B_n1;
        }

        describe(".setup_next_level(...) + .[has_]current_level() + ", [&]() {
          it("has no levels for 'F' terminal", []() {
            levelized_file<node> nf;
            {
              node_writer nw(nf);
              nw << node(false);
            }

            node_random_access nra(nf);

            AssertThat(nra.has_current_level(), Is().False());
            AssertThat(nra.has_next_level(), Is().False());

            nra.setup_next_level(0u);
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(0u));
            AssertThat(nra.has_next_level(), Is().False());

            nra.setup_next_level(2u);
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(2u));
            AssertThat(nra.has_next_level(), Is().False());
          });

          it("has no levels for 'T' terminal", []() {
            levelized_file<node> nf;
            {
              node_writer nw(nf);
              nw << node(true);
            }

            node_random_access nra(nf);

            AssertThat(nra.has_current_level(), Is().False());
            AssertThat(nra.has_next_level(), Is().False());

            nra.setup_next_level(0u);
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(0u));
            AssertThat(nra.has_next_level(), Is().False());

            nra.setup_next_level(1u);
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(1u));
            AssertThat(nra.has_next_level(), Is().False());
          });

          it("has empty levels before root of 'x1'", [&]() {
            node_random_access nra(nf_A);

            AssertThat(nra.has_current_level(), Is().False());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(1u));

            nra.setup_next_level(0u);

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(0u));

            AssertThat(nra.empty_level(), Is().True());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(1u));
          });

          it("has empty levels after having skipped content of 'x1'", [&]() {
            node_random_access nra(nf_A);

            AssertThat(nra.has_current_level(), Is().False());

            AssertThat(nra.empty_level(), Is().True());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(1u));

            nra.setup_next_level(2u);

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(2u));

            AssertThat(nra.empty_level(), Is().True());

            AssertThat(nra.has_next_level(), Is().False());
          });

          it("sets up the first default 'next' level to be the root", [&]() {
            node_random_access nra(nf_B);

            AssertThat(nra.has_current_level(), Is().False());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(0u));

            nra.setup_next_level();
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(0u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(1u));
          });

          it("sets up consecutive default 'next' level to be the non-empty levels", [&]() {
            node_random_access nra(nf_B);

            AssertThat(nra.has_current_level(), Is().False());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(0u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(0u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(1u));

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(1u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(1u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(1u));

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(2u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(2u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(3u));

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(4u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(4u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(2u));

            AssertThat(nra.has_next_level(), Is().False());
          });

          it("can go to empty level in-between non-empty ones", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(3u);

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(3u));

            AssertThat(nra.empty_level(), Is().True());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(4u));
          });

          it("can go from empty level to the next non-empty one", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(3u);

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(4u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(4u));

            AssertThat(nra.empty_level(), Is().False());
          });
        });

        describe(".at(...)", [&]() {
          it("provides random access to root level of 'x1'", [&]() {
            node_random_access nra(nf_A);

            AssertThat(nra.has_current_level(), Is().False());

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(1u));

            nra.setup_next_level(1u);
            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(1u));

            AssertThat(nra.empty_level(), Is().False());
            AssertThat(nra.current_width(), Is().EqualTo(1u));

            AssertThat(nra.at(0u), Is().EqualTo(A_n1));
            AssertThat(nra.at(A_n1.uid()), Is().EqualTo(A_n1));

            AssertThat(nra.has_next_level(), Is().False());
          });

          it("provides random access to root [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(0u);

            AssertThat(nra.at(0u), Is().EqualTo(B_n1));
          });

          it("provides random access to root [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(0u);

            AssertThat(nra.at(B_n1.uid()), Is().EqualTo(B_n1));
          });

          it("provides random access to root [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(0u);

            AssertThat(nra.at(0u), Is().EqualTo(B_n1));
          });

          it("provides random access to root [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(0u);

            AssertThat(nra.at(B_n1.uid()), Is().EqualTo(B_n1));
          });

          it("provides random access to non-root single-node level [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(1u);

            AssertThat(nra.at(0u), Is().EqualTo(B_n2));
          });

          it("provides random access to non-root single-node level [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(1u);

            AssertThat(nra.at(B_n2.uid()), Is().EqualTo(B_n2));
          });

          it("provides in-order access to multi-node level in reverse [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(0u), Is().EqualTo(B_n3));
            AssertThat(nra.at(1u), Is().EqualTo(B_n4));
            AssertThat(nra.at(2u), Is().EqualTo(B_n5));
          });

          it("provides in-order random access to multi-node level [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(B_n5.uid()), Is().EqualTo(B_n5));
            AssertThat(nra.at(B_n4.uid()), Is().EqualTo(B_n4));
            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));
          });

          it("allows skipping over nodes on multi-node level [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(1u), Is().EqualTo(B_n4));
          });

          it("provides in-order random access to multi-node level [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(B_n4.uid()), Is().EqualTo(B_n4));
          });

          it("provides out-of-order random access to multi-node level [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(2u), Is().EqualTo(B_n5));
            AssertThat(nra.at(0u), Is().EqualTo(B_n3));
            AssertThat(nra.at(1u), Is().EqualTo(B_n4));
          });

          it("provides out-of-order random access to multi-node level [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));
            AssertThat(nra.at(B_n5.uid()), Is().EqualTo(B_n5));
            AssertThat(nra.at(B_n4.uid()), Is().EqualTo(B_n4));
          });

          it("provides recurring out-of-order random access to multi-node level [idx]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(0u), Is().EqualTo(B_n3));
            AssertThat(nra.at(2u), Is().EqualTo(B_n5));
            AssertThat(nra.at(1u), Is().EqualTo(B_n4));
            AssertThat(nra.at(0u), Is().EqualTo(B_n3));
            AssertThat(nra.at(2u), Is().EqualTo(B_n5));
            AssertThat(nra.at(0u), Is().EqualTo(B_n3));
            AssertThat(nra.at(1u), Is().EqualTo(B_n4));
          });

          it("provides recurring out-of-order random access to multi-node level [uid]", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);

            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));
            AssertThat(nra.at(B_n5.uid()), Is().EqualTo(B_n5));
            AssertThat(nra.at(B_n4.uid()), Is().EqualTo(B_n4));
            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));
            AssertThat(nra.at(B_n5.uid()), Is().EqualTo(B_n5));
            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));
            AssertThat(nra.at(B_n4.uid()), Is().EqualTo(B_n4));
          });

          it("provides access after having gone by default from an empty level", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(3u);

            AssertThat(nra.has_next_level(), Is().True());
            AssertThat(nra.next_level(), Is().EqualTo(4u));

            nra.setup_next_level();

            AssertThat(nra.has_current_level(), Is().True());
            AssertThat(nra.current_level(), Is().EqualTo(4u));

            AssertThat(nra.empty_level(), Is().False());

            AssertThat(nra.at(B_n7.uid()), Is().EqualTo(B_n7));
            AssertThat(nra.at(B_n6.uid()), Is().EqualTo(B_n6));
            AssertThat(nra.at(B_n6.uid()), Is().EqualTo(B_n6));
            AssertThat(nra.at(B_n7.uid()), Is().EqualTo(B_n7));
          });
        });

        describe(".root()", [&]() {
          it("provides root for 'F' terminal", []() {
            levelized_file<node> nf;
            {
              node_writer nw(nf);
              nw << node(false);
            }

            node_random_access nra(nf);

            AssertThat(nra.root(), Is().EqualTo(node::ptr_t(false)));
          });

          it("provides root for 'T' terminal", []() {
            levelized_file<node> nf;
            {
              node_writer nw(nf);
              nw << node(true);
            }

            node_random_access nra(nf);

            AssertThat(nra.root(), Is().EqualTo(node::ptr_t(true)));
          });

          it("provides root before accessing anything", [&]() {
            node_random_access nra(nf_B);

            AssertThat(nra.root(), Is().EqualTo(B_n1.uid()));
          });

          it("provides root after accessing node below it", [&]() {
            node_random_access nra(nf_B);
            nra.setup_next_level(2u);
            AssertThat(nra.at(B_n3.uid()), Is().EqualTo(B_n3));

            AssertThat(nra.root(), Is().EqualTo(B_n1.uid()));
          });
        });

        // TODO: reverse
      });
    });
  });
 });
