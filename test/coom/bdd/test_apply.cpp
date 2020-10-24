go_bandit([]() {
    describe("COOM: Apply", [&]() {
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==
        //                  START
        node_file obdd_F_1;
        node_writer nw_F_1(obdd_F_1);
        nw_F_1 << create_sink(false);

        node_file obdd_F_2;
        node_writer nw_F_2(obdd_F_2);
        nw_F_2 << create_sink(false);

        node_file obdd_T_1;
        node_writer nw_T_1(obdd_T_1);
        nw_T_1 << create_sink(true);

        node_file obdd_T_2;
        node_writer nw_T_2(obdd_T_2);
        nw_T_2 << create_sink(true);

        //                   END
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==

        it("should XOR F and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_T_2, xor_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should XOR T and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_T_1, obdd_T_2, xor_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND F and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_T_2, and_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND T and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_T_1, obdd_T_2, and_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_T_1, obdd_F_2, or_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_F_2, or_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES F and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_T_2, implies_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and F sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_T_1, obdd_F_2, implies_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and T sink-only OBDDs", [&]() {
            node_or_arc_file out = bdd_apply(obdd_T_1, obdd_T_2, implies_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        // == CREATE SINGLE VARIABLE OBDDs FOR UNIT TESTS ==
        //                    START
        node_file obdd_x0;
        node_writer nw_x0(obdd_x0);
        nw_x0 << create_node(0,MAX_ID, sink_F, sink_T);

        node_file obdd_not_x0;
        node_writer nw_not_x0(obdd_not_x0);
        nw_not_x0 << create_node(0,MAX_ID, sink_T, sink_F);

        node_file obdd_x1;
        node_writer nw_x1(obdd_x1);
        nw_x1 << create_node(1,MAX_ID, sink_F, sink_T);

        node_file obdd_x2;
        node_writer nw_x2(obdd_x2);
        nw_x2 << create_node(2,MAX_ID, sink_F, sink_T);

        //                     END
        // == CREATE SINGLE VARIABLE FOR UNIT TESTS ==

        it("should AND x0 and T", [&]() {
            node_or_arc_file out = bdd_apply(obdd_x0, obdd_T_2, and_op);
            node_arc_test_stream node_arcs(out);
            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_x0, obdd_x1, xor_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_x0, obdd_x1, implies_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_x0, obdd_x2, or_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND x0 and !x0", [&]() {
            /*
                   1     ---- x0
                  / \
                  F F
             */

            node_or_arc_file out = bdd_apply(obdd_x0, obdd_not_x0, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND (and shortcut) F and x0", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_x0, and_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
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
        node_file obdd_1;

        node_t n1_5 = create_node(3,MAX_ID, sink_F, sink_T);
        node_t n1_4 = create_node(2,MAX_ID, sink_T, n1_5.uid);
        node_t n1_3 = create_node(2,MAX_ID-1, sink_F, sink_T);
        node_t n1_2 = create_node(1,MAX_ID, n1_3.uid, n1_4.uid);
        node_t n1_1 = create_node(0,MAX_ID, n1_3.uid, n1_2.uid);

        node_writer nw_1(obdd_1);
        nw_1 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;

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
        node_file obdd_2;

        node_t n2_2 = create_node(3,MAX_ID, sink_T, sink_F);
        node_t n2_1 = create_node(1,MAX_ID, n2_2.uid, sink_T);

        node_writer nw_2(obdd_2);
        nw_2 << n2_2 << n2_1;


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
        node_file obdd_3;

        node_t n3_8 = create_node(3,MAX_ID, sink_F, sink_T);
        node_t n3_7 = create_node(2,MAX_ID, sink_T, sink_F);
        node_t n3_6 = create_node(2,MAX_ID - 1, n3_8.uid, sink_T);
        node_t n3_5 = create_node(2,MAX_ID - 2, sink_T, n3_8.uid);
        node_t n3_4 = create_node(2,MAX_ID - 3, sink_F, sink_T);
        node_t n3_3 = create_node(1,MAX_ID, n3_4.uid, n3_6.uid);
        node_t n3_2 = create_node(1,MAX_ID - 1, n3_5.uid, n3_7.uid);
        node_t n3_1 = create_node(0,MAX_ID, n3_2.uid, n3_3.uid);

        node_writer nw_3(obdd_3);
        nw_3 << n3_8 << n3_7 << n3_6 << n3_5 << n3_4 << n3_3 << n3_2 << n3_1;

        //                 END
        // == CREATE BIG OBDDs FOR UNIT TESTS ==

        it("should IMPLY (and shortcut) F and OBBD1", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_1, implies_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 1 and T", [&]() {
            node_or_arc_file out = bdd_apply(obdd_1, obdd_T_2, or_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 2 and T", [&]() {
            node_or_arc_file out = bdd_apply(obdd_2, obdd_T_2, or_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND (and shortcut) F and OBBD 2", [&]() {
            node_or_arc_file out = bdd_apply(obdd_F_1, obdd_2, and_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
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
            node_or_arc_file out = bdd_apply(obdd_2, obdd_x2, xor_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 3 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_1, obdd_2, or_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 3 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_1, obdd_2, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,2), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 3 }));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should XOR OBBD 1 and 2", [&]() {
            /* There is no shortcutting possible on an XOR, so see the product
               construction above. */

            node_or_arc_file out = bdd_apply(obdd_1, obdd_2, xor_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,2) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,2), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,2), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 3 }));

            AssertThat(meta.can_pull(), Is().False());
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

            node_or_arc_file out = bdd_apply(obdd_3, obdd_1, xor_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True()); // (2,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (3,2)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (4,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (5,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (7,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (6,4)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,3) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (8,T)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,3), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (T,5)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,3)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,2), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 0 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 1 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 2 }));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t { 3 }));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND two 3-Queens placements [1]", [&]() {
            // 3-Queens placement at (0,1)
            node_file queen_0_1;

            node_t q1_7 = create_node(7,0, sink_T, sink_F);
            node_t q1_5 = create_node(5,0, q1_7.uid, sink_F);
            node_t q1_4 = create_node(4,0, q1_5.uid, sink_F);
            node_t q1_3 = create_node(3,0, q1_4.uid, sink_F);
            node_t q1_2 = create_node(2,0, q1_3.uid, sink_F);
            node_t q1_1_1 = create_node(1,1, sink_T, sink_F);
            node_t q1_1_0 = create_node(1,0, sink_T, q1_2.uid);
            node_t q1_0 = create_node(0,0, q1_1_0.uid, q1_1_1.uid);

            node_writer queen_0_1_w(queen_0_1);
            queen_0_1_w << q1_7 << q1_5 << q1_4 << q1_3 << q1_2 << q1_1_1 << q1_1_0 << q1_0;


            // 3-Queens placement at (1,1)
            node_file queen_1_1;

            node_t q2_8 = create_node(8,0, sink_T, sink_F);
            node_t q2_7 = create_node(7,0, q2_8.uid, sink_F);
            node_t q2_6 = create_node(6,0, q2_7.uid, sink_F);
            node_t q2_5 = create_node(5,0, q2_6.uid, sink_F);
            node_t q2_4_1 = create_node(4,1, sink_T, sink_F);
            node_t q2_4_0 = create_node(4,0, sink_T, q2_5.uid);
            node_t q2_3 = create_node(3,0, q2_4_0.uid, q2_4_1.uid);
            node_t q2_2 = create_node(2,0, q2_3.uid, q2_4_1.uid);
            node_t q2_1 = create_node(1,0, q2_2.uid, q2_4_1.uid);
            node_t q2_0 = create_node(0,0, q2_1.uid, q2_4_1.uid);

            node_writer queen_1_1_w(queen_1_1);
            queen_1_1_w << q2_8 << q2_7 << q2_6 << q2_5 << q2_4_1
                        << q2_4_0 << q2_3 << q2_2 << q2_1 << q2_0;


            // Apply it
            node_or_arc_file out = bdd_apply(queen_0_1, queen_1_1, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), create_node_ptr(3,1) }));

            /* You will notice an interesting crossing of id'd rather than a neat chain in the
             * id 0 and id 1. The ones with id 1 up until this point have all been due to x1
             * being set to false, which immediately gives the true sink, which comes late in
             * the sorting order */

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), create_node_ptr(4,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(4,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(4,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), create_node_ptr(4,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(5,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,1), create_node_ptr(5,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,1), create_node_ptr(7,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,0), create_node_ptr(7,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,1), create_node_ptr(8,0) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,2), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,2)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), sink_F}));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(8,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(8,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {0}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {1}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {2}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {3}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {4}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {5}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {6}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {7}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {8}));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND two 3-Queens placements [2]", [&]() {
            // Queen placed at 2,0
            node_file queen_2_0;

            node_t q1_8 = create_node(8,0, sink_T, sink_F);
            node_t q1_7 = create_node(7,0, q1_8.uid, sink_F);
            node_t q1_6_1 = create_node(6,1, sink_T, sink_F);
            node_t q1_6_0 = create_node(6,0, sink_T, q1_7.uid);
            node_t q1_4 = create_node(4,0, q1_6_0.uid, q1_6_1.uid);
            node_t q1_3 = create_node(3,0, q1_4.uid, q1_6_1.uid);
            node_t q1_2 = create_node(2,0, q1_3.uid, q1_6_1.uid);
            node_t q1_0 = create_node(0,0, q1_2.uid, q1_6_1.uid);

            node_writer queen_2_0_w(queen_2_0);
            queen_2_0_w << q1_8 << q1_7 << q1_6_1 << q1_6_0 << q1_4 << q1_3 << q1_2 << q1_0;

            // Queen placed at 2,1
            node_file queen_2_1;

            node_t q2_8 = create_node(8,0, sink_T, sink_F);
            node_t q2_7_1 = create_node(7,1, sink_T, sink_F);
            node_t q2_7_0 = create_node(7,0, sink_T, q2_8.uid);
            node_t q2_6 = create_node(6,0, q2_7_0.uid, q2_7_1.uid);
            node_t q2_5 = create_node(5,0, q2_6.uid, q2_7_1.uid);
            node_t q2_4 = create_node(4,0, q2_5.uid, q2_7_1.uid);
            node_t q2_3 = create_node(3,0, q2_4.uid, q2_7_1.uid);
            node_t q2_1 = create_node(1,0, q2_3.uid, q2_7_1.uid);

            node_writer queen_2_1_w(queen_2_1);
            queen_2_1_w << q2_8 << q2_7_1 << q2_7_0 << q2_6 << q2_5 << q2_4 << q2_3 << q2_1;

            // Apply it
            node_or_arc_file out = bdd_apply(queen_2_0, queen_2_1, and_op);

            // Check it
            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), create_node_ptr(3,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), create_node_ptr(4,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,2), create_node_ptr(4,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), create_node_ptr(5,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,1), create_node_ptr(5,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,2), create_node_ptr(6,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), create_node_ptr(6,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,1), create_node_ptr(6,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,1)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,2)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,1)), create_node_ptr(6,3) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,0), create_node_ptr(7,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,2), create_node_ptr(7,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), create_node_ptr(7,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,1)), create_node_ptr(7,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,1), create_node_ptr(7,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,3), create_node_ptr(7,2) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,0)), create_node_ptr(8,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,1), create_node_ptr(8,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,2)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,3)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,2), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,2)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(8,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(8,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(8,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(8,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, 2> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {0}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {1}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {2}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {3}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {4}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {5}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {6}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {7}));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(meta_t {8}));

            AssertThat(meta.can_pull(), Is().False());
          });
      });
  });
