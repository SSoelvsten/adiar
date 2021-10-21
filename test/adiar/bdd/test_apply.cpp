go_bandit([]() {
    describe("BDD: Apply", [&]() {
        // == CREATE SINK-ONLY BDD FOR UNIT TESTS ==
        //                  START
        node_file bdd_F;
        node_file bdd_T;

        { // Garbage collect writers to free write-lock
          node_writer nw_F(bdd_F);
          nw_F << create_sink(false);

          node_writer nw_T(bdd_T);
          nw_T << create_sink(true);
        }

        //                   END
        // == CREATE SINK-ONLY BDD FOR UNIT TESTS ==

        it("should XOR F and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_T, xor_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should XOR T and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_T, xor_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()) ;
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND F and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_T, and_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND T and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_T, and_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_F, or_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR T and F sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_F, or_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES F and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_T, imp_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and F sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_F, imp_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should IMPLIES T and T sink-only BDDs", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_T, imp_op);
            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        // == CREATE SINGLE VARIABLE BDDs FOR UNIT TESTS ==
        //                    START
        node_file bdd_x0;
        node_file bdd_not_x0;
        node_file bdd_x1;
        node_file bdd_x2;

        { // Garbage collect writers early
          node_writer nw_x0(bdd_x0);
          nw_x0 << create_node(0,MAX_ID, sink_F, sink_T);

          node_writer nw_not_x0(bdd_not_x0);
          nw_not_x0 << create_node(0,MAX_ID, sink_T, sink_F);

          node_writer nw_x1(bdd_x1);
          nw_x1 << create_node(1,MAX_ID, sink_F, sink_T);

          node_writer nw_x2(bdd_x2);
          nw_x2 << create_node(2,MAX_ID, sink_F, sink_T);
        }

        //                     END
        // == CREATE SINGLE VARIABLE FOR UNIT TESTS ==

        it("should AND (and shortcut on irrelevance) x0 and T", [&]() {
            __bdd out = bdd_apply(bdd_x0, bdd_T, and_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().False());
          });

        it("should OR (and shortcut on irrelevance) x0 and F", [&]() {
            __bdd out = bdd_apply(bdd_x0, bdd_F, or_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().False());
          });

        it("should IMPLIES (and shortcut on irrelevance) T and x0", [&]() {
            __bdd out = bdd_apply(bdd_T, bdd_x0, imp_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().False());
          });

        it("should OR (and shortcut on irrelevance) F and x0", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_x0, or_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().False());
          });

        it("should XOR (and shortcut on negating) x0 and T", [&]() {
            __bdd out = bdd_apply(bdd_x0, bdd_T, xor_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().True());
          });

        it("should XOR (and shortcut on negating) T and x0", [&]() {
           __bdd out = bdd_apply(bdd_x0, bdd_T, xor_op);

           AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
           AssertThat(out.negate, Is().True());
         });

        it("should NAND (and shortcut on negating) T and x0", [&]() {
            __bdd out = bdd_apply(bdd_x0, bdd_T, nand_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
            AssertThat(out.negate, Is().True());
          });

        it("should NAND (and shortcut on negating) T and x0", [&]() {
           __bdd out = bdd_apply(bdd_x0, bdd_T, nand_op);

           AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_x0._file_ptr));
           AssertThat(out.negate, Is().True());
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

            __bdd out = bdd_apply(bdd_x0, bdd_x1, xor_op);

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

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,2u)));

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

            __bdd out = bdd_apply(bdd_x0, bdd_x1, imp_op);

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

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,1u)));

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

            __bdd out = bdd_apply(bdd_x0, bdd_x2, or_op);

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

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,1u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND x0 and !x0", [&]() {
            /*
                   1     ---- x0
                  / \
                  F F
             */

            __bdd out = bdd_apply(bdd_x0, bdd_not_x0, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND (and shortcut) F and x0", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_x0, and_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        // == CREATE BIG BDDs FOR UNIT TESTS ==
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
        node_file bdd_1;

        node_t n1_5 = create_node(3,MAX_ID, sink_F, sink_T);
        node_t n1_4 = create_node(2,MAX_ID, sink_T, n1_5.uid);
        node_t n1_3 = create_node(2,MAX_ID-1, sink_F, sink_T);
        node_t n1_2 = create_node(1,MAX_ID, n1_3.uid, n1_4.uid);
        node_t n1_1 = create_node(0,MAX_ID, n1_3.uid, n1_2.uid);

        { // Garbage collect early and free write-lock
          node_writer nw_1(bdd_1);
          nw_1 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
        }

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
        node_file bdd_2;

        node_t n2_2 = create_node(3,MAX_ID, sink_T, sink_F);
        node_t n2_1 = create_node(1,MAX_ID, n2_2.uid, sink_T);

        { // Garbage collect early and free write-lock
          node_writer nw_2(bdd_2);
          nw_2 << n2_2 << n2_1;
        }


        /*     BDD 3

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
        node_file bdd_3;

        node_t n3_8 = create_node(3,MAX_ID, sink_F, sink_T);
        node_t n3_7 = create_node(2,MAX_ID, sink_T, sink_F);
        node_t n3_6 = create_node(2,MAX_ID - 1, n3_8.uid, sink_T);
        node_t n3_5 = create_node(2,MAX_ID - 2, sink_T, n3_8.uid);
        node_t n3_4 = create_node(2,MAX_ID - 3, sink_F, sink_T);
        node_t n3_3 = create_node(1,MAX_ID, n3_4.uid, n3_6.uid);
        node_t n3_2 = create_node(1,MAX_ID - 1, n3_5.uid, n3_7.uid);
        node_t n3_1 = create_node(0,MAX_ID, n3_2.uid, n3_3.uid);

        { // Garbage collect early and free write-lock
          node_writer nw_3(bdd_3);
          nw_3 << n3_8 << n3_7 << n3_6 << n3_5 << n3_4 << n3_3 << n3_2 << n3_1;
        }

        //                 END
        // == CREATE BIG BDDs FOR UNIT TESTS ==

        it("should IMPLY (and shortcut) F and OBBD1", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_1, imp_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 1 and T", [&]() {
            __bdd out = bdd_apply(bdd_1, bdd_T, or_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should OR (and shortcut) OBBD 2 and T", [&]() {
            __bdd out = bdd_apply(bdd_2, bdd_T, or_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should AND (and shortcut) F and OBBD 2", [&]() {
            __bdd out = bdd_apply(bdd_F, bdd_2, and_op);

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
            __bdd out = bdd_apply(bdd_2, bdd_x2, xor_op);

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

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,2u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        /* The product construction of bbd_1 and bdd_2 above is as follows in sorted order.

                                            (1,1)                       ---- x0
                                            \_ _/
                                             _X_                        // Match in fst, but not coordinatewise
                                            /   \
                                        (3,1)   (2,1)                   ---- x1
                                       /    \_ _/    \
                                      /       X       \
                                     /_______/ \       \
                                     |          \       \
                                 (3,2)          (3,T)   (4,T)           ---- x2
                                  \ \           /   \   /   \
                                   \ \      (F,T)   (T,T)   /
                                    \ \________ ___________/
                                     \________ X________
                                              X_________\ _______
                                             /           \       \
                                            /             \       \
                                         (5,T)         (F,2)     (T,2)   ---- x3
                                         /   \         /   \     /   \
                                      (F,T) (T,T)   (F,T)(F,F)  (T,T)(T,F)
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

            __bdd out = bdd_apply(bdd_1, bdd_2, or_op);

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

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,1u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should AND (and shortcut) BBD 1 and 2", [&]() {
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

            __bdd out = bdd_apply(bdd_1, bdd_2, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,1) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,2) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

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
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,3u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,2u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should XOR BBD 1 and 2", [&]() {
            /* There is no shortcutting possible on an XOR, so see the product
               construction above. */

            __bdd out = bdd_apply(bdd_1, bdd_2, xor_op);

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
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,2) }));

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
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,2), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,3u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,3u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should XOR BDD 3 and 1", [&]() {
            /* The queue appD_data is used to forward data across the level. When
               BDD 1 and 3 are combined, this is needed

                  The product between the BDD 3 and BDD 1 then is

                                   (1,1)                      ---- x0
                           ________/   \_______
                          /                    \
                        (2,3)                  (3,2)          ---- x1
                        /   \_________ ________/   \
                        |             X            |          //      (5,3) (7,3) (4,3) (6,4)
                        \__ _________/ \__________ /          // min:   0     0     0     1
                        ___X___                   X           // max:   1     3     0     2
                       /       \            _____/ \          // coord: 2     3     1     4
                      /         \          /        \
                   (4,3)       (5,3)    (6,4)     (7,3)       ---- x2
                   /   \       /   \    /   \     /   \
                (F,F) (T,T) (T,F)  |   /     \  (T,F) (F,T)
                                   |  /       \
                                   | /        |
                                   |/         |
                                 (8,T)      (T,5)             ---- x3
                                 /   \      /   \
                              (F,T) (T,T) (T,F) (T,T)
            */

            __bdd out = bdd_apply(bdd_3, bdd_1, xor_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True()); // (2,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (3,2)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (4,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (5,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (6,4)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,2) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (7,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,3) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (8,T)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,2), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (T,5)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,2)), create_node_ptr(3,1) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,3), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,3)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,4u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,2u)));

            AssertThat(meta.can_pull(), Is().False());
          });

        it("should group all recursion requests together", [&]() {
            // This is a counter-example to the prior "break ties on fst() with
            // snd()" approach. Here we will have three requests to the level of
            // x2, but in the following order:
            //
            // [((2,0),(2,1)), ((2,1),(2,0)), ((2,0),(2,1))]
            //
            // which all are tied, and hence the prior version would create
            // three nodes on this level rather than just two.

            /*
                 1    ---- x0
                / \
                2 |   ---- x1
               / \|
               3  4   ---- x2
              / \/ \
              T  F T
            */

            // The second version is the same but has the nodes 3 and 4 mirrored
            // and the T sinks are replaced with an arc to a node for x3.

            node_file bdd_group_1, bdd_group_2;
            { // Garbage collect writers to free write-lock
              node_writer w1(bdd_group_1);
              w1 << create_node(2,1, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(2,0, create_sink_ptr(true), create_sink_ptr(false))
                 << create_node(1,0, create_node_ptr(2,0), create_node_ptr(2,1))
                 << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,1));

              node_writer w2(bdd_group_2);
              w2 << create_node(3,0, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(2,1, create_node_ptr(3,0), create_sink_ptr(false))
                 << create_node(2,0, create_sink_ptr(false), create_node_ptr(3,0))
                 << create_node(1,0, create_node_ptr(2,1), create_node_ptr(2,0))
                 << create_node(0,1, create_node_ptr(1,0), create_node_ptr(2,0));
            }

            __bdd out = bdd_apply(bdd_group_1, bdd_group_2, and_op);

            node_arc_test_stream node_arcs(out);

            AssertThat(node_arcs.can_pull(), Is().True()); // (2,2)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (3,4)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (4,3)
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));
            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

            AssertThat(node_arcs.can_pull(), Is().True()); // (T,5) i.e. the added node
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));
            AssertThat(node_arcs.can_pull(), Is().True());
            AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(3,0) }));

            AssertThat(node_arcs.can_pull(), Is().False());

            sink_arc_test_stream sink_arcs(out);

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

            AssertThat(sink_arcs.can_pull(), Is().True());
            AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

            AssertThat(sink_arcs.can_pull(), Is().False());

            meta_test_stream<arc_t, ARC_FILE_COUNT> meta(out);

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(0,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(1,1u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(2,2u)));

            AssertThat(meta.can_pull(), Is().True());
            AssertThat(meta.pull(), Is().EqualTo(create_meta(3,1u)));

            AssertThat(meta.can_pull(), Is().False());
          });


        it("should collapse an XOR on the same BDD twice", [&]() {
            __bdd out = bdd_apply(bdd_1, bdd_1, xor_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should collapse a NAND on the same BDD twice, where one is negated [1]", [&]() {
            __bdd out = bdd_apply(bdd_2, bdd_not(bdd_2), nand_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should collapse a NOR on the same BDD twice to a sink, where one is negated [2]", [&]() {
            __bdd out = bdd_apply(bdd_not(bdd_3), bdd_3, nor_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should collapse an XOR on the same BDD twice to a sink, when both are negated", [&]() {
            __bdd out = bdd_apply(bdd_not(bdd_1), bdd_not(bdd_1), xor_op);

            node_test_stream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_pull(), Is().False());

            AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
          });

        it("should do an AND on being given the same BDD twice", [&]() {
            __bdd out = bdd_apply(bdd_1, bdd_1, and_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_1._file_ptr));
            AssertThat(out.negate, Is().False());
          });

        it("should do an IMPLIES on being given the same BDD twice, where one is negated [1]", [&]() {
            __bdd out = bdd_apply(bdd_not(bdd_2), bdd_2, imp_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_2._file_ptr));
            AssertThat(out.negate, Is().False()); // negated the already negated input doubly-negating
          });

        it("should do an IMPLIES on being given the same BDD twice, where one is negated [2]", [&]() {
            __bdd out = bdd_apply(bdd_2, bdd_not(bdd_2), imp_op);

            AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_2._file_ptr));
            AssertThat(out.negate, Is().True()); // negated the first of the two
          });
      });
  });
