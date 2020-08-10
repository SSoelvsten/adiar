#include <tpie/tpie.h>
#include <coom/apply.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Apply", [&]() {
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==
        //                  START
        tpie::file_stream<node> obdd_F_1;
        obdd_F_1.open();

        obdd_F_1.write(create_sink_node(false));

        tpie::file_stream<node> obdd_F_2;
        obdd_F_2.open();

        obdd_F_2.write(create_sink_node(false));

        tpie::file_stream<node> obdd_T_1;
        obdd_T_1.open();

        obdd_T_1.write(create_sink_node(true));

        tpie::file_stream<node> obdd_T_2;
        obdd_T_2.open();

        obdd_T_2.write(create_sink_node(true));

        //                   END
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==

        it("should XOR F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, xor_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should XOR T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, xor_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should AND F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should AND T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_F_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_F_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_F_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        auto sink_T = create_sink(true);
        auto sink_F = create_sink(false);

        // == CREATE SINGLE VARIABLE OBDDs FOR UNIT TESTS ==
        //                    START
        tpie::file_stream<node> obdd_x0;
        obdd_x0.open();
        obdd_x0.write(create_node(0,MAX_ID, sink_F, sink_T));

        tpie::file_stream<node> obdd_not_x0;
        obdd_not_x0.open();
        obdd_not_x0.write(create_node(0,MAX_ID, sink_T, sink_F));

        tpie::file_stream<node> obdd_x1;
        obdd_x1.open();
        obdd_x1.write(create_node(1,MAX_ID, sink_F, sink_T));

        tpie::file_stream<node> obdd_x2;
        obdd_x2.open();
        obdd_x2.write(create_node(2,MAX_ID, sink_F, sink_T));

        //                     END
        // == CREATE SINGLE VARIABLE FOR UNIT TESTS ==

        it("should XOR x0 and x1", [&]() {
            /* The order on the leaves are due to the sorting of sink requests
               after evaluating x0

                   1     ---- x0
                  / \
                 2   3   ---- x1
                / \ / \
                F T T F
             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_x0, obdd_x1, xor_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_node_ptr(1,0))));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_node_ptr(1,1))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should IMPLY (and shortcut) on x0 and x1", [&]() {
            /* The order on the leaves are due to the sorting of sink requests
               after evaluating x0

                   1     ---- x0
                  / \
                  T 2    ---- x1
                   / \
                   F T
             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_x0, obdd_x1, implies_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_node_ptr(1,0))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should OR (and shortcut) on x0 and x2", [&]() {
            /*
                   1     ---- x0
                  / \
                  | T
                  |
                  2      ---- x2
                 / \
                 F T
             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_x0, obdd_x2, or_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);


            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should AND x0 and !x0", [&]() {
            /*
                   1     ---- x0
                  / \
                  F F
             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_x0, obdd_not_x0, and_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should AND (and shortcut) F and x0", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_x0, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        // == CREATE BIG OBDDs FOR UNIT TESTS ==
        //                START

        /*      OBBD 1

            1        ---- x0
           / \
           | 2       ---- x1
           |/ \
           3   4     ---- x2
          / \ / \
          F T T 5    ---- x3
               / \
               F T
         */
        tpie::file_stream<node> obdd_1;
        obdd_1.open();

        auto n1_5 = create_node(3,MAX_ID, sink_F, sink_T);
        obdd_1.write(n1_5);

        auto n1_4 = create_node(2,MAX_ID, sink_T, n1_5.node_ptr);
        obdd_1.write(n1_4);

        auto n1_3 = create_node(2,MAX_ID-1, sink_F, sink_T);
        obdd_1.write(n1_3);

        auto n1_2 = create_node(1,MAX_ID, n1_3.node_ptr, n1_4.node_ptr);
        obdd_1.write(n1_2);

        auto n1_1 = create_node(0,MAX_ID, n1_3.node_ptr, n1_2.node_ptr);
        obdd_1.write(n1_1);

        /*      OBBD 2

                   ---- x0

             1     ---- x1
            / \
            | T    ---- x2
            |
            2      ---- x3
           / \
           T F
         */
        tpie::file_stream<node> obdd_2;
        obdd_2.open();

        auto n2_2 = create_node(3,MAX_ID, sink_T, sink_F);
        obdd_2.write(n2_2);

        auto n2_1 = create_node(1,MAX_ID, n2_2.node_ptr, sink_T);
        obdd_2.write(n2_1);

        //                 END
        // == CREATE BIG OBDDs FOR UNIT TESTS ==

        it("should IMPLY (and shortcut) F and OBBD1", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_1, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR (and shortcut) OBBD 1 and T", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_1, obdd_T_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR (and shortcut) OBBD 2 and T", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_2, obdd_T_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should AND (and shortcut) F and OBBD 2", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_2, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should XOR OBBD 2 and x2", [&]() {
            /*
                                         ---- x0

                         (1,1)           ---- x1
                         /   \
                     (2,1)   (T,1)       ---- x2
                     /   \   /   \
                    /     \  T   F
                    |     |
                (2,F)     (2,T)          ---- x3
                /   \     /   \
                T   F     F   T
             */
            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_2, obdd_x2, xor_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_node_ptr(2,1))));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        false,
                                                                        create_node_ptr(3,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        true,
                                                                        create_node_ptr(3,1))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        /* The product construction of obbd_1 and obdd_2 above is as follows in sorted order.

                                            (1,1)                     ---- x0
                                            /   \
                                        (3,1)   (2,1)                 ---- x1
                                       /    \_ _/    \
                                      /       X       \
                                     /_______/ \       \
                                     |          \       \
                                 (3,2)          (3,T)   (4,T)         ---- x2
                                 /   \          /   \   /   \
                                 |   |      (F,T)   (T,T)   |
                              (F,2) (T,2)                 (5,T)       ---- x3
                              /  \    /  \                /   \
                          (F,T)(F,F)(T,T)(T,F)         (F,T) (T,T)
         */

        it("should OR (and shortcut) OBBD 1 and OBBD 2", [&]() {
            /*
                           1       ---- x0
                          / \
                         2   3     ---- x1
                        / \ / \
                        | T | T
                        \_ _/
                          4        ---- x2
                         / \
                         5  T      ---- x3
                        / \
                        T F
             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_1, obdd_2, or_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_node_ptr(1,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_node_ptr(1,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        false,
                                                                        create_node_ptr(3,0))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should AND (and shortcut) OBBD 1 OBBD 2", [&]() {
            /*
                            1                        ---- x0
                           / \
                          2   3                      ---- x1
                         / \ / \
                        /   X   \
                       /___/ \   \
                      /      |    \
                     4       5     6                 ---- x2
                    / \     / \_ _/ \
                    F 7     F   T   8                ---- x3
                     / \           / \
                     T F           F T

             */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_1, obdd_2, and_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_node_ptr(1,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_node_ptr(1,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_node_ptr(2,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        true,
                                                                        create_node_ptr(2,2))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        true,
                                                                        create_node_ptr(3,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,2),
                                                                        true,
                                                                        create_node_ptr(3,1))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,2),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });

        it("should XOR OBBD 1 and 2", [&]() {
            /* There is no shortcutting possible on an XOR, so see the product
               construction above. */

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            apply(obdd_1, obdd_2, xor_op, reduce_node_arcs, reduce_sink_arcs);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        false,
                                                                        create_node_ptr(1,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(0,0),
                                                                        true,
                                                                        create_node_ptr(1,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        false,
                                                                        create_node_ptr(2,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,0),
                                                                        true,
                                                                        create_node_ptr(2,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(1,1),
                                                                        true,
                                                                        create_node_ptr(2,2))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        false,
                                                                        create_node_ptr(3,0))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,0),
                                                                        true,
                                                                        create_node_ptr(3,1))));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,2),
                                                                        true,
                                                                        create_node_ptr(3,2))));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,1),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(2,2),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,0),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        false,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,1),
                                                                        true,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,2),
                                                                        false,
                                                                        create_sink(true))));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(create_node_ptr(3,2),
                                                                        true,
                                                                        create_sink(false))));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });
      });
  });
