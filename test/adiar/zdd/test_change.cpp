go_bandit([]() {
  describe("ZDD: Change", [&]() {
    // == CREATE SINK-ONLY BDD FOR UNIT TESTS ==
    //                  START
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_sink(false);

      node_writer nw_T(zdd_T);
      nw_T << create_sink(true);
    }

    const ptr_t sink_F = create_sink_ptr(false);
    const ptr_t sink_T = create_sink_ptr(true);

    node_file zdd_x0;
    node_file zdd_x1;

    { // Garbage collect writers to free write-lock
      node_writer nw_0(zdd_x0);
      nw_0 << create_node(0, MAX_ID, sink_F, sink_T);

      node_writer nw_1(zdd_x1);
      nw_1 << create_node(1, MAX_ID, sink_F, sink_T);
    }

    /*
                1      ---- x0
               / \
               2  \    ---- x1
              / \ /
              3  4     ---- x2
             / \/ \
             5 T  T    ---- x3
            / \
            F T
     */
    node_file zdd_1;

    const node_t n1_5 = create_node(3, MAX_ID,   sink_F,   sink_T);
    const node_t n1_4 = create_node(2, MAX_ID,   sink_T,   sink_T);
    const node_t n1_3 = create_node(2, MAX_ID-1, n1_5.uid, sink_T);
    const node_t n1_2 = create_node(1, MAX_ID,   n1_3.uid, n1_4.uid);
    const node_t n1_1 = create_node(0, MAX_ID,   n1_2.uid, n1_4.uid);

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_1);
      nw << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    it("returns same file for Ø on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_change(zdd_F, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
    });

    it("returns same file for { Ø } on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_change(zdd_T, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_T._file_ptr));
    });

    it("returns same file for { {1} } on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_change(zdd_x1, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
    });

    it("returns same file for Ø on (1,2)", [&]() {
      label_file labels;

      {  // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1 << 2;
      }

      __zdd out = zdd_change(zdd_F, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
    });

    it("returns { {1,2} } for { Ø } on (1,2)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1 << 2;
      }

      __zdd out = zdd_change(zdd_T, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2,MAX_ID, sink_F, sink_T)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1,MAX_ID, sink_F, create_node_ptr(2,MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds new root for { {1} } on (0)", [&]() {
      label_file labels;

      {  // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds new root chain for { {2} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_T, sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds new nodes after root for { {1} } on (2,3)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2 << 3;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds a new node before and after root for { {1} } on (0,2)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds new nodes before and after root for { {1} } on (0,2,4)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2 << 4;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("adds node between root and child for { {1}, {3} } on (2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(3, MAX_ID, sink_F, sink_T)
          << create_node(1, MAX_ID, create_node_ptr(3, MAX_ID), sink_T)
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("returns { Ø } for { {0} } on (0)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_x0, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("returns { Ø } for { {1} } on (1)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("shortcuts root for { {0,1} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(1, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, sink_F, create_node_ptr(1, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("shortcuts root for { {0,2} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, sink_F, create_node_ptr(2, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("shortcuts root and its child for { {0,2}, {0,2,3} } on (0,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(3, MAX_ID, sink_T, sink_T)
          << create_node(2, MAX_ID, sink_F, create_node_ptr(3, MAX_ID))
          << create_node(0, MAX_ID, sink_F, create_node_ptr(2, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips root for { Ø, {0} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(0, MAX_ID, sink_T, sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips root for { {0}, {1} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(1, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID), sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

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

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips root for [1] on (0)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips node in the middle for { Ø, {0}, {1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(1, MAX_ID, sink_T, create_node_ptr(2, MAX_ID))
          << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID), sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips and adds a node for [1] on (1)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(zdd_1, labels);

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
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("collapses to a sink for { {0,1} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(1, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, sink_F, create_node_ptr(1, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("bridges over a deleted node for { {0,1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(1, MAX_ID, sink_F, create_node_ptr(2, MAX_ID))
          << create_node(0, MAX_ID, sink_F, create_node_ptr(1, MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("bridges over a deleted node for { {1,2}, {0,1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(1, MAX_ID, sink_F, create_node_ptr(2, MAX_ID))
          << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID), create_node_ptr(1, MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("only adds a one node when cutting arcs to the same node for { {2}, {0,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, create_node_ptr(2, MAX_ID), create_node_ptr(2, MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("bridges node before the last label and a sink for { {0}, {1} } on (0,1)", [&]() {
      /*
              1     ---- x0
             / \
             2 T    ---- x1
            / \
            F T

         When resoving for (2) the label is at x1, but as it is pruned then the
         source is still at x0 from (1). Hence, if one looks at label_of(source)
         rather than the current level then this edge is by mistake placed into
         the "cut edge with a new node" queue.
       */
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(1, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID), sink_T)
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

    it("flips, adds and bridges nodes for [1] on (2,3)", [&]() {
      /*
               1      ---- x0
              / \
              2  \    ---- x1
             / \ /
             3  4     ---- x2
            / \ ||
            | F ||            (The F is the shortcutting on 5)
            \_ _//
              *       ---- x3 (From the T sink)
             / \
             F T
       */

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2 << 3;
      }

      __zdd out = zdd_change(zdd_1, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t, ARC_FILE_COUNT> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());
    });

  });
 });
