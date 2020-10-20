#include <coom/apply.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Apply", [&]() {
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==
        //                  START
        tpie::file_stream<node_t> obdd_F_1;
        obdd_F_1.open();

        obdd_F_1.write(create_sink(false));

        tpie::file_stream<node_t> obdd_F_2;
        obdd_F_2.open();

        obdd_F_2.write(create_sink(false));

        tpie::file_stream<node_t> obdd_T_1;
        obdd_T_1.open();

        obdd_T_1.write(create_sink(true));

        tpie::file_stream<node_t> obdd_T_2;
        obdd_T_2.open();

        obdd_T_2.write(create_sink(true));


        tpie::file_stream<meta_t> sink_meta_1;
        sink_meta_1.open();

        tpie::file_stream<meta_t> sink_meta_2;
        sink_meta_2.open();

        //                   END
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==

        it("should XOR F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_T_2, sink_meta_2, xor_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should XOR T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_T_1, sink_meta_1, obdd_T_2, sink_meta_2, xor_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should AND F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_T_2, sink_meta_2, and_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should AND T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_T_1, sink_meta_1, obdd_T_2, sink_meta_2, and_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_T_1, sink_meta_1, obdd_F_2, sink_meta_2, or_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_F_2, sink_meta_2, or_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should IMPLIES F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_T_2, sink_meta_2, implies_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_T_1, sink_meta_1, obdd_F_2, sink_meta_2, implies_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_T_1, sink_meta_1, obdd_T_2, sink_meta_1, implies_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        // == CREATE SINGLE VARIABLE OBDDs FOR UNIT TESTS ==
        //                    START
        tpie::file_stream<node_t> obdd_x0;
        obdd_x0.open();
        obdd_x0.write(create_node(0,MAX_ID, sink_F, sink_T));

        tpie::file_stream<meta_t> meta_x0;
        meta_x0.open();
        meta_x0.write({0});


        tpie::file_stream<node_t> obdd_not_x0;
        obdd_not_x0.open();
        obdd_not_x0.write(create_node(0,MAX_ID, sink_T, sink_F));

        tpie::file_stream<meta_t> meta_not_x0;
        meta_not_x0.open();
        meta_not_x0.write({0});


        tpie::file_stream<node_t> obdd_x1;
        obdd_x1.open();
        obdd_x1.write(create_node(1,MAX_ID, sink_F, sink_T));

        tpie::file_stream<meta_t> meta_x1;
        meta_x1.open();
        meta_x1.write({1});


        tpie::file_stream<node_t> obdd_x2;
        obdd_x2.open();
        obdd_x2.write(create_node(2,MAX_ID, sink_F, sink_T));

        tpie::file_stream<meta_t> meta_x2;
        meta_x2.open();
        meta_x2.write({2});


        //                     END
        // == CREATE SINGLE VARIABLE FOR UNIT TESTS ==

        it("should AND x0 and T", [&]() {
            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_x0, meta_x0, obdd_T_2, sink_meta_2, and_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);
            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should XOR x0 and x1", [&]() {
            /* The order on the leaves are due to the sorting of sink requests
               after evaluating x0

                   1     ---- x0
                  / \
                 2   3   ---- x1
                / \ / \
                F T T F
             */

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_x0, meta_x0, obdd_x1, meta_x1, xor_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().False());
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

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_x0, meta_x0, obdd_x1, meta_x1, implies_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().False());
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

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_x0, meta_x0, obdd_x2, meta_x2, or_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should AND x0 and !x0", [&]() {
            /*
                   1     ---- x0
                  / \
                  F F
             */

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_x0, meta_x0, obdd_not_x0, meta_not_x0, and_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should AND (and shortcut) F and x0", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_x0, meta_x0, and_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
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
        tpie::file_stream<node_t> obdd_1;
        obdd_1.open();

        node_t n1_5 = create_node(3,MAX_ID, sink_F, sink_T);
        obdd_1.write(n1_5);

        node_t n1_4 = create_node(2,MAX_ID, sink_T, n1_5.uid);
        obdd_1.write(n1_4);

        node_t n1_3 = create_node(2,MAX_ID-1, sink_F, sink_T);
        obdd_1.write(n1_3);

        node_t n1_2 = create_node(1,MAX_ID, n1_3.uid, n1_4.uid);
        obdd_1.write(n1_2);

        node_t n1_1 = create_node(0,MAX_ID, n1_3.uid, n1_2.uid);
        obdd_1.write(n1_1);

        tpie::file_stream<meta_t> meta_1;
        meta_1.open();

        meta_1.write({3});
        meta_1.write({2});
        meta_1.write({1});
        meta_1.write({0});

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
        tpie::file_stream<node_t> obdd_2;
        obdd_2.open();

        node_t n2_2 = create_node(3,MAX_ID, sink_T, sink_F);
        obdd_2.write(n2_2);

        node_t n2_1 = create_node(1,MAX_ID, n2_2.uid, sink_T);
        obdd_2.write(n2_1);

        tpie::file_stream<meta_t> meta_2;
        meta_2.open();

        meta_2.write({3});
        meta_2.write({1});


        /*     OBDD 3

                1         ---- x0
               / \
               2 3        ---- x1
             _/ X \_
            | _/ \_ |
             X     X
            / \   / \
           4  5  6  7     ---- x2
          / \/ \/ \/ \
          F T  8  T  F    ---- x3
              / \
              F T

         */
        tpie::file_stream<node_t> obdd_3;
        obdd_3.open();

        node_t n3_8 = create_node(3,MAX_ID, sink_F, sink_T);
        obdd_3.write(n3_8);

        node_t n3_7 = create_node(2,MAX_ID, sink_T, sink_F);
        obdd_3.write(n3_7);

        node_t n3_6 = create_node(2,MAX_ID - 1, n3_8.uid, sink_T);
        obdd_3.write(n3_6);

        node_t n3_5 = create_node(2,MAX_ID - 2, sink_T, n3_8.uid);
        obdd_3.write(n3_5);

        node_t n3_4 = create_node(2,MAX_ID - 3, sink_F, sink_T);
        obdd_3.write(n3_4);

        node_t n3_3 = create_node(1,MAX_ID, n3_4.uid, n3_6.uid);
        obdd_3.write(n3_3);

        node_t n3_2 = create_node(1,MAX_ID - 1, n3_5.uid, n3_7.uid);
        obdd_3.write(n3_2);

        node_t n3_1 = create_node(0,MAX_ID, n3_2.uid, n3_3.uid);
        obdd_3.write(n3_1);

        tpie::file_stream<meta_t> meta_3;
        meta_3.open();

        meta_3.write({3});
        meta_3.write({2});
        meta_3.write({1});
        meta_3.write({0});


        //                 END
        // == CREATE BIG OBDDs FOR UNIT TESTS ==

        it("should IMPLY (and shortcut) F and OBBD1", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_1, meta_1, implies_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 1 and T", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_1, meta_1, obdd_T_2, sink_meta_2, or_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 2 and T", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_2, meta_2, obdd_T_2, sink_meta_2, or_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
          });

        it("should AND (and shortcut) F and OBBD 2", [&]() {
            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<meta_t> out_meta;
            out_meta.open();

            apply(obdd_F_1, sink_meta_1, obdd_2, meta_2, and_op, out_nodes, out_meta);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());

            AssertThat(out_meta.size(), Is().EqualTo(0u));
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
            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_2, meta_2, obdd_x2, meta_x2, xor_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        /* The product construction of obbd_1 and obdd_2 above is as follows in sorted order.

                                            (1,1)                     ---- x0
                                            \_ _/
                                             _X_                      // Match in min, but differ on max
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

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_1, meta_1, obdd_2, meta_2, or_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should AND (and shortcut) OBBD 1 OBBD 2", [&]() {
            /*
                            1                        ---- x0
                            X
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

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_1, meta_1, obdd_2, meta_2, and_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,2), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should XOR OBBD 1 and 2", [&]() {
            /* There is no shortcutting possible on an XOR, so see the product
               construction above. */

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_1, meta_1, obdd_2, meta_2, xor_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,2), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,2), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should XOR OBDD 3 and 1", [&]() {
            /* The queue appD_data is used to forward data across the layer. When
               OBDD 1 and 3 are combined, this is needed

                  The product between the OBDD 3 and OBDD 1 then is

                                   (1,1)                      ---- x0
                           ________/   \_______
                          /                    \
                        (2,3)                  (3,2)          ---- x1
                        /   \_________ ________/   \
                        |             X             \           //      (5,3) (7,3) (4,3) (6,4)
                        \__ _________/ \            |          // min:   0     0     0     1
                        ___X___         \           |          // max:   1     3     0     2
                       /       \         \          |
                      /         \         \         |
                   (4,3)       (5,3)    (7,3)     (6,4)       ---- x2
                    / \         / \      / \       / \
                (F,F) (T,T) (T,F) |  (T,F) (F,T)  /  |
                                  |   ___________/   |
                                  |  /         ______/
                                  \ /         /
                                 (8,T)  --  (T,5)             ---- x3
                                 /   \      /   \
                              (F,T) (T,T) (T,F) (T,T)
            */

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(obdd_3, meta_3, obdd_1, meta_1, xor_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (2,3)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (3,2)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (4,3)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (5,3)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (7,3)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (6,4)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (8,T)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,3), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True()); // (T,5)
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,3)), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,2), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2 }));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3 }));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should AND two 3-Queens placements [1]", [&]() {
            // 3-Queens placement at (0,1)
            tpie::file_stream<node_t> queen_0_1;
            queen_0_1.open();

            node_t q1_7 = create_node(7,0, sink_T, sink_F);
            queen_0_1.write(q1_7);

            node_t q1_5 = create_node(5,0, q1_7.uid, sink_F);
            queen_0_1.write(q1_5);

            node_t q1_4 = create_node(4,0, q1_5.uid, sink_F);
            queen_0_1.write(q1_4);

            node_t q1_3 = create_node(3,0, q1_4.uid, sink_F);
            queen_0_1.write(q1_3);

            node_t q1_2 = create_node(2,0, q1_3.uid, sink_F);
            queen_0_1.write(q1_2);

            node_t q1_1_1 = create_node(1,1, sink_T, sink_F);
            queen_0_1.write(q1_1_1);

            node_t q1_1_0 = create_node(1,0, sink_T, q1_2.uid);
            queen_0_1.write(q1_1_0);

            node_t q1_0 = create_node(0,0, q1_1_0.uid, q1_1_1.uid);
            queen_0_1.write(q1_0);

            tpie::file_stream<meta_t> queen_0_1_meta;
            queen_0_1_meta.open();

            queen_0_1_meta.write({7});
            queen_0_1_meta.write({5});
            queen_0_1_meta.write({4});
            queen_0_1_meta.write({3});
            queen_0_1_meta.write({2});
            queen_0_1_meta.write({1});
            queen_0_1_meta.write({0});


            // 3-Queens placement at (1,1)
            tpie::file_stream<node_t> queen_1_1;
            queen_1_1.open();

            node_t q2_8 = create_node(8,0, sink_T, sink_F);
            queen_1_1.write(q2_8);

            node_t q2_7 = create_node(7,0, q2_8.uid, sink_F);
            queen_1_1.write(q2_7);

            node_t q2_6 = create_node(6,0, q2_7.uid, sink_F);
            queen_1_1.write(q2_6);

            node_t q2_5 = create_node(5,0, q2_6.uid, sink_F);
            queen_1_1.write(q2_5);

            node_t q2_4_1 = create_node(4,1, sink_T, sink_F);
            queen_1_1.write(q2_4_1);

            node_t q2_4_0 = create_node(4,0, sink_T, q2_5.uid);
            queen_1_1.write(q2_4_0);

            node_t q2_3 = create_node(3,0, q2_4_0.uid, q2_4_1.uid);
            queen_1_1.write(q2_3);

            node_t q2_2 = create_node(2,0, q2_3.uid, q2_4_1.uid);
            queen_1_1.write(q2_2);

            node_t q2_1 = create_node(1,0, q2_2.uid, q2_4_1.uid);
            queen_1_1.write(q2_1);

            node_t q2_0 = create_node(0,0, q2_1.uid, q2_4_1.uid);
            queen_1_1.write(q2_0);

            tpie::file_stream<meta_t> queen_1_1_meta;
            queen_1_1_meta.open();

            queen_1_1_meta.write({8});
            queen_1_1_meta.write({7});
            queen_1_1_meta.write({6});
            queen_1_1_meta.write({5});
            queen_1_1_meta.write({4});
            queen_1_1_meta.write({3});
            queen_1_1_meta.write({2});
            queen_1_1_meta.write({1});
            queen_1_1_meta.write({0});


            // Apply it
            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(queen_0_1, queen_0_1_meta, queen_1_1, queen_1_1_meta, and_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), create_node_ptr(3,1) }));

            /* You will notice an interesting crossing of id'd rather than a neat chain in the
             * id 0 and id 1. The ones with id 1 up until this point have all been due to x1
             * being set to false, which immediately gives the true sink, which comes late in
             * the sorting order */

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), create_node_ptr(4,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(4,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(4,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), create_node_ptr(4,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(5,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,1), create_node_ptr(5,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(5,1), create_node_ptr(7,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(6,0), create_node_ptr(7,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(7,1), create_node_ptr(8,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,2), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,2)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), sink_F}));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(5,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(7,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(7,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(7,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(8,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(8,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {0}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {1}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {2}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {3}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {4}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {5}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {6}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {7}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {8}));

            AssertThat(reduce_meta.can_read(), Is().False());
          });

        it("should AND two 3-Queens placements [2]", [&]() {
            // Queen placed at 2,0
            tpie::file_stream<node_t> queen_2_0;
            queen_2_0.open();

            node_t q1_8 = create_node(8,0, sink_T, sink_F);
            queen_2_0.write(q1_8);

            node_t q1_7 = create_node(7,0, q1_8.uid, sink_F);
            queen_2_0.write(q1_7);

            node_t q1_6_1 = create_node(6,1, sink_T, sink_F);
            queen_2_0.write(q1_6_1);

            node_t q1_6_0 = create_node(6,0, sink_T, q1_7.uid);
            queen_2_0.write(q1_6_0);

            node_t q1_4 = create_node(4,0, q1_6_0.uid, q1_6_1.uid);
            queen_2_0.write(q1_4);

            node_t q1_3 = create_node(3,0, q1_4.uid, q1_6_1.uid);
            queen_2_0.write(q1_3);

            node_t q1_2 = create_node(2,0, q1_3.uid, q1_6_1.uid);
            queen_2_0.write(q1_2);

            node_t q1_0 = create_node(0,0, q1_2.uid, q1_6_1.uid);
            queen_2_0.write(q1_0);

            tpie::file_stream<meta_t> queen_2_0_meta;
            queen_2_0_meta.open();

            queen_2_0_meta.write({8});
            queen_2_0_meta.write({7});
            queen_2_0_meta.write({6});
            queen_2_0_meta.write({4});
            queen_2_0_meta.write({3});
            queen_2_0_meta.write({2});
            queen_2_0_meta.write({0});

            // Queen placed at 2,1
            tpie::file_stream<node_t> queen_2_1;
            queen_2_1.open();

            node_t q2_8 = create_node(8,0, sink_T, sink_F);
            queen_2_1.write(q2_8);

            node_t q2_7_1 = create_node(7,1, sink_T, sink_F);
            queen_2_1.write(q2_7_1);

            node_t q2_7_0 = create_node(7,0, sink_T, q2_8.uid);
            queen_2_1.write(q2_7_0);

            node_t q2_6 = create_node(6,0, q2_7_0.uid, q2_7_1.uid);
            queen_2_1.write(q2_6);

            node_t q2_5 = create_node(5,0, q2_6.uid, q2_7_1.uid);
            queen_2_1.write(q2_5);

            node_t q2_4 = create_node(4,0, q2_5.uid, q2_7_1.uid);
            queen_2_1.write(q2_4);

            node_t q2_3 = create_node(3,0, q2_4.uid, q2_7_1.uid);
            queen_2_1.write(q2_3);

            node_t q2_1 = create_node(1,0, q2_3.uid, q2_7_1.uid);
            queen_2_1.write(q2_1);

            tpie::file_stream<meta_t> queen_2_1_meta;
            queen_2_1_meta.open();

            queen_2_1_meta.write({8});
            queen_2_1_meta.write({7});
            queen_2_1_meta.write({6});
            queen_2_1_meta.write({5});
            queen_2_1_meta.write({4});
            queen_2_1_meta.write({3});
            queen_2_1_meta.write({1});


            // Apply it
            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            tpie::file_stream<meta_t> reduce_meta;
            reduce_meta.open();

            apply(queen_2_0, queen_2_0_meta, queen_2_1, queen_2_1_meta, and_op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);


            // Check it
            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(2,1), create_node_ptr(3,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,1), create_node_ptr(4,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(3,2), create_node_ptr(4,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,0), create_node_ptr(5,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,1), create_node_ptr(5,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(4,2), create_node_ptr(6,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), create_node_ptr(6,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(5,1), create_node_ptr(6,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,1)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,2)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(5,1)), create_node_ptr(6,3) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(6,0), create_node_ptr(7,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(6,2), create_node_ptr(7,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), create_node_ptr(7,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(6,1)), create_node_ptr(7,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(6,1), create_node_ptr(7,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(6,3), create_node_ptr(7,2) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(7,0)), create_node_ptr(8,0) }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { create_node_ptr(7,1), create_node_ptr(8,1) }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(6,2)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(6,3)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(7,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(7,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(7,2), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(7,2)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(8,0), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(8,0)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { create_node_ptr(8,1), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(create_node_ptr(8,1)), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());

            reduce_meta.seek(0);

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {0}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {1}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {2}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {3}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {4}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {5}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {6}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {7}));

            AssertThat(reduce_meta.can_read(), Is().True());
            AssertThat(reduce_meta.read(), Is().EqualTo(meta_t {8}));

            AssertThat(reduce_meta.can_read(), Is().False());
          });
      });
  });
