go_bandit([]() {
  describe("adiar/zdd/expand.cpp", [&]() {
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

    node_file zdd_x1;
    /*
             1              ---- x1
            / \
            F T
    */

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_x1);
      nw << create_node(1, MAX_ID, sink_F, sink_T);
    }

    node_file zdd_1;
    /*
             _1_            ---- x2
            /   \
            2   3           ---- x4
           / \ / \
           F _4_ T          ---- x6
            /   \
            5   6           ---- x8
           / \ / \
           7 T T T          ---- x10
          / \
          F T
     */

    const node_t n1_7 = create_node(10, MAX_ID,   sink_F,   sink_T);
    const node_t n1_6 = create_node(8,  MAX_ID,   sink_T,   sink_T);
    const node_t n1_5 = create_node(8,  MAX_ID-1, n1_7.uid, sink_T);
    const node_t n1_4 = create_node(6,  MAX_ID,   n1_5.uid, n1_6.uid);
    const node_t n1_3 = create_node(4,  MAX_ID,   n1_4.uid, sink_T);
    const node_t n1_2 = create_node(4,  MAX_ID-1, sink_F,   n1_4.uid);
    const node_t n1_1 = create_node(2,  MAX_ID,   n1_2.uid, n1_3.uid);

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_1);
      nw << n1_7 << n1_6 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    it("returns same file for Ø on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_expand(zdd_F, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
    });

    it("returns same file for { Ø } on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_expand(zdd_T, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_T._file_ptr));
    });

    it("returns same file for { {1} } on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_expand(zdd_x1, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
    });

    it("returns same file for [1] on empty labels", [&]() {
      label_file labels;

      __zdd out = zdd_expand(zdd_x1, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
    });

    it("returns same file for Ø on (1,2)", [&]() {
      label_file labels;

      {  // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1 << 2;
      }

      __zdd out = zdd_expand(zdd_F, labels);
      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
    });

    it("returns don't care node for { Ø } on (42)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 42;
      }

      __zdd out = zdd_expand(zdd_T, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(42,MAX_ID, sink_T, sink_T)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(42,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[false][false], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[true][false], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[false][true], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[true][true], Is().EqualTo(2u));

      AssertThat(out.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("returns don't care chain for { Ø } on (0,2)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_expand(zdd_T, labels);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2,MAX_ID, sink_T, sink_T)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[false][false], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[true][false], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[false][true], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()._file_ptr->max_1level_cut[true][true], Is().EqualTo(2u));

      AssertThat(out.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("adds new root for { { 1 } } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(1, MAX_ID, sink_F, sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("adds node chain for { { 3 }, { 3,4 } } on (0,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(4, MAX_ID, sink_F, sink_T)
          << create_node(3, MAX_ID, sink_T, create_node_ptr(4,MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("adds nodes before and after for { Ø, { 3 } } on (0,2,4)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(3, MAX_ID, sink_T, sink_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2 << 4;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("adds nodes for T sink but not F sink for { { 1,2 }, { 1,3 } } on (4,5,6)", [&]() {
      // Alternative title: Only creates one sink-chain.

      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(3, MAX_ID, sink_F, sink_T)
          << create_node(2, MAX_ID, create_node_ptr(3, MAX_ID), sink_T)
          << create_node(1, MAX_ID, sink_F, create_node_ptr(2, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 4 << 5 << 6;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), create_node_ptr(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), create_node_ptr(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(5,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(6,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("adds node in between levels { { 0,2 } } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, sink_F, create_node_ptr(2, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("adds nodes in between levels { { 0,3 } } on (1,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(3, MAX_ID, sink_F, sink_T)
          << create_node(0, MAX_ID, sink_F, create_node_ptr(3, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1 << 2;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("adds different don't care nodes on different arcs cut for { {0}, {2}, { 0,2 } } on (1,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << create_node(2, MAX_ID,   sink_T, sink_T)
          << create_node(2, MAX_ID-1, sink_F, sink_T)
          << create_node(0, MAX_ID,   create_node_ptr(2, MAX_ID-1), create_node_ptr(2, MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_expand(in, labels);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), create_node_ptr(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(4u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
    });

    it("adds don't care nodes before, in between, and after for [1] on (0,1,3,5,7,9,11)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1 << 3 << 5 << 7 << 9 << 11;
      }

      __zdd out = zdd_expand(zdd_1, labels);

      node_arc_test_stream node_arcs(out);

      // Pre-chain
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

      // (1) and its x3 cut
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

      // (2) and (3) and their x5 cut
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), create_node_ptr(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), create_node_ptr(4,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), create_node_ptr(4,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,0)), create_node_ptr(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,1), create_node_ptr(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(4,1)), create_node_ptr(5,1) }));

      // (4) and the x7 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,0), create_node_ptr(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,0)), create_node_ptr(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(6,0), create_node_ptr(7,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(6,0)), create_node_ptr(7,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(5,1), create_node_ptr(7,2) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(5,1)), create_node_ptr(7,2) }));

      // (5) and (6) and their x9 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,0), create_node_ptr(8,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,0)), create_node_ptr(8,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,1), create_node_ptr(8,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,1)), create_node_ptr(8,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(8,0), create_node_ptr(9,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(7,2), create_node_ptr(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(7,2)), create_node_ptr(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(8,0)), create_node_ptr(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(8,1), create_node_ptr(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(8,1)), create_node_ptr(9,1) }));

      // (7) and the x11 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(9,0), create_node_ptr(10,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(9,0)), create_node_ptr(10,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(9,1), create_node_ptr(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(9,1)), create_node_ptr(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(10,0)), create_node_ptr(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(4,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(10,0), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { create_node_ptr(11,0), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(11,0)), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(5,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(6,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(7,3u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(8,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(9,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(10,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(11,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()._file_ptr->max_1level_cut, Is().EqualTo(6u));

      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
    });
  });
 });
