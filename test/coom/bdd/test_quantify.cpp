go_bandit([]() {
    describe("BDD: Quantify", [&]() {
        ////////////////////////////////////////////////////////////////////////
        // Sink only OBDDs
        node_file sink_F;

        { // Garbage collect writer to free write-lock}
          node_writer nw_F(sink_F);
          nw_F << create_sink(false);
        }

        node_file sink_T;

        { // Garbage collect writer to free write-lock
          node_writer nw_T(sink_T);
          nw_T << create_sink(true);
        }

        ////////////////////////////////////////////////////////////////////////
        // OBDD 1
        /*
        //   1     ---- x0
        //  / \
        //  T 2    ---- x1
        //   / \
        //   F T
        */
        node_file obdd_1;

        node_t n1_2 = create_node(1,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        node_t n1_1 = create_node(0,MAX_ID, create_sink_ptr(true), n1_2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_1(obdd_1);
          nw_1 << n1_2 << n1_1;
        }

        ////////////////////////////////////////////////////////////////////////
        // OBDD 2
        /*
        //       1       ---- x0
        //      / \
        //     2   3     ---- x1
        //    / \ / \
        //   4   5  F    ---- x2
        //  / \ / \
        //  T F F T
        */
        node_file obdd_2;

        node_t n2_5 = create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        node_t n2_4 = create_node(2,MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false));
        node_t n2_3 = create_node(1,MAX_ID, n2_5.uid, create_sink_ptr(false));
        node_t n2_2 = create_node(1,MAX_ID-1, n2_4.uid, n2_5.uid);
        node_t n2_1 = create_node(0,MAX_ID, n2_2.uid, n2_3.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_2(obdd_2);
          nw_2 << n2_5 << n2_4 << n2_3 <<n2_2 << n2_1;
        }

        ////////////////////////////////////////////////////////////////////////
        // OBDD 3
        /*
        //       1       ---- x0
        //      / \
        //      |  2     ---- x1
        //      \ / \
        //       3   4   ---- x2
        //      / \ / \
        //      T F F T
        */
        node_file obdd_3;

        node_t n3_4 = create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        node_t n3_3 = create_node(2,MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false));
        node_t n3_2 = create_node(1,MAX_ID, n3_3.uid, n3_4.uid);
        node_t n3_1 = create_node(0,MAX_ID, n3_3.uid, n3_2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_3(obdd_3);
          nw_3 << n3_4 << n3_3 << n3_2 << n3_1;
        }

        ////////////////////////////////////////////////////////////////////////
        // OBDD 4
        /*
        //       1       ---- x0
        //      / \
        //      |  2     ---- x1
        //      \ / \
        //       3   4   ---- x2
        //      / \ / \
        //      F  5  T  ---- x3
        //        / \
        //        F T
        */
        node_file obdd_4;

        node_t n4_5 = create_node(3,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        node_t n4_4 = create_node(2,MAX_ID, n4_5.uid, create_sink_ptr(true));
        node_t n4_3 = create_node(2,MAX_ID-1, create_sink_ptr(false), n4_5.uid);
        node_t n4_2 = create_node(1,MAX_ID, n4_3.uid, n4_4.uid);
        node_t n4_1 = create_node(0,MAX_ID, n4_3.uid, n4_2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_4(obdd_4);
          nw_4 << n4_5 << n4_4 << n4_3 << n4_2 << n4_1;
        }

        ////////////////////////////////////////////////////////////////////////
        // OBDD 5
        /*
        //    1      ---- x0
        //   / \
        //   F 2     ---- x1
        //    / \
        //   3   4   ---- x2
        //  / \ / \
        //  F T T F
        */
        node_file obdd_5;

        node_t n5_4 = create_node(2,MAX_ID, create_sink_ptr(true), create_sink_ptr(false));
        node_t n5_3 = create_node(2,MAX_ID-1, create_sink_ptr(false), create_sink_ptr(true));
        node_t n5_2 = create_node(1,MAX_ID, n5_3.uid, n5_4.uid);
        node_t n5_1 = create_node(0,MAX_ID, create_sink_ptr(false), n5_2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_5(obdd_5);
          nw_5 << n5_4 << n5_3 << n5_2 << n5_1;
        }

        ////////////////////////////////////////////////////////////////////////////
        // x2 variable OBDD
        node_file obdd_x2;

        { // Garbage collect writer to free write-lock
          node_writer nw_x2(obdd_x2);
          nw_x2 << create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        }

        ////////////////////////////////////////////////////////////////////////////
        describe("Exists", [&]() {
            it("should quantify T sink-only OBDD as itself", [&]() {
                __bdd out = bdd_exists(sink_T, 42);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
              });

            it("should quantify F sink-only OBDD as itself", [&]() {
                __bdd out = bdd_exists(sink_F, 21);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_pull(), Is().False());

                AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification of root into T sink [OBDD 1]", [&]() {
                __bdd out = bdd_exists(obdd_1, 0);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification of root into T sink [x2]", [&]() {
                __bdd out = bdd_exists(obdd_x2, 2);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification on non-existent label in input [OBDD 1]", [&]() {
                __bdd out = bdd_exists(obdd_1, 42);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(n1_2));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(n1_1));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 1 }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("should quantify bottom-most nodes [OBDD 1]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                __bdd out = bdd_exists(obdd_1, 1);

                node_arc_test_stream node_arcs(out);
                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should quantify root without sink arcs [OBDD 2]", [&]() {
                __bdd out = bdd_exists(obdd_2, 0);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_node_ptr(2,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // (4,5)
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // (5,_)
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should quantify nodes with sink or nodes as children [OBDD 2]", [&]() {
                __bdd out = bdd_exists(obdd_2, 1);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(2,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // (4,5)
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // (5,_)
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should output sink arcs in order, despite the order of resolvement [OBDD 2]", [&]() {
                __bdd out = bdd_exists(obdd_2, 2);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(1,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 4.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 5.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 5.high
                           Is().EqualTo(arc_t { create_node_ptr(1,1), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // false due to its own leaf
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,1)), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should keep nodes as is when skipping quantified layer [OBDD 3]", [&]() {
                __bdd out = bdd_exists(obdd_3, 1);

                node_arc_test_stream node_arcs(out);

                // Note, that node (2,0) reflects (3,NIL) since while n4 < NIL we process this
                // request without forwarding n3 through the secondary priority queue
                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(2,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));


                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // n3
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // n3
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should output sink arcs in order, despite the order of resolvement [OBDD 3]", [&]() {
                __bdd out = bdd_exists(obdd_3, 2);

                node_arc_test_stream node_arcs(out);

                // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
                // request without forwarding n3 through the secondary priority queue
                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should resolve sink-sink requests in [OBDD 5]", [&]() {
                __bdd out = bdd_exists(obdd_5, 1);

                node_arc_test_stream node_arcs(out);

                // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
                // request without forwarding n3 through the secondary priority queue
                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 4.low
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 3.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("can shortcut/prune irrelevant subtrees [OR-chain]", [&]() {
                node_file obdd_chain;

                node_t n4 = create_node(3,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
                node_t n3 = create_node(2,MAX_ID, n4.uid, create_sink_ptr(true));
                node_t n2 = create_node(1,MAX_ID, n3.uid, create_sink_ptr(true));
                node_t n1 = create_node(0,MAX_ID, n2.uid, create_sink_ptr(true));

                { // Garbage collect writer to free write-lock
                  node_writer obdd_chain_w(obdd_chain);
                  obdd_chain_w << n4 << n3 << n2 << n1;
                }

                __bdd out = bdd_exists(obdd_chain, 2);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);
                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to quantification of x2
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("can quantify list [x1, x2] in sink-only OBDD", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 2;
                }

                __bdd out = bdd_exists(sink_T, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can quantify list [x1, x2] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 2;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 3 }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("can quantify list [x2, x1] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 2 << 1;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 3 }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("should quantify singleton list [x2] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 2;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_node_ptr(1,MAX_ID))));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 3u }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("should quantify list [x1, x3] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 3;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(2,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("should quantify list [x0, x2] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 0 << 2;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> out_meta(out);

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 3u }));

                AssertThat(out_meta.can_pull(), Is().True());
                AssertThat(out_meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(out_meta.can_pull(), Is().False());
              });

            it("should quantify list [x3, x1, x0, x2] where it is sink-only already before x2 [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 3 << 1 << 0 << 2;
                }

                bdd out = bdd_exists(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("should quantify list [x2, x1] into T sink [x2]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 2 << 1;
                }

                bdd out = bdd_exists(obdd_x2, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        ////////////////////////////////////////////////////////////////////////////
        // We will not test the Forall operator as much, since it is the same
        // underlying algorithm, but just with the AND operator.
        describe("Forall", [&]() {
            it("should quantify T sink-only OBDD as itself", [&]() {
                __bdd out = bdd_forall(sink_T, 42);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("should quantify F sink-only OBDD as itself", [&]() {
                __bdd out = bdd_forall(sink_F, 21);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("should quantify root with non-shortcutting sink [OBDD 1]", [&]() {
                __bdd out = bdd_forall(obdd_1, 0);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));


                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should quantify root of [OBDD 3]", [&]() {
                __bdd out = bdd_forall(obdd_3, 0);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_node_ptr(2,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 2u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should prune shortcuttable requests [OBDD 4]", [&]() {
                __bdd out = bdd_forall(obdd_4, 2);

                node_arc_test_stream node_arcs(out);

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

                AssertThat(node_arcs.can_pull(), Is().True());
                AssertThat(node_arcs.pull(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(3,0) }));

                AssertThat(node_arcs.can_pull(), Is().False());

                sink_arc_test_stream sink_arcs(out);

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // false due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // false due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // false due to 5.low
                           Is().EqualTo(arc_t { create_node_ptr(3,0), create_sink_ptr(false) }));

                AssertThat(sink_arcs.can_pull(), Is().True());
                AssertThat(sink_arcs.pull(), // true due to 5.high and 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(3,0)), create_sink_ptr(true) }));

                AssertThat(sink_arcs.can_pull(), Is().False());

                meta_test_stream<arc_t, 2> meta(out);

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 0u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 1u }));

                AssertThat(meta.can_pull(), Is().True());
                AssertThat(meta.pull(), Is().EqualTo(meta_t { 3u }));

                AssertThat(meta.can_pull(), Is().False());
              });

            it("should quantify list [x0, x2, x1] [OBDD 4]", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 0 << 2 << 1;
                }

                bdd out = bdd_forall(obdd_4, labels);

                node_test_stream out_nodes(out);

                AssertThat(out_nodes.can_pull(), Is().True());
                AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(out);
                AssertThat(ms.can_pull(), Is().False());
              });
          });
      });
  });
