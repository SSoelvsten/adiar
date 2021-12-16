go_bandit([]() {
  describe("ZDD: Subset derivation", [&]() {
    // == CREATE SINK-ONLY ZDD FOR UNIT TESTS ==
    //                  START
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_sink(false);

      node_writer nw_T(zdd_T);
      nw_T << create_sink(true);
    }

    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    /*
                  1      ---- x0
                  ||
                  2      ---- x1
                 / \
                 | 3     ---- x2
                 |/ \
                 4  5    ---- x3
                / \/ \
                F 6  T   ---- x4
                 / \
                 T T
     */

    node_t n6 = create_node(4, MAX_ID,   sink_T, sink_T);
    node_t n5 = create_node(3, MAX_ID,   n6.uid, sink_T);
    node_t n4 = create_node(3, MAX_ID-1, sink_F, n6.uid);
    node_t n3 = create_node(2, MAX_ID, n4.uid, n5.uid);
    node_t n2 = create_node(1, MAX_ID, n4.uid, n3.uid);
    node_t n1 = create_node(0, MAX_ID, n2.uid, n2.uid);

    node_file zdd_1;

    { // Garbage collect writer to free write-lock
      node_writer w(zdd_1);
      w << n6 << n5 << n4 << n3 << n2 << n1;
    }

    describe("zdd_offset", [&]() {
      it("should return input unchanged when given Ø", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 21 << 42;
        }

        __zdd out = zdd_offset(zdd_F, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
      });

      it("should return input unchanged when given { Ø }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 7 << 42;
        }

        __zdd out = zdd_offset(zdd_T, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_T._file_ptr));
      });

      it("should return input unchanged when given empty set of labels", [&]() {
        label_file labels;

        __zdd out = zdd_offset(zdd_1, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_1._file_ptr));
      });

      it("should return input unchanged when given { { 2 }, { 1,2 } } without (0,3,4,5,6)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 3 << 4 << 5 << 6;
        }

        node_file in;

        {
          node_writer nw(in);
          nw << create_node(2, MAX_ID, sink_F, sink_T)
             << create_node(1, MAX_ID, sink_T, create_node_ptr(2, MAX_ID));
        }

        __zdd out = zdd_offset(in, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(in._file_ptr));
      });

      it("should return { Ø } when given { Ø, { 1,2 } } without (0,2)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2;
        }

        node_file in;

        node_t n2 = create_node(2, MAX_ID, sink_F, sink_T);
        node_t n1 = create_node(1, MAX_ID, sink_T, n2.uid);;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n1.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t, ARC_FILE_COUNT> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should return { { 1 }, Ø } when given { Ø, { 1 }, { 2 }, { 1, 2 } } without (0,2)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2;
        }

        node_file in;

        node_t n2 = create_node(2, MAX_ID-1, sink_T, sink_T);
        node_t n1 = create_node(1, MAX_ID, n2.uid, n2.uid);;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n1.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t, ARC_FILE_COUNT> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should return { { 2 }, Ø } when given { Ø, { 1 }, { 2 }, { 1, 2 } } without (1,3)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 3;
        }

        node_file in;

        node_t n2 = create_node(2, MAX_ID, sink_T, sink_T);
        node_t n1 = create_node(1, MAX_ID, n2.uid, n2.uid);;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t, ARC_FILE_COUNT> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should skip root of [1] without (0,42)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 42;
        }

        __zdd out = zdd_offset(zdd_1, labels);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().True());

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid, n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n3.uid, n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n6.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n5.uid, n6.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n6.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n6.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t, ARC_FILE_COUNT> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(3,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should skip 'dead' nodes in [1] without (1)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1;
        }

        __zdd out = zdd_offset(zdd_1, labels);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().True());

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n6.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n6.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n6.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t, ARC_FILE_COUNT> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_meta(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should restrict to a sink in [1] without (0,1,3)", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 3;
        }

        __zdd out = zdd_offset(zdd_1, labels);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t, NODE_FILE_COUNT> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should restrict to a sink in { Ø, { 1 }, { 2, 3 } } without (1,2)", [&]() {
        /*
                  1       ---- x1
                 / \
                 2 T      ---- x2
                / \
                T 3
                 / \
                 F T
        */
        node_file in;

        {
          node_writer w(in);
          w << create_node(3, MAX_ID, sink_F, sink_T)
            << create_node(2, MAX_ID, sink_T, create_node_ptr(3, MAX_ID))
            << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), sink_T);
        }

        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2;
        }

        __zdd out = zdd_offset(in, labels);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t, NODE_FILE_COUNT> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());
      });

      it("should restrict to a sink from root", [&]() {
        /*
                 1       ---- x1
                / \
                T 2      ---- x2
                 / \
                 T T
         */
        node_file in;

        {
          node_writer w(in);
          w << create_node(2, MAX_ID, sink_T, sink_T)
            << create_node(1, MAX_ID, sink_T, create_node_ptr(2, MAX_ID));
        }


        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1;
        }

        __zdd out = zdd_offset(in, labels);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t, NODE_FILE_COUNT> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());
      });
    });
  });
 });
