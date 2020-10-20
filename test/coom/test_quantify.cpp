#include <coom/quantify.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Quantify", [&]() {
        ////////////////////////////////////////////////////////////////////////
        // Sink only OBDDs
        tpie::file_stream<node_t> sink_F;
        sink_F.open();
        sink_F.write(create_sink(false));

        tpie::file_stream<node_t> sink_T;
        sink_T.open();
        sink_T.write(create_sink(true));

            tpie::file_stream<meta_t> sink_meta;
        sink_meta.open();

        ////////////////////////////////////////////////////////////////////////
        // OBDD 1
        /*
        //   1     ---- x0
        //  / \
            //  T 2    ---- x1
        //   / \
        //   F T
        */
            tpie::file_stream<node_t> obdd_1;
        obdd_1.open();

        node_t n1_2 = create_node(1,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
            obdd_1.write(n1_2);

        node_t n1_1 = create_node(0,MAX_ID, create_sink_ptr(true), n1_2.uid);
        obdd_1.write(n1_1);

        tpie::file_stream<meta_t> meta_1;
        meta_1.open();

        meta_1.write({ 1 });
        meta_1.write({ 0 });

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
        tpie::file_stream<node_t> obdd_2;
        obdd_2.open();

        node_t n2_5 = create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        obdd_2.write(n2_5);

        node_t n2_4 = create_node(2,MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false));
        obdd_2.write(n2_4);

        node_t n2_3 = create_node(1,MAX_ID, n2_5.uid, create_sink_ptr(false));
        obdd_2.write(n2_3);

        node_t n2_2 = create_node(1,MAX_ID-1, n2_4.uid, n2_5.uid);
        obdd_2.write(n2_2);

        node_t n2_1 = create_node(0,MAX_ID, n2_2.uid, n2_3.uid);
        obdd_2.write(n2_1);

        tpie::file_stream<meta_t> meta_2;
        meta_2.open();

        meta_2.write({ 2 });
        meta_2.write({ 1 });
        meta_2.write({ 0 });


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
        tpie::file_stream<node_t> obdd_3;
        obdd_3.open();

        node_t n3_4 = create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        obdd_3.write(n3_4);

        node_t n3_3 = create_node(2,MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false));
        obdd_3.write(n3_3);

        node_t n3_2 = create_node(1,MAX_ID, n3_3.uid, n3_4.uid);
        obdd_3.write(n3_2);

        node_t n3_1 = create_node(0,MAX_ID, n3_3.uid, n3_2.uid);
        obdd_3.write(n3_1);

        tpie::file_stream<meta_t> meta_3;
        meta_3.open();

        meta_3.write({ 2 });
        meta_3.write({ 1 });
        meta_3.write({ 0 });

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
        tpie::file_stream<node_t> obdd_4;
        obdd_4.open();

        node_t n4_5 = create_node(3,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
        obdd_4.write(n4_5);

        node_t n4_4 = create_node(2,MAX_ID, n4_5.uid, create_sink_ptr(true));
        obdd_4.write(n4_4);

        node_t n4_3 = create_node(2,MAX_ID-1, create_sink_ptr(false), n4_5.uid);
        obdd_4.write(n4_3);

        node_t n4_2 = create_node(1,MAX_ID, n4_3.uid, n4_4.uid);
        obdd_4.write(n4_2);

        node_t n4_1 = create_node(0,MAX_ID, n4_3.uid, n4_2.uid);
        obdd_4.write(n4_1);

        tpie::file_stream<meta_t> meta_4;
        meta_4.open();

        meta_4.write({ 3 });
        meta_4.write({ 2 });
        meta_4.write({ 1 });
        meta_4.write({ 0 });

        ////////////////////////////////////////////////////////////////////////////
        // x2 variable OBDD
        tpie::file_stream<node_t> obdd_x2;
        obdd_x2.open();
        obdd_x2.write(create_node(2,MAX_ID, create_sink_ptr(false), create_sink_ptr(true)));

        tpie::file_stream<meta_t> meta_x2;
        meta_x2.open();
        meta_x2.write({2});

        ////////////////////////////////////////////////////////////////////////////
        describe("COOM: Exists", [&]() {
            it("should quantify T sink-only OBDD as itself", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(42, sink_T, sink_meta, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should quantify F sink-only OBDD as itself", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(21, sink_F, sink_meta, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification of root into T sink [OBDD 1]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(0, obdd_1, meta_1, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification of root into T sink [x2]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(2, obdd_x2, meta_x2, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should shortcut quantification on non-existent label in input [OBDD 1]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(42, obdd_1, meta_1, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(n1_2));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(n1_1));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify bottom-most nodes [OBDD 1]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(1, obdd_1, meta_1, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                AssertThat(reduce_node_arcs.size(), Is().EqualTo(0u));

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should quantify root without sink arcs [OBDD 2]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(0, obdd_2, meta_2, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_node_ptr(2,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // (4,5)
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // (5,_)
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should quantify nodes with sink or nodes as children [OBDD 2]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(1, obdd_2, meta_2, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(2,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // (4,5)
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // (5,_)
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should output sink arcs in order, despite the order of resolvement [OBDD 2]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(2, obdd_2, meta_2, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(1,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 4.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 5.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 5.high
                           Is().EqualTo(arc_t { create_node_ptr(1,1), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // false due to its own leaf
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,1)), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should keep nodes as is when skipping quantified layer [OBDD 3]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(1, obdd_3, meta_3, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                // Note, that node (2,0) reflects (3,NIL) since while n4 < NIL we process this
                // request without forwarding n3 through the secondary priority queue
                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_node_ptr(2,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));


                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // n3
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // n3
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should output sink arcs in order, despite the order of resolvement [OBDD 3]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(2, obdd_3, meta_3, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
                // request without forwarding n3 through the secondary priority queue
                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("can shortcut/prune irrelevant subtrees [OR-chain]", [&]() {
                tpie::file_stream<node_t> obdd_chain;
                obdd_chain.open();

                node_t n4 = create_node(3,MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
                obdd_chain.write(n4);

                node_t n3 = create_node(2,MAX_ID, n4.uid, create_sink_ptr(true));
                obdd_chain.write(n3);

                node_t n2 = create_node(1,MAX_ID, n3.uid, create_sink_ptr(true));
                obdd_chain.write(n2);

                node_t n1 = create_node(0,MAX_ID, n2.uid, create_sink_ptr(true));
                obdd_chain.write(n1);

                tpie::file_stream<meta_t> meta_chain;
                meta_chain.open();

                meta_chain.write({ 3 });
                meta_chain.write({ 2 });
                meta_chain.write({ 1 });
                meta_chain.write({ 0 });

                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                exists(2, obdd_chain, meta_chain, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);
                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to quantification of x2
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("can quantify list [x1, x2] in sink-only OBDD", [&]() {
                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(1);
                labels.write(2);

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(labels, sink_T, sink_meta, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("can quantify list [x1, x2] [OBDD 4]", [&]() {
                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(1);
                labels.write(2);

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);


                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("can quantify list [x2, x1] [OBDD 4]", [&]() {
                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(2);
                labels.write(1);

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);


                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify singleton list [x2] [OBDD 4]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(2);

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_node_ptr(1,MAX_ID))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3u }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify list [x1, x3] [OBDD 4]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(1);
                labels.write(3);

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0,MAX_ID,
                                                                      create_node_ptr(2,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 2u }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify list [x0, x2] [OBDD 4]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(0);
                labels.write(2);

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1,MAX_ID,
                                                                      create_node_ptr(3,MAX_ID),
                                                                      create_sink_ptr(true))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3u }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify list [x3, x1, x0, x2] where it is sink-only already before x2 [OBDD 4]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(3);
                labels.write(1);
                labels.write(0);
                labels.write(2);

                exists(labels, obdd_4, meta_4, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should quantify list [x2, x1] into T sink [x2]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(2);
                labels.write(1);

                exists(labels, obdd_x2, meta_x2, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });
          });

        ////////////////////////////////////////////////////////////////////////////
        // We will not test the Forall operator as much, since it is the same
        // underlying algorithm, but just with the AND operator.
        describe("COOM: Forall", [&]() {
            it("should quantify T sink-only OBDD as itself", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                forall(42, sink_T, sink_meta, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should quantify F sink-only OBDD as itself", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                forall(21, sink_F, sink_meta, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("should quantify root with non-shortcutting sink [OBDD 1]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                forall(0, obdd_1, meta_1, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1,MAX_ID,
                                                                      create_sink_ptr(false),
                                                                      create_sink_ptr(true))));
                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("should quantify root of [OBDD 3]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                forall(0, obdd_3, meta_3, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_node_ptr(2,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(2,0), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,0)), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { create_node_ptr(2,1), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(2,1)), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 2u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should prune shortcuttable requests [OBDD 4]", [&]() {
                tpie::file_stream<arc_t> reduce_node_arcs;
                reduce_node_arcs.open();

                tpie::file_stream<arc_t> reduce_sink_arcs;
                reduce_sink_arcs.open();

                tpie::file_stream<meta_t> reduce_meta;
                reduce_meta.open();

                forall(2, obdd_4, meta_4, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

                reduce_node_arcs.seek(0);

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().True());
                AssertThat(reduce_node_arcs.read(),
                           Is().EqualTo(arc_t { flag(create_node_ptr(1,0)), create_node_ptr(3,0) }));

                AssertThat(reduce_node_arcs.can_read(), Is().False());

                reduce_sink_arcs.seek(0);

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // false due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(0,0), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // false due to 3.low
                           Is().EqualTo(arc_t { create_node_ptr(1,0), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // false due to 5.low
                           Is().EqualTo(arc_t { create_node_ptr(3,0), create_sink_ptr(false) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().True());
                AssertThat(reduce_sink_arcs.read(), // true due to 5.high and 4.high
                           Is().EqualTo(arc_t { flag(create_node_ptr(3,0)), create_sink_ptr(true) }));

                AssertThat(reduce_sink_arcs.can_read(), Is().False());

                reduce_meta.seek(0);

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 0u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 1u }));

                AssertThat(reduce_meta.can_read(), Is().True());
                AssertThat(reduce_meta.read(), Is().EqualTo(meta_t { 3u }));

                AssertThat(reduce_meta.can_read(), Is().False());
              });

            it("should quantify list [x0, x2, x1] [OBDD 4]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                tpie::file_stream<label_t> labels;
                labels.open();

                labels.write(0);
                labels.write(2);
                labels.write(1);

                forall(labels, obdd_4, meta_4, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });
          });
      });
  });
