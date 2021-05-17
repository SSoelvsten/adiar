#include <adiar/isomorphism.h>

go_bandit([]() {
    describe("CORE: Isomorphism", [&]() {
        describe("Trivial cases", [&]() {
            node_file sink_F, x21, x42, x21_and_x22, x21_and_x22_and_x42, x21_and_x42, x21_xor_x42;

            { // Garbage collect writers to free write-lock
              node_writer w_F(sink_F);
              w_F << create_sink(false);

              node_writer w_21(x21);
              w_21 << create_node(21,0, create_sink_ptr(false), create_sink_ptr(true));

              node_writer w_42(x42);
              w_42 << create_node(42,0, create_sink_ptr(false), create_sink_ptr(true));

              node_writer w_21_and_22(x21_and_x22);
              w_21_and_22 << create_node(22,0, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(21,0, create_sink_ptr(false), create_node_ptr(22,0));

              node_writer w_21_and_22_and_42(x21_and_x22_and_x42);
              w_21_and_22_and_42 << create_node(42,0, create_sink_ptr(false), create_sink_ptr(true))
                                 << create_node(22,0, create_sink_ptr(false), create_node_ptr(42,0))
                                 << create_node(21,0, create_sink_ptr(false), create_node_ptr(22,0));

              node_writer w_21_and_42(x21_and_x42);
              w_21_and_42 << create_node(42,0, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(21,0, create_sink_ptr(false), create_node_ptr(42,0));

              node_writer w_21_xor_42(x21_xor_x42);
              w_21_xor_42 << create_node(42,1, create_sink_ptr(true), create_sink_ptr(false))
                          << create_node(42,0, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(21,0, create_node_ptr(42,0), create_node_ptr(42,1));
            }

            it("accepts same file with same negation flag", [&]() {
                AssertThat(is_isomorphic(sink_F, sink_F, false, false), Is().True());
                AssertThat(is_isomorphic(sink_F, sink_F, true, true), Is().True());
              });

            it("rejects same file with differing negation falgs", [&]() {
                AssertThat(is_isomorphic(sink_F, sink_F, false, true), Is().False());
                AssertThat(is_isomorphic(sink_F, sink_F, true, false), Is().False());
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
                AssertThat(is_isomorphic(sink_F, x42, false, false), Is().False());
                AssertThat(is_isomorphic(sink_F, x42, false, true), Is().False());
                AssertThat(is_isomorphic(x42, sink_F, true, false), Is().False());
                AssertThat(is_isomorphic(x42, sink_F, true, true), Is().False());
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
                // Create two BDDs with 6 nodes each on variables x0, x1, x2,
                // and x3 (i.e. the same number of nodes and levels). One with 2
                // nodes for x1 and x2 and one with 3 nodes for x2.

                node_file six_a, six_b;

                { // Garbage collect writers to free write-lock
                  node_writer w_six_a(six_a);
                  w_six_a << create_node(3,0, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(2,1, create_node_ptr(3,0), create_sink_ptr(true))
                          << create_node(2,0, create_node_ptr(3,0), create_sink_ptr(false))
                          << create_node(1,1, create_node_ptr(2,1), create_node_ptr(2,0))
                          << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
                          << create_node(0,0, create_node_ptr(1,0), create_node_ptr(1,0));

                  node_writer w_six_b(six_b);
                  w_six_b << create_node(3,0, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(2,2, create_sink_ptr(false), create_sink_ptr(true))
                          << create_node(2,1, create_node_ptr(3,0), create_sink_ptr(true))
                          << create_node(2,0, create_node_ptr(3,0), create_sink_ptr(false))
                          << create_node(1,0, create_node_ptr(2,1), create_node_ptr(2,2))
                          << create_node(0,0, create_node_ptr(1,0), create_node_ptr(2,0));
                }

                AssertThat(is_isomorphic(six_a, six_b, false, false), Is().False());
                AssertThat(is_isomorphic(six_a, six_b, false, true), Is().False());
                AssertThat(is_isomorphic(six_b, six_a, true, false), Is().False());
                AssertThat(is_isomorphic(six_b, six_a, true, true), Is().False());
              });
          });

        describe("Slow O(sort(N²)) check", [&]() {
            //////////////////////
            // Sink-only cases
            node_file F_a, F_b, T_a;

            { // Garbage collect writers to free write-lock
              node_writer nw_F_a(F_a);
              nw_F_a << create_sink(false);

              node_writer nw_F_b(F_b);
              nw_F_b << create_sink(false);

              node_writer nw_T_a(T_a);
              nw_T_a << create_sink(true);
            }
            it("can compare two F different sinks ", [&]() {
                AssertThat(is_isomorphic(F_a, F_b), Is().True());
              });

            it("can compare the same F sink ", [&]() {
                AssertThat(is_isomorphic(F_a, F_a), Is().True());
              });

            it("can compare an F with a T sink ", [&]() {
                AssertThat(is_isomorphic(T_a, F_a), Is().False());
                AssertThat(is_isomorphic(F_b, T_a), Is().False());
              });

            it("can compare the same F sink, where one is negated", [&]() {
                AssertThat(is_isomorphic(F_a, F_a, true, false), Is().False());
                AssertThat(is_isomorphic(F_a, F_a, false, true), Is().False());
              });

            it("can compare an F with a T sink, where one is negated", [&]() {
                AssertThat(is_isomorphic(T_a, F_a, true, false), Is().True());
                AssertThat(is_isomorphic(T_a, F_a, false, true), Is().True());
                AssertThat(is_isomorphic(F_b, T_a, true, false), Is().True());
                AssertThat(is_isomorphic(F_b, T_a, false, true), Is().True());
              });

            it("can compare an F with a T sink, where both are negated", [&]() {
                AssertThat(is_isomorphic(T_a, F_a, true, true), Is().False());
                AssertThat(is_isomorphic(T_a, F_a, true, true), Is().False());
              });

            //////////////////////
            // One-node cases
            /*
                  1     ---- x42
                 / \
                 F T
            */
            node_file x42_a, x42_b, not_x42;

            { // Garbage collect writers to free write-lock
              node_writer wa(x42_a);
              wa << create_node(42,1, create_sink_ptr(true), create_sink_ptr(false));

              node_writer wb(x42_b);
              wb << create_node(42,0, create_sink_ptr(true), create_sink_ptr(false));

              node_writer wn(not_x42);
              wn << create_node(42,2, create_sink_ptr(false), create_sink_ptr(true));
            }

            it("accepts compare two different x42", [&]() {
                AssertThat(is_isomorphic(x42_a, x42_b, false, false), Is().True());
              });

            it("rejects x42 with ~x42", [&]() {
                AssertThat(is_isomorphic(x42_a, not_x42, false, false), Is().False());
              });

            it("accepts x42 with negated ~x42", [&]() {
                AssertThat(is_isomorphic(x42_a, not_x42, false, true), Is().True());
              });

            //////////////////////
            // Traversal cases
            /*
                     _1_     ---- x0
                    /   \
                    2   3    ---- x1
                   / \ / \
                   T  4  F   ---- x2
                     / \
                     F T

               where we can mirror the 2 and 3 nodes
            */
            node_file bdd_1;
            { // Garbage collect writers to free write-lock
              node_writer w(bdd_1);
              w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                << create_node(1,1, create_node_ptr(2,1), create_sink_ptr(true))
                << create_node(1,0, create_sink_ptr(true), create_node_ptr(2,1))
                << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));
            }

            /*
                 1    ---- x0
                / \
                2 |   ---- x1
               / \/
               3  4   ---- x2
              / \/ \
              T  F T

              where we can mirror the 3 and 4 nodes
            */
            node_file bdd_2;
            { // Garbage collect writers to free write-lock
              node_writer w(bdd_2);
              w << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
                << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
                << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
                << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));
            }

            it("accepts with negation of negated [1]", [&]() {
                // bdd_1 with children negated
                node_file bdd_1n;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_1n);
                  w << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(1,1, create_node_ptr(2,1), create_sink_ptr(false))
                    << create_node(1,0, create_sink_ptr(false), create_node_ptr(2,1))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));
                }

                AssertThat(is_isomorphic(bdd_1, bdd_1n, false, true), Is().True());
                AssertThat(is_isomorphic(bdd_1, bdd_1n, true, false), Is().True());
              });

            it("accepts with negation of negated [2]", [&]() {
                // bdd_1 with children negated
                node_file bdd_2n;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_2n);
                  w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(2,0, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));
                }

                AssertThat(is_isomorphic(bdd_2, bdd_2n, false, true), Is().True());
                AssertThat(is_isomorphic(bdd_2, bdd_2n, true, false), Is().True());
              });


            it("accepts with nodes swapped [1]", [&]() {
                // bdd_1 with nodes (2) and (3) swapped
                node_file bdd_1b;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_1b);
                  w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(1,1, create_sink_ptr(true), create_node_ptr(2,1))
                    << create_node(1,0, create_node_ptr(2,1), create_sink_ptr(true))
                    << create_node(0,1, create_node_ptr(1,1), create_node_ptr(1,0));
                }

                AssertThat(is_isomorphic(bdd_1, bdd_1b, false, false), Is().True());
                AssertThat(is_isomorphic(bdd_1, bdd_1b, true, true), Is().True());
              });

            it("accepts with nodes swapped [2]", [&]() {
                // bdd_2 with nodes (3) and (4) swapped
                node_file bdd_2b;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_2b);
                  w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(2,0, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(1,0, create_node_ptr(2,1), create_node_ptr(2,0))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,0));
                }

                AssertThat(is_isomorphic(bdd_2, bdd_2b, false, false), Is().True());
                AssertThat(is_isomorphic(bdd_2, bdd_2b, true, true), Is().True());
              });

            it("rejects on low child mismatch (leaf value) [1]", [&]() {
                // bdd_1 with low child of 2 flipped in truth value
                node_file bdd_1c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_1c);
                  w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(1,1, create_node_ptr(2,1), create_sink_ptr(true))
                    << create_node(1,0, create_sink_ptr(false), create_node_ptr(2,1))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));
                }

                AssertThat(is_isomorphic(bdd_1, bdd_1c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_1, bdd_1c, true, true), Is().False());
              });

            it("rejects on low child mismatch (internal node vs. leaf) [2]", [&]() {
                /*
                     1      ---- x0
                    / \
                    2  \    ---- x1
                   / \  \
                   T 3  4   ---- x2
                    / \/ \
                    T  F T

                  which in traversal look similar to bdd_2 until (2) on level x1
                */
                node_file bdd_2c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_2c);
                  w << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(1,0, create_sink_ptr(true), create_node_ptr(2,0))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));
                }

                AssertThat(is_isomorphic(bdd_2, bdd_2c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, true, false), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, false, true), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, true, true), Is().False());
              });

            it("rejects on low child mismatch (internal node labels)", [&]() {
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
                node_file bdd_3;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_3);
                  w << create_node(3,0, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(2,0, create_sink_ptr(true), create_node_ptr(3,0))
                    << create_node(1,0, create_node_ptr(2,0), create_sink_ptr(false))
                    << create_node(0,0, create_node_ptr(2,0), create_node_ptr(1,0));
                }

                // Same with (2) directly going to (4) on the low
                node_file bdd_3c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_3c);
                  w << create_node(3,0, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(2,0, create_sink_ptr(true), create_node_ptr(3,0))
                    << create_node(1,0, create_node_ptr(3,0), create_sink_ptr(false))
                    << create_node(0,0, create_node_ptr(2,0), create_node_ptr(1,0));
                }

                AssertThat(is_isomorphic(bdd_3, bdd_3c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, true, false), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, false, true), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, true, true), Is().False());
              });

            it("rejects on high child mismatch (leaf value) [1]", [&]() {
                // bdd_1 with low child of 2 flipped in truth value
                node_file bdd_1c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_1c);
                  w << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(1,1, create_node_ptr(2,1), create_sink_ptr(false))
                    << create_node(1,0, create_sink_ptr(true), create_node_ptr(2,1))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));
                }

                AssertThat(is_isomorphic(bdd_1, bdd_1c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_1, bdd_1c, true, true), Is().False());
              });

            it("rejects on high child mismatch (internal node vs. leaf) [2]", [&]() {
                /*
                     1      ---- x0
                    / \
                    2  \    ---- x1
                   / \  \
                   3 F  4   ---- x2
                  / \  / \
                  T F  T T

                  which in traversal look similar to bdd_2 until (2) on level x1
                */
                node_file bdd_2c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_2c);
                  w << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
                    << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(1,0, create_node_ptr(2,0), create_sink_ptr(false))
                    << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));
                }

                AssertThat(is_isomorphic(bdd_2, bdd_2c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, true, false), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, false, true), Is().False());
                AssertThat(is_isomorphic(bdd_2, bdd_2c, true, true), Is().False());
              });

            it("rejects on high child mismatch (internal node labels)", [&]() {
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

                   The same as the other internal node label child mismatch
                   (bdd_3), but mirrored horisontally
                */
                node_file bdd_3;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_3);
                  w << create_node(3,0, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(2,0, create_node_ptr(3,0), create_sink_ptr(true))
                    << create_node(1,0, create_sink_ptr(false), create_node_ptr(2,0))
                    << create_node(0,0, create_node_ptr(1,0), create_node_ptr(2,0));
                }

                // Same with (2) directly going to (4) on the high
                node_file bdd_3c;
                { // Garbage collect writers to free write-lock
                  node_writer w(bdd_3c);
                  w << create_node(3,0, create_sink_ptr(true), create_sink_ptr(false))
                    << create_node(2,0, create_node_ptr(3,0), create_sink_ptr(true))
                    << create_node(1,0, create_sink_ptr(false), create_node_ptr(3,0))
                    << create_node(0,0, create_node_ptr(1,0), create_node_ptr(2,0));
                }

                AssertThat(is_isomorphic(bdd_3, bdd_3c, false, false), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, true, false), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, false, true), Is().False());
                AssertThat(is_isomorphic(bdd_3, bdd_3c, true, true), Is().False());
              });

            // TODO: Fail on too many requests
          });
      });
  });
