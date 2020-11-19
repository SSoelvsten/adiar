#include <coom/homomorphism.h>

go_bandit([]() {
    describe("CORE: Homomorphism", [&]() {
        /*
                      Sinks
         */
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
          AssertThat(is_homomorphic(F_a, F_b), Is().True());
        });

        it("can compare the same F sink ", [&]() {
          AssertThat(is_homomorphic(F_a, F_a), Is().True());
        });

        it("can compare an F with a T sink ", [&]() {
          AssertThat(is_homomorphic(T_a, F_a), Is().False());
          AssertThat(is_homomorphic(F_b, T_a), Is().False());
        });

        it("can compare the same F sink, where one is negated", [&]() {
          AssertThat(is_homomorphic(F_a, F_a, true, false), Is().False());
          AssertThat(is_homomorphic(F_a, F_a, false, true), Is().False());
        });

        it("can compare an F with a T sink, where one is negated", [&]() {
          AssertThat(is_homomorphic(T_a, F_a, true, false), Is().True());
          AssertThat(is_homomorphic(T_a, F_a, false, true), Is().True());
          AssertThat(is_homomorphic(F_b, T_a, true, false), Is().True());
          AssertThat(is_homomorphic(F_b, T_a, false, true), Is().True());
        });

        it("can compare an F with a T sink, where both are negated", [&]() {
          AssertThat(is_homomorphic(T_a, F_a, true, true), Is().False());
          AssertThat(is_homomorphic(T_a, F_a, true, true), Is().False());
        });

        /*
               1     ---- x42
              / \
              F T
         */
        node_file x42_a, x42_b, notx42_a, x22_a;

        { // Garbage collect writers to free write-lock
          node_writer w42a(x42_a);
          w42a << create_node(42,1, create_sink_ptr(true), create_sink_ptr(false));

          node_writer w42b(x42_b);
          w42b << create_node(42,0, create_sink_ptr(true), create_sink_ptr(false));

          node_writer w42n(notx42_a);
          w42n << create_node(42,2, create_sink_ptr(false), create_sink_ptr(true));

          node_writer w22(x22_a);
          w22 << create_node(22,3, create_sink_ptr(true), create_sink_ptr(false));
        }

        it("can compare x42 with a sink", [&]() {
          AssertThat(is_homomorphic(T_a, x42_a), Is().False());
          AssertThat(is_homomorphic(x42_b, F_a), Is().False());
        });

        it("can compare the same file instance of x42", [&]() {
          AssertThat(is_homomorphic(x42_a, x42_a), Is().True());
          AssertThat(is_homomorphic(x42_a, x42_a, true, false), Is().False());
          AssertThat(is_homomorphic(x42_a, x42_a, false, true), Is().False());
          AssertThat(is_homomorphic(x42_a, x42_a, true, true), Is().True());
        });

        it("can compare the two different files of x42", [&]() {
          AssertThat(is_homomorphic(x42_a, x42_b), Is().True());
          AssertThat(is_homomorphic(x42_a, x42_b, true, false), Is().False());
          AssertThat(is_homomorphic(x42_a, x42_b, false, true), Is().False());
          AssertThat(is_homomorphic(x42_a, x42_b, true, true), Is().True());
        });

        it("can compare x42 and ~x42", [&]() {
          AssertThat(is_homomorphic(x42_a, notx42_a), Is().False());
          AssertThat(is_homomorphic(x42_a, notx42_a, false, true), Is().True());
          AssertThat(is_homomorphic(x42_a, notx42_a, true, false), Is().True());
          AssertThat(is_homomorphic(x42_a, notx42_a, true, true), Is().False());
        });

        it("can compare the two different files of x42", [&]() {
          AssertThat(is_homomorphic(x42_a, x22_a), Is().False());
          AssertThat(is_homomorphic(x22_a, notx42_a), Is().False());
          AssertThat(is_homomorphic(x22_a, x42_b, true, false), Is().False());
          AssertThat(is_homomorphic(x22_a, x42_a, false, true), Is().False());
          AssertThat(is_homomorphic(x22_a, x42_a, true, true), Is().False());
        });

        /*
                 _1_     ---- x0
                /   \
                2   3    ---- x1
               / \ / \
               T  4  T   ---- x2
                 / \
                 F T

           where we can mirror the 2 and 3 nodes
         */
        node_file bdd_1_a, bdd_1_b, bdd_1_a_shifted;
        { // Garbage collect writers to free write-lock
          node_writer aw(bdd_1_a);
          aw << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,1, create_node_ptr(2,1), create_sink_ptr(true))
             << create_node(1,0, create_sink_ptr(true), create_node_ptr(2,1))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));

          node_writer bw(bdd_1_b); // x1 nodes swapped
          bw << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,1, create_sink_ptr(true), create_node_ptr(2,0))
             << create_node(1,0, create_node_ptr(2,0), create_sink_ptr(true))
             << create_node(0,1, create_node_ptr(1,1), create_node_ptr(1,0));

          node_writer cw(bdd_1_a_shifted);
          cw << create_node(12,1, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(11,1, create_node_ptr(12,1), create_sink_ptr(true))
             << create_node(11,0, create_sink_ptr(true), create_node_ptr(12,1))
             << create_node(10,1, create_node_ptr(11,0), create_node_ptr(11,1));
        }

        it("can compare bdd 1 with x42 and x22", [&]() {
          AssertThat(is_homomorphic(x42_a, bdd_1_a), Is().False());
          AssertThat(is_homomorphic(x42_a, bdd_1_a, true, false), Is().False());
          AssertThat(is_homomorphic(x42_a, bdd_1_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_b, x22_a), Is().False());
        });

        it("can compare bdd 1 with the same instance", [&]() {
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a), Is().True());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a, true, true), Is().True());
        });

        it("can compare both instances of bdd 1", [&]() {
           AssertThat(is_homomorphic(bdd_1_a, bdd_1_b), Is().True());
           AssertThat(is_homomorphic(bdd_1_a, bdd_1_b, true, false), Is().False());
           AssertThat(is_homomorphic(bdd_1_a, bdd_1_b, false, true), Is().False());
           AssertThat(is_homomorphic(bdd_1_a, bdd_1_b, true, true), Is().True());
        });

        it("can reject bdd 1 shifted by 10 labels", [&]() {
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a_shifted), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a_shifted, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a_shifted, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_a_shifted, true, true), Is().False());
        });

        it("can compare both instances of bdd 1, where one is negated", [&]() {
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_b, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_1_b, true, false), Is().False());
        });

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
        node_file bdd_2_a, bdd_2_b, bdd_2_c, bdd_2_a_shifted;
        { // Garbage collect writers to free write-lock
          node_writer aw(bdd_2_a); // As depicted
          aw << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));

          node_writer bw(bdd_2_b); // x2 nodes swapped
          bw << create_node(2,3, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(1,1, create_node_ptr(2,3), create_node_ptr(2,1))
             << create_node(0,0, create_node_ptr(1,1), create_node_ptr(2,1));

          node_writer cw(bdd_2_c); // Same as bdd_2_a, but with different ids
          cw << create_node(2,3, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,3))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,3));

          node_writer dw(bdd_2_a_shifted); // As depicted, but with x2 shifted
          dw << create_node(3,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(3,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,0, create_node_ptr(3,0), create_node_ptr(3,1))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));

        }

        it("can compare the same instance of bdd 2", [&]() {
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a), Is().True());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a, true, true), Is().True());
        });

        it("can compare two instances of bdd 2 with only different ids", [&]() {
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_c), Is().True());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_c, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_c, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_c, true, true), Is().True());
        });

        it("can compare both instances of bdd 2 that have shuffled nodes", [&]() {
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_b), Is().True());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_b, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_b, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_b, true, true), Is().True());
        });

        it("can compare bdd 1 with bdd 2", [&]() {
          AssertThat(is_homomorphic(bdd_1_a, bdd_2_a), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_2_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_2_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_2_a, true, true), Is().False());
        });

        it("can reject bdd 2 with labels of x2 nodes shifted", [&]() {
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a_shifted), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a_shifted, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a_shifted, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_2_a_shifted, true, true), Is().False());
        });

        /*
                 _1_     ---- x0
                /   \
               2     3   ---- x1
              / \   / \
             3   4  F T  ---- x2
            / \ / \
            T F F T
         */
        node_file bdd_3_a, bdd_3_b, bdd_3_c;
        { // Garbage collect writers to free write-lock
          node_writer aw(bdd_3_a); // As depicted
          aw << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));

          node_writer bw(bdd_3_b); // x1 nodes swapped
          bw << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(1,1, create_node_ptr(2,0), create_node_ptr(2,1))
             << create_node(1,0, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(0,1, create_node_ptr(1,1), create_node_ptr(1,0));

          node_writer cw(bdd_3_c); // x2 nodes swapped
          cw << create_node(2,1, create_sink_ptr(true), create_sink_ptr(false))
             << create_node(2,0, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(1,1, create_sink_ptr(false), create_sink_ptr(true))
             << create_node(1,0, create_node_ptr(2,1), create_node_ptr(2,0))
             << create_node(0,1, create_node_ptr(1,0), create_node_ptr(1,1));
        }

        it("can compare bdd 3 with the same instance", [&]() {
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_a), Is().True());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_a, true, true), Is().True());
        });

        it("can compare all instances of bdd 3", [&]() {
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_b), Is().True());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_b, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_b, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_b, true, true), Is().True());

          AssertThat(is_homomorphic(bdd_3_a, bdd_3_c), Is().True());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_c, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_c, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_3_a, bdd_3_c, true, true), Is().True());

          AssertThat(is_homomorphic(bdd_3_b, bdd_3_c), Is().True());
          AssertThat(is_homomorphic(bdd_3_b, bdd_3_c, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_3_b, bdd_3_c, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_3_b, bdd_3_c, true, true), Is().True());
        });

        it("can compare bdd 1 with bdd 3", [&]() {
          AssertThat(is_homomorphic(bdd_1_a, bdd_3_a), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_3_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_3_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_1_a, bdd_3_a, true, true), Is().False());
        });

        it("can compare bdd 2 with bdd 3", [&]() {
          AssertThat(is_homomorphic(bdd_2_a, bdd_3_a), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_3_a, true, false), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_3_a, false, true), Is().False());
          AssertThat(is_homomorphic(bdd_2_a, bdd_3_a, true, true), Is().False());
        });
    });
});
