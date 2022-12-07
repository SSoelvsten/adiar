#include "../../../test.h"

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
          AssertThat(nf_F.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(nf_F.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_F.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
          AssertThat(nf_F.max_1level_cut[cut_type::ALL], Is().EqualTo(1u));
        });

        it("is exact for T", [&]() {
          AssertThat(nf_T.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(nf_T.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
          AssertThat(nf_T.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_T.max_1level_cut[cut_type::ALL], Is().EqualTo(1u));
        });

        it("is exact for x42", [&]() {
          AssertThat(nf_42.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_42.max_1level_cut[cut_type::ALL], Is().EqualTo(2u));
        });

        it("is exact for ~x42", [&]() {
          AssertThat(nf_not42.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_not42.max_1level_cut[cut_type::ALL], Is().EqualTo(2u));
        });

        it("is exact for x0 & x1", [&]() {
          AssertThat(nf_0and1.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_1level_cut[cut_type::ALL], Is().EqualTo(3u));
        });

        it("is exact for x0 & x1 & 2", [&]() {
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
        });

        it("is exact for x0 & x1 | c2", [&]() {
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
          AssertThat(nf_0and1_or_2.max_1level_cut[cut_type::ALL], Is().EqualTo(3u));
        });

        it("is exact for x21 ^ x42", [&]() {
          AssertThat(nf_21xor42.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
          AssertThat(nf_21xor42.max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
        });

        it("is exact for (x0 + x1 + x2 + x3) mod 2", [&]() {
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::INTERNAL], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
        });
      });

      describe("2-level cut", [&]() {
        it("is exact for F", [&]() {
          AssertThat(nf_F.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(nf_F.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_F.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
          AssertThat(nf_F.max_2level_cut[cut_type::ALL], Is().EqualTo(1u));
        });

        it("is exact for T", [&]() {
          AssertThat(nf_T.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(nf_T.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
          AssertThat(nf_T.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_T.max_2level_cut[cut_type::ALL], Is().EqualTo(1u));
        });

        it("is exact for x42", [&]() {
          AssertThat(nf_42.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_42.max_2level_cut[cut_type::ALL], Is().EqualTo(2u));
        });

        it("is exact for ~x42", [&]() {
          AssertThat(nf_not42.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_not42.max_2level_cut[cut_type::ALL], Is().EqualTo(2u));
        });

        // The maximum 1-level and maximum 2-level cuts are the same.
        it("is copy of 1-level for x0 & x1", [&]() {
          AssertThat(nf_0and1.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_0and1.max_2level_cut[cut_type::ALL], Is().EqualTo(3u));
        });

        it("is copy of 1-level for x0 & x1 & 2", [&]() {
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(nf_0and1and2.max_2level_cut[cut_type::ALL], Is().EqualTo(4u));
        });

        it("is copy of 1-level for x0 & x1 | c2", [&]() {
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
          AssertThat(nf_0and1_or_2.max_2level_cut[cut_type::ALL], Is().EqualTo(3u));
        });

        it("is soundly upper bounded for x21 ^ x42", [&]() {
          AssertThat(nf_21xor42.max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(nf_21xor42.max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(4u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(nf_21xor42.max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));

          AssertThat(nf_21xor42.max_2level_cut[cut_type::ALL], Is().EqualTo(4u));
        });

        it("is soundly upper bounded for (x0 + x1 + x2 + x3 + x4) mod 2", [&]() {
          // Uses upper bounds derived from 1-level cut.
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_sum01234_mod2.max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
        });

        // The maximum 2-level cut greater than the maximum 1-level cut.
        it("is soundly upper bounded when 2-level cut > 1-level cut [A]", [&]() {
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(8u));

          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_A.max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
        });

        it("is soundly upper bounded when 2-level cut > 1-level cut [B]", [&]() {
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(7u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(7u));

          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
          AssertThat(nf_larger_2level_cut_B.max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
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
    });
  });
 });
