#include <filesystem>

namespace adiar
{
  template <>
  struct FILE_CONSTANTS<int>
  {
    static constexpr size_t files = 2u;

    struct stats
    { /* No extra 'int' specific variables */ };
  };
}

go_bandit([]() {
  describe("adiar/file.h, adiar/file_stream.h, adiar/file_writer.h", [&]() {
    describe("adiar/file.h", [&]() {
      it("can construct fresh file<T> and make it read-only", [&]() {
        file<int> f;

        AssertThat(f.is_read_only(), Is().False());

        f.make_read_only();

        AssertThat(f.is_read_only(), Is().True());
      });

      it("can construct fresh __meta_file<T> and make it read-only", [&]() {
        __meta_file<int> f;

        AssertThat(f.is_read_only(), Is().False());

        f.make_read_only();

        AssertThat(f.is_read_only(), Is().True());
      });
    });

    simple_file<int> test_file_simple;
    simple_file<int> test_file_simple_sorted;

    meta_file<int> test_file_meta_1;
    meta_file<int> test_file_meta_2;

    node_file node_test_file;

    node_t n2 = node(1,1, ptr_uint64(false), ptr_uint64(true));
    node_t n1 = node(1,0, ptr_uint64(true), ptr_uint64(false));
    node_t n0 = node(0,0, ptr_uint64(1,1), ptr_uint64(1,0));

    arc_file arc_test_file;

    arc_t node_arc_1 = { ptr_uint64(0,0), ptr_uint64(1,0) };
    arc_t terminal_arc_1 = { flag(ptr_uint64(0,0)), ptr_uint64(false) };

    arc_t terminal_arc_2 = { ptr_uint64(1,0), ptr_uint64(true) };
    arc_t terminal_arc_3 = { flag(ptr_uint64(1,0)), ptr_uint64(false) };

    describe("adiar/file_writer", [&]() {
      describe("simple_file_writer", [&]() {
        it("can hook into and write to test_file_simple", [&]() {
          AssertThat(test_file_simple.is_read_only(), Is().False());

          simple_file_writer<int> fw(test_file_simple);

          fw.unsafe_push(21);
          fw.unsafe_push(42);

          AssertThat(test_file_simple.is_read_only(), Is().False());
        });

        it("can sort after pushing out-of-order", [&]() {
          AssertThat(test_file_simple_sorted.is_read_only(), Is().False());

          simple_file_writer<int, std::less<int>> fw(test_file_simple_sorted);

          fw.unsafe_push(5);
          fw.unsafe_push(3);
          fw.unsafe_push(4);
          fw.unsafe_push(1);
          fw.unsafe_push(2);

          fw.sort();

          AssertThat(test_file_simple_sorted.is_read_only(), Is().False());
        });

        it("does not break when source file is destructed early [simple_file]", [&]() {
          simple_file<int>* f = new simple_file<int>();
          simple_file_writer<int> fw(*f);

          fw.unsafe_push(21);

          delete f;

          fw.unsafe_push(42);
        });
      });

      describe("meta_file_writer", [&]() {
        it("can hook into and write to test_file_meta_1", [&]() {
          AssertThat(test_file_meta_1.is_read_only(), Is().False());

          meta_file_writer<int> fw(test_file_meta_1);

          fw.unsafe_push(21);
          fw.unsafe_push(42);

          fw.unsafe_push(create_level_info(0,2u));

          AssertThat(test_file_meta_1.is_read_only(), Is().False());
        });


        it("can hook into and write to test_file_meta_2", [&]() {
          meta_file_writer<int> fw(test_file_meta_2);

          fw.unsafe_push(create_level_info(5,1u));
          fw.unsafe_push(create_level_info(4,1u));

          fw.unsafe_push(1); // Check idx argument is defaulted to 0
          fw.unsafe_push(2, 0);
          fw.unsafe_push(4, 1);
          fw.unsafe_push(5, 1);
          fw.unsafe_push(3, 0);

          AssertThat(test_file_meta_2.is_read_only(), Is().False());
        });

        it("does not break when source file is destructed early [simple_file]", [&]() {
          meta_file<int>* f = new meta_file<int>();
          meta_file_writer fw(*f);

          fw.unsafe_push(21);

          delete f;

          fw.unsafe_push(42);
          fw.unsafe_push(create_level_info(0,1u));
        });

        describe("node_writer", [&]() {
          const ptr_uint64 terminal_F = ptr_uint64(false);
          const ptr_uint64 terminal_T = ptr_uint64(true);

          it("can hook into and write to node_test_file", [&]() {
            AssertThat(node_test_file.is_read_only(), Is().False());

            node_writer nw(node_test_file);
            nw << n2 << n1 << n0;

            AssertThat(node_test_file.is_read_only(), Is().False());
          });

          // -------------------------------------------------------------------
          // Canonical node files where the max 1-level cut and max 2-level cuts
          // are the same.
          /*
                    F
           */
          node_file nf_F;
          {
            node_writer nw(nf_F);
            nw << node(false);
          }

          /*
                    T
          */
          node_file nf_T;
          {
            node_writer nw(nf_T);
            nw << node(true);
          }

          /*
                    1           ---- 42
                   / \
                   F T
           */
          node_file nf_42;
          {
            node_writer nw(nf_42);
            nw << node(42, MAX_ID, terminal_F, terminal_T);
          }

          /*
                    1           ---- 42
                   / \
                   T F
          */
          node_file nf_not42;
          {
            node_writer nw(nf_not42);
            nw << node(42, MAX_ID, terminal_F, terminal_T);
          }

          /*
                    1            ---- x0
                   / \
                   F 2           ---- x1
                    / \
                    F T
           */
          node_file nf_0and1;
          {
            node_writer nw(nf_0and1);
            nw << node(1, MAX_ID, terminal_F, terminal_T)
               << node(0, MAX_ID, terminal_F, ptr_uint64(1,MAX_ID));
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
          node_file nf_0and1and2;
          {
            node_writer nw(nf_0and1and2);
            nw << node(2, MAX_ID, terminal_F, terminal_T)
               << node(1, MAX_ID, terminal_F, ptr_uint64(2,MAX_ID))
               << node(0, MAX_ID, terminal_F, ptr_uint64(1,MAX_ID));
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
          node_file nf_0and1_or_2;
          {
            node_writer nw(nf_0and1_or_2);
            nw << node(2, MAX_ID, terminal_F, terminal_T)
               << node(1, MAX_ID, ptr_uint64(2,MAX_ID), terminal_T)
               << node(0, MAX_ID, ptr_uint64(2,MAX_ID), ptr_uint64(1,MAX_ID));
          }

          /*
                     1           ---- 21
                    / \
                    2 3          ---- 42
                    |X|
                    F T
          */
          node_file nf_21xor42;
          {
            node_writer nw(nf_21xor42);
            nw << node(42, MAX_ID, terminal_F, terminal_T)
               << node(42, MAX_ID-1, terminal_T, terminal_F)
               << node(21, MAX_ID, ptr_uint64(42, MAX_ID), ptr_uint64(42, MAX_ID-1));
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
          node_file nf_0xnor1_or_2;
          {
            node_writer nw(nf_0xnor1_or_2);
            nw << node(2, MAX_ID, terminal_F, terminal_T)
               << node(1, MAX_ID, ptr_uint64(2, MAX_ID), terminal_T)
               << node(1, MAX_ID-1, terminal_T, ptr_uint64(2, MAX_ID))
               << node(0, MAX_ID, ptr_uint64(1, MAX_ID-1), ptr_uint64(1, MAX_ID));
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
          node_file nf_0xor1_or_2;
          {
            node_writer nw(nf_0xor1_or_2);
            nw << node(2, MAX_ID, terminal_F, terminal_T)
               << node(1, MAX_ID, ptr_uint64(2, MAX_ID), terminal_T)
               << node(1, MAX_ID-1, terminal_T, ptr_uint64(2, MAX_ID))
               << node(0, MAX_ID, ptr_uint64(1, MAX_ID), ptr_uint64(1, MAX_ID-1));
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
          node_file nf_sum01234_mod2;
          {
            // In comments, we provide the sum (mod 2) before adding the
            // respective variable.
            node_writer nw(nf_sum01234_mod2);
            nw << node(4, MAX_ID,   terminal_F, terminal_T)                                            // 0
               << node(4, MAX_ID-1, terminal_T, terminal_F)                                            // 1
               << node(3, MAX_ID,   ptr_uint64(4,MAX_ID-1), ptr_uint64(4,MAX_ID))    // 1
               << node(3, MAX_ID-1, ptr_uint64(4,MAX_ID),   ptr_uint64(4,MAX_ID-1))  // 0
               << node(2, MAX_ID,   ptr_uint64(3,MAX_ID-1), ptr_uint64(3,MAX_ID))    // 0
               << node(2, MAX_ID-1, ptr_uint64(3,MAX_ID),   ptr_uint64(3,MAX_ID-1))  // 1
               << node(1, MAX_ID,   ptr_uint64(2,MAX_ID-1), ptr_uint64(2,MAX_ID))    // 1
               << node(1, MAX_ID-1, ptr_uint64(2,MAX_ID),   ptr_uint64(2,MAX_ID-1))  // 0
               << node(0, MAX_ID,   ptr_uint64(1,MAX_ID-1),   ptr_uint64(1,MAX_ID)); // 0
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
          node_file nf_larger_2level_cut_A;
          {
            node_writer nw(nf_larger_2level_cut_A);
            nw << node(3, MAX_ID,   terminal_F, terminal_T)
               << node(3, MAX_ID-1, terminal_T, terminal_F)
               << node(2, MAX_ID,   ptr_uint64(3,MAX_ID),   terminal_T)
               << node(2, MAX_ID-1, ptr_uint64(3,MAX_ID-1), ptr_uint64(3,MAX_ID))
               << node(1, MAX_ID,   ptr_uint64(2,MAX_ID-1), terminal_T)
               << node(1, MAX_ID-1, ptr_uint64(2,MAX_ID-1), ptr_uint64(2,MAX_ID))
               << node(0, MAX_ID,   ptr_uint64(1,MAX_ID-1), ptr_uint64(1,MAX_ID));
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
          node_file nf_larger_2level_cut_B;
          {
            node_writer nw(nf_larger_2level_cut_B);
            nw << node(3, MAX_ID,   terminal_F, terminal_T)
               << node(2, MAX_ID,   terminal_F, terminal_T)
               << node(2, MAX_ID-1, ptr_uint64(3,MAX_ID), ptr_uint64(3,MAX_ID))
               << node(1, MAX_ID,   ptr_uint64(2,MAX_ID), ptr_uint64(2,MAX_ID-1))
               << node(0, MAX_ID,   ptr_uint64(1,MAX_ID), ptr_uint64(3,MAX_ID));
          }

          describe("canonicity", [&]() {
            it("is true for False terminal", [&]() {
              AssertThat(is_canonical(nf_F), Is().True());
            });

            it("is true for True terminal", [&]() {
              AssertThat(is_canonical(nf_T), Is().True());
            });

            it("is true for single node [1]", [&]() {
              AssertThat(is_canonical(nf_42), Is().True());
            });

            it("is true for single node [2]", [&]() {
              AssertThat(is_canonical(nf_not42), Is().True());
            });

            it("is false for single node due to too small Id", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(21, 42, terminal_F, terminal_T);
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is true for x21 + x42", [&]() {
              AssertThat(is_canonical(nf_21xor42), Is().True());
            });

            it("is false if child ordering with terminals are mismatching", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(42, MAX_ID, terminal_T, terminal_F)
                   << node(42, MAX_ID-1, terminal_F, terminal_T)
                   << node(21, MAX_ID, ptr_uint64(42, MAX_ID), ptr_uint64(42, MAX_ID-1));
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is false if id is not reset per level", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(42, MAX_ID, terminal_F, terminal_T)
                   << node(42, MAX_ID-1, terminal_T, terminal_F)
                   << node(21, MAX_ID-2, ptr_uint64(42, MAX_ID), ptr_uint64(42, MAX_ID-1));
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is false if id is decremented by more than one", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(42, MAX_ID, terminal_F, terminal_T)
                   << node(42, MAX_ID-2, terminal_T, terminal_F)
                   << node(21, MAX_ID, ptr_uint64(42, MAX_ID), ptr_uint64(42, MAX_ID-2));
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is true for ~(x0 + x1) \\/ x2", [&]() {
              AssertThat(is_canonical(nf_0xnor1_or_2), Is().True());
            });

            it("is true for (x0 + x1) \\/ x2", [&]() {
              AssertThat(is_canonical(nf_0xor1_or_2), Is().True());
            });

            it("is false due to internal uid child is out-of-order compared to a terminal child", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(2, MAX_ID, terminal_F, terminal_T)
                   << node(1, MAX_ID, terminal_T, ptr_uint64(2, MAX_ID))
                   << node(1, MAX_ID-1, ptr_uint64(2, MAX_ID), terminal_F)
                   << node(0, MAX_ID, ptr_uint64(1, MAX_ID), ptr_uint64(1, MAX_ID-1));
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is false due to internal uid low-children are out-of-order", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(3, MAX_ID,   terminal_F, terminal_T)
                   << node(2, MAX_ID,   terminal_F, terminal_T)
                   << node(2, MAX_ID-1, terminal_T, ptr_uint64(3, MAX_ID))
                   << node(1, MAX_ID,   ptr_uint64(2, MAX_ID-1), ptr_uint64(2, MAX_ID))
                   << node(1, MAX_ID-1, ptr_uint64(2, MAX_ID),   ptr_uint64(2, MAX_ID))
                   << node(0, MAX_ID,   ptr_uint64(1, MAX_ID),   ptr_uint64(1, MAX_ID-1));
              }

              AssertThat(is_canonical(nf), Is().False());
            });

            it("is false due to internal uid high-children are out-of-order", [&]() {
              node_file nf;
              {
                node_writer nw(nf);
                nw << node(3, MAX_ID,   terminal_F, terminal_T)
                   << node(2, MAX_ID,   terminal_F, terminal_T)
                   << node(2, MAX_ID-1, terminal_T, ptr_uint64(3, MAX_ID))
                   << node(1, MAX_ID,   ptr_uint64(2, MAX_ID-1), ptr_uint64(2, MAX_ID-1))
                   << node(1, MAX_ID-1, ptr_uint64(2, MAX_ID-1), ptr_uint64(2, MAX_ID))
                   << node(0, MAX_ID,   ptr_uint64(1, MAX_ID),   ptr_uint64(1, MAX_ID-1));
              }

              AssertThat(is_canonical(nf), Is().False());
            });
          });

          describe("max 1-level cut", [&]() {
            it("is exact for F", [&]() {
              AssertThat(nf_F->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
              AssertThat(nf_F->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_F->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
              AssertThat(nf_F->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));
            });

            it("is exact for T", [&]() {
              AssertThat(nf_T->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
              AssertThat(nf_T->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
              AssertThat(nf_T->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_T->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));
            });

            it("is exact for x42", [&]() {
              AssertThat(nf_42->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_42->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_42->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_42->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));
            });

            it("is exact for ~x42", [&]() {
              AssertThat(nf_not42->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_not42->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_not42->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_not42->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));
            });

            it("is exact for x0 & x1", [&]() {
              AssertThat(nf_0and1->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_0and1->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
              AssertThat(nf_0and1->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_0and1->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));
            });

            it("is exact for x0 & x1 & 2", [&]() {
              AssertThat(nf_0and1and2->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_0and1and2->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
              AssertThat(nf_0and1and2->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_0and1and2->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
            });

            it("is exact for x0 & x1 | c2", [&]() {
              AssertThat(nf_0and1_or_2->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
              AssertThat(nf_0and1_or_2->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
              AssertThat(nf_0and1_or_2->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
              AssertThat(nf_0and1_or_2->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));
            });

            it("is exact for x21 ^ x42", [&]() {
              AssertThat(nf_21xor42->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
              AssertThat(nf_21xor42->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
              AssertThat(nf_21xor42->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
              AssertThat(nf_21xor42->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
            });

            it("is exact for (x0 + x1 + x2 + x3) mod 2", [&]() {
              AssertThat(nf_sum01234_mod2->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));
            });
          });

          describe("2-level cut", [&]() {
            it("is exact for F", [&]() {
              AssertThat(nf_F->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
              AssertThat(nf_F->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_F->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
              AssertThat(nf_F->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));
            });

            it("is exact for T", [&]() {
              AssertThat(nf_T->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
              AssertThat(nf_T->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
              AssertThat(nf_T->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_T->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));
            });

            it("is exact for x42", [&]() {
              AssertThat(nf_42->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_42->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_42->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_42->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));
            });

            it("is exact for ~x42", [&]() {
              AssertThat(nf_not42->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_not42->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
              AssertThat(nf_not42->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_not42->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));
            });

            // The maximum 1-level and maximum 2-level cuts are the same.
            it("is copy of 1-level for x0 & x1", [&]() {
              AssertThat(nf_0and1->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_0and1->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
              AssertThat(nf_0and1->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_0and1->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));
            });

            it("is copy of 1-level for x0 & x1 & 2", [&]() {
              AssertThat(nf_0and1and2->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
              AssertThat(nf_0and1and2->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
              AssertThat(nf_0and1and2->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
              AssertThat(nf_0and1and2->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));
            });

            it("is copy of 1-level for x0 & x1 | c2", [&]() {
              AssertThat(nf_0and1_or_2->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
              AssertThat(nf_0and1_or_2->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
              AssertThat(nf_0and1_or_2->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
              AssertThat(nf_0and1_or_2->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));
            });

            it("is soundly upper bounded for x21 ^ x42", [&]() {
              AssertThat(nf_21xor42->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));

              AssertThat(nf_21xor42->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
              AssertThat(nf_21xor42->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(4u));

              AssertThat(nf_21xor42->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
              AssertThat(nf_21xor42->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));

              AssertThat(nf_21xor42->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));
            });

            it("is soundly upper bounded for (x0 + x1 + x2 + x3 + x4) mod 2", [&]() {
              // Uses upper bounds derived from 1-level cut.
              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(8u));

              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(8u));

              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_sum01234_mod2->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
            });

            // The maximum 2-level cut greater than the maximum 1-level cut.
            it("is soundly upper bounded when 2-level cut > 1-level cut [A]", [&]() {
              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(6u));

              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(5u));
              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(8u));

              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
              AssertThat(nf_larger_2level_cut_A->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
            });

            it("is soundly upper bounded when 2-level cut > 1-level cut [B]", [&]() {
              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(7u));

              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(7u));

              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
              AssertThat(nf_larger_2level_cut_B->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));
            });
          });
        });

        describe("arc_writer", [&]() {
          arc_writer aw(arc_test_file);

          it("can hook into arc_test_file and write node arcs", [&]() {
            AssertThat(arc_test_file.is_read_only(), Is().False());

            aw.unsafe_push_node(node_arc_1);
          });

          it("can hook into arc_test_file and write level_info", [&]() {
            aw.unsafe_push(create_level_info(0,1u));
            aw.unsafe_push(create_level_info(1,1u));
          });

          it("can hook into arc_test_file and  write terminal arcs out of order", [&]() {
            aw.unsafe_push_terminal(terminal_arc_3);
            aw.unsafe_push_terminal(terminal_arc_1);
            aw.unsafe_push_terminal(terminal_arc_2);
          });
        });
      });
    });

    describe("adiar/file_stream.h", [&]() {
      describe("simple_file_stream", [&]() {
        it("locks the file to be read-only on attachment", [&]() {
          AssertThat(test_file_simple.is_read_only(), Is().False());

          file_stream<int, false> fs(test_file_simple);

          AssertThat(test_file_simple.is_read_only(), Is().True());
        });

        it("can read test_file_simple [forwards]", [&]() {
          file_stream<int, false> fs(test_file_simple);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_simple [forwards]", [&]() {
          file_stream<int, false> fs(test_file_simple);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_simple_sorted [reverse]", [&]() {
          file_stream<int, true> fs(test_file_simple_sorted);

          // This also tests, whether test_file_simple_sorted actually
          // ended up being sorted.
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(5));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(4));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(2));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(1));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("does not break when source simple_file is destructed early", [&]() {
          simple_file<int>* f = new simple_file<int>();

          { // Garbage collect the writer early, releasing it's reference counter
            simple_file_writer<int> fw(*f);
            fw.unsafe_push(21);
          }

          file_stream<int, false> fs(*f);

          delete f;

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
        });
      });

      describe("meta_file_stream", [&]() {
        it("locks the file to be read-only on attachment", [&]() {
          AssertThat(test_file_meta_1.is_read_only(), Is().False());
          meta_file_stream<int, 0, false> fs1(test_file_meta_1);

          AssertThat(test_file_meta_1.is_read_only(), Is().True());

          AssertThat(test_file_meta_2.is_read_only(), Is().False());
          meta_file_stream<int, 0, false> fs2(test_file_meta_2);

          AssertThat(test_file_meta_2.is_read_only(), Is().True());
        });

        it("can read test_file_meta_1 [forwards]", [&]() {
          meta_file_stream<int, 0, false> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_meta_1 [forwards]", [&]() {
          meta_file_stream<int, 0, false> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_meta_1 [reverse]", [&]() {
          meta_file_stream<int, 0, true> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_1 [reverse]", [&]() {
          meta_file_stream<int, 0, true> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_meta_2 [forwards]", [&]() {
          meta_file_stream<int, 0, false> fs1(test_file_meta_2);

          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(1));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(2));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(3));
          AssertThat(fs1.can_pull(), Is().False());

          meta_file_stream<int, 1, false> fs2(test_file_meta_2);

          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.pull(), Is().EqualTo(4));
          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.pull(), Is().EqualTo(5));
          AssertThat(fs2.can_pull(), Is().False());
        });

        it("can peek test_file_meta_2 [forwards]", [&]() {
          meta_file_stream<int, 0, false> fs1(test_file_meta_2);

          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(1));
          AssertThat(fs1.pull(), Is().EqualTo(1));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(2));
          AssertThat(fs1.pull(), Is().EqualTo(2));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(3));
          AssertThat(fs1.pull(), Is().EqualTo(3));
          AssertThat(fs1.can_pull(), Is().False());

          meta_file_stream<int, 1, false> fs2(test_file_meta_2);

          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.peek(), Is().EqualTo(4));
          AssertThat(fs2.pull(), Is().EqualTo(4));
          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.peek(), Is().EqualTo(5));
          AssertThat(fs2.pull(), Is().EqualTo(5));
          AssertThat(fs2.can_pull(), Is().False());
        });

        it("can read test_file_1 forwards via fresh proxy object", [&]() {
          meta_file<int> proxy(test_file_meta_1);
          meta_file_stream<int, 0, false> fs(proxy);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("does not break when source meta_file is destructed early", [&]() {
          meta_file<int>* f = new meta_file<int>();

          { // Garbage collect the writer early, releasing it's reference counter
            meta_file_writer<int> fw(*f);
            fw.unsafe_push(21, 0);
          }

          meta_file_stream<int, 0, false> fs(*f);

          delete f;

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
        });

        describe("level_info_stream", [&]() {
          it("can read level_info stream of test_file_meta_1", [&]() {
            level_info_stream ms(test_file_meta_1);

            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,2u)));
            AssertThat(ms.can_pull(), Is().False());
          });

          it("can read level_info stream of node_test_file", [&]() {
            level_info_stream ms(node_test_file);

            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));
            AssertThat(ms.can_pull(), Is().False());
          });
        });

        describe("node_stream", [&]() {
          it("can read node stream of node_test_file", [&]() {
            node_stream<> ns(node_test_file);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n0));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n1));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n2));
            AssertThat(ns.can_pull(), Is().False());
          });

          it("can read negated node stream of node_test_file", [&]() {
            node_stream<> ns(node_test_file, true);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n0));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n1));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n2));
            AssertThat(ns.can_pull(), Is().False());
          });
        });

        describe("arc_streams", [&]() {
          it("can read node arcs of arc_test_file", [&]() {
            node_arc_stream<> as(arc_test_file);

            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(node_arc_1));
            AssertThat(as.can_pull(), Is().False());
          });

          it("can read terminal arcs of arc_test_file", [&]() {
            terminal_arc_stream<> as(arc_test_file);

            // The stream has been sorted 1, 2, 3 yet the
            // terminal_arc_stream is per default in reverse, so we will
            // expect 3, 2, 1.
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(terminal_arc_3));
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(terminal_arc_2));
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(terminal_arc_1));
            AssertThat(as.can_pull(), Is().False());
          });
        });
      });
    });

    describe("shared_file", [&]() {
      describe("simple_file", [&]() {
        it("can return a written to simple_file, that then can be read", [&]() {
          auto t = []() {
            simple_file<int> f;

            simple_file_writer<int> fw(f);
            fw.unsafe_push(42);
            fw.unsafe_push(7);

            return f;
          };

          simple_file<int> f = t();
          file_stream<int> fs(f);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can construct a new named simple_file", [&]() {
          simple_file<int> file("simple_file_test.adiar");

          AssertThat(std::filesystem::exists("simple_file_test.adiar"), Is().True());

          {
            simple_file_writer<int> fw(file);
            fw.push(1);
            fw.push(2);
            fw.push(3);
          }
          AssertThat(file.is_read_only(), Is().False());

          {
            file_stream<int, false> fs(file);
            AssertThat(file.is_read_only(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(1));
            AssertThat(fs.pull(), Is().EqualTo(2));
            AssertThat(fs.pull(), Is().EqualTo(3));
            AssertThat(fs.can_pull(), Is().False());
          }
          AssertThat(file.is_read_only(), Is().True());
        });

        it("should have two temporary simple_files be two different files", [&]() {
          simple_file<int> file1;
          simple_file<int> file2;

          AssertThat(file1._file_ptr, Is().Not().EqualTo(file2._file_ptr));
        });

        it("can construct a copy of a simple_file", [&]() {
          simple_file<int> file1;

          { // Garbage collect the writer to detach it before the reader
            simple_file_writer<int> fw(file1);
            fw.push(42);
          }

          simple_file<int> file2(file1);

          file_stream<int, false> fs(file2);
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can construct a prior named simple_file (i.e. reopen a stored file)", [&]() {
          AssertThat(std::filesystem::exists("simple_file_test.adiar"), Is().True());
          simple_file<int> file("simple_file_test.adiar");
          AssertThat(file.is_read_only(), Is().True());

          file_stream<int, false> fs(file);
          AssertThat(file.is_read_only(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(1));
          AssertThat(fs.pull(), Is().EqualTo(2));
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can compute sizes of test_file_simple", [&]() {
          AssertThat(test_file_simple.size(), Is().EqualTo(2u));
          AssertThat(test_file_simple.file_size(), Is().EqualTo(2u * sizeof(int)));
        });

        it("can sort a yet unread simple_file", [&]() {
          simple_file<int> file;

          { // Garbage collect the writer to detach it before the reader
            simple_file_writer<int> fw(file);
            fw.push(42);
            fw.push(7);
            fw.push(21);
          }

          simple_file_sorter<int, std::less<>>::sort(file);

          file_stream<int, false> fs(file);

          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });
      });

      describe("meta_file", [&]() {
        it("can return a written to meta_file, that then can be read", [&]() {
          auto t = []() {
            meta_file<int> f;

            meta_file_writer<int> fw(f);
            fw.unsafe_push(42);
            fw.unsafe_push(7);

            return f;
          };

          meta_file<int> f = t();
          meta_file_stream<int, 0, false> fs(f);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("should have two temporary meta_files be two different files", [&]() {
          meta_file<int> file1;
          meta_file<int> file2;

          AssertThat(file1._file_ptr, Is().Not().EqualTo(file2._file_ptr));
        });

        it("can construct a copy of a meta_file", [&]() {
          meta_file<int> file1;

          { // Garbage collect the writer to detach it before the reader
            meta_file_writer<int> fw(file1);
            fw.unsafe_push(42);
          }

          meta_file<int> file2(file1);

          meta_file_stream<int, 0, false> fs(file2);
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can compute sizes of test_file_meta_1", [&]() {
          AssertThat(test_file_meta_1.size(), Is().EqualTo(2u));
          AssertThat(test_file_meta_1.file_size(), Is().EqualTo(2u * sizeof(int) + 1u * sizeof(level_info_t)));
        });

        it("can compute size of test_file_meta_2", [&]() {
          test_file_meta_2.make_read_only();
          AssertThat(test_file_meta_2.size(), Is().EqualTo(5u));
          AssertThat(test_file_meta_2.file_size(), Is().EqualTo(5u * sizeof(int) + 2u * sizeof(level_info_t)));
        });

        describe("node_file", [&]() {
          node_file x0;

          {
            node_writer nw_0(x0);
            nw_0 << node(0,MAX_ID,
                                ptr_uint64(false),
                                ptr_uint64(true));
          }

          node_file x0_and_x1;

          {
            node_writer nw_01(x0_and_x1);

            nw_01 << node(1, MAX_ID,
                                 ptr_uint64(false),
                                 ptr_uint64(true));

            nw_01 << node(0, MAX_ID,
                                 ptr_uint64(false),
                                 ptr_uint64(1, MAX_ID));
          }

          node_file terminal_T;

          {
            node_writer nw_T(terminal_T);
            nw_T << node(true);
          }

          node_file terminal_F;

          {
            node_writer nw_F(terminal_F);
            nw_F << node(false);
          }

          describe("size computation", [&]() {
            it("can compute size of node_test_file", [&]() {
              AssertThat(node_test_file.size(), Is().EqualTo(3u));
              AssertThat(node_test_file.meta_size(), Is().EqualTo(2u));
              AssertThat(node_test_file.file_size(), Is().EqualTo(3u * sizeof(node_t) + 2u * sizeof(level_info_t)));
            });

            it("can compute size of x0", [&]() {
              AssertThat(x0.size(), Is().EqualTo(1u));
              AssertThat(x0.meta_size(), Is().EqualTo(1u));
              AssertThat(x0.file_size(), Is().EqualTo(1u * sizeof(node_t) + 1u * sizeof(level_info_t)));
            });

            it("can compute size of x0 & x1", [&]() {
              AssertThat(x0_and_x1.size(), Is().EqualTo(2u));
              AssertThat(x0_and_x1.meta_size(), Is().EqualTo(2u));
              AssertThat(x0_and_x1.file_size(), Is().EqualTo(2u * sizeof(node_t) + 2u * sizeof(level_info_t)));
            });

            it("can compute size of terminal_T", [&]() {
              AssertThat(terminal_T.size(), Is().EqualTo(1u));
              AssertThat(terminal_T.meta_size(), Is().EqualTo(0u));
              AssertThat(terminal_T.file_size(), Is().EqualTo(1u * sizeof(node_t) + 0u * sizeof(level_info_t)));
            });
          });

          describe("is_terminal predicate", [&]() {
            it("should reject x0 as a terminal file", [&]() {
              AssertThat(is_terminal(x0), Is().False());
            });

            it("should reject x0 & x1 as a terminal file", [&]() {
              AssertThat(is_terminal(x0_and_x1), Is().False());
            });

            it("should recognise a true terminal", [&]() {
              AssertThat(is_terminal(terminal_T), Is().True());
              AssertThat(value_of(terminal_T), Is().True());
            });

            it("should recognise a false terminal", [&]() {
              AssertThat(is_terminal(terminal_F), Is().True());
              AssertThat(value_of(terminal_F), Is().False());
            });
          });

          describe("min_label and max_label", [&]() {
            it("should extract labels from x0", [&]() {
              AssertThat(min_label(x0), Is().EqualTo(0u));
              AssertThat(max_label(x0), Is().EqualTo(0u));
            });

            it("should extract labels from x0_and_x1", [&]() {
              AssertThat(min_label(x0_and_x1), Is().EqualTo(0u));
              AssertThat(max_label(x0_and_x1), Is().EqualTo(1u));
            });

            it("should extract labels from node_test_file", [&]() {
              AssertThat(min_label(node_test_file), Is().EqualTo(0u));
              AssertThat(max_label(node_test_file), Is().EqualTo(1u));
            });
          });
        });
      });
    });
  });
 });
