go_bandit([]() {
  describe("adiar/zdd/change.cpp", [&]() {
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << node(false);

      node_writer nw_T(zdd_T);
      nw_T << node(true);
    }

    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    node_file zdd_x0;
    node_file zdd_x1;

    { // Garbage collect writers to free write-lock
      node_writer nw_0(zdd_x0);
      nw_0 << node(0, node::MAX_ID, terminal_F, terminal_T);

      node_writer nw_1(zdd_x1);
      nw_1 << node(1, node::MAX_ID, terminal_F, terminal_T);
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

    const node n1_5 = node(3, node::MAX_ID,   terminal_F,   terminal_T);
    const node n1_4 = node(2, node::MAX_ID,   terminal_T,   terminal_T);
    const node n1_3 = node(2, node::MAX_ID-1, n1_5.uid(), terminal_T);
    const node n1_2 = node(1, node::MAX_ID,   n1_3.uid(), n1_4.uid());
    const node n1_1 = node(0, node::MAX_ID,   n1_2.uid(), n1_4.uid());

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

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, terminal_F, terminal_T)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds new root for { {1} } on (0)", [&]() {
      label_file labels;

      {  // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds new root chain for { {2} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_T, terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("adds new nodes after root for { {1} } on (2,3)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2 << 3;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds a new node before and after root for { {1} } on (0,2)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds new nodes before and after root for { {1} } on (0,2,4)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2 << 4;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(4u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds node between root and child for { {1}, {3} } on (2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(3, node::MAX_ID, terminal_F, terminal_T)
          << node(1, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), terminal_T)
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("returns { Ø } for { {0} } on (0)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_x0, labels);

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("returns { Ø } for { {1} } on (1)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(zdd_x1, labels);

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("shortcuts root for { {0,1} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(1, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("shortcuts root for { {0,2} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("shortcuts root and its child for { {0,2}, {0,2,3} } on (0,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(3, node::MAX_ID, terminal_T, terminal_T)
          << node(2, node::MAX_ID, terminal_F, ptr_uint64(3, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("flips root for { Ø, {0} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(0, node::MAX_ID, terminal_T, terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("flips root for { {0}, {1} } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(1, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("flips root for [1] on (0)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_change(zdd_1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(4u));
    });

    it("flips node in the middle for { Ø, {0}, {1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(1, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
    });

    it("flips and adds a node for [1] on (1)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(zdd_1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(4u));
    });

    it("collapses to a terminal for { {0,1} } on (0,1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(1, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(true)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("bridges over a deleted node for { {0,1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(1, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("bridges over a deleted node for { {1,2}, {0,1,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(1, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("only adds a one node when cutting arcs to the same node for { {2}, {0,2} } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("bridges node before the last label and a terminal for { {0}, {1} } on (0,1)", [&]() {
      /*
              1     ---- x0
             / \
             2 T    ---- x1
            / \
            F T

         When resoving for (2) the label is at x1, but as it is pruned then the
         source is still at x0 from (1). Hence, if one looks at source.label()
         rather than the current level then this edge is by mistake placed into
         the "cut edge with a new node" queue.
       */
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(1, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
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
              *       ---- x3 (From the T terminal)
             / \
             F T
       */

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 2 << 3;
      }

      __zdd out = zdd_change(zdd_1, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("correctly connects pre-root chain with skipped root for { { 1 }, { 1,2 } } on (0,1)", [&]() {
      node_file in;
      /*
                1     ---- x1
               / \
               F 2    ---- x2
                / \
                T T

       */

      const node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
      const node n1 = node(1, node::MAX_ID, terminal_F, n2.uid());

      {
        node_writer nw(in);
        nw << n2 << n1;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("cuts collapse of root to a terminal for { { 0 } } on (0,1)", [&]() {
      node_file in;
      /*
                1     ---- x0
               / \
               F T
      */
      { // Garbage collect writer to free write-lock
        node_writer nw(in);
        nw << node(0, node::MAX_ID, terminal_F, terminal_T);
      }

      label_file labels;
      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("cuts collapse to a terminal for { { 0,1 } } on (0,1,2)", [&]() {
      node_file in;
      /*
                1     ---- x0
               / \
               F 2    ---- x1
                / \
                F T
      */
      { // Garbage collect writer to free write-lock
        node_writer nw(in);
        nw << node(1, node::MAX_ID, terminal_F, terminal_T)
           << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      label_file labels;
      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1 << 2;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("keeps pre-root chain despite collapse to a terminal of the root for { { 1 } } on (0,1)", [&]() {
      node_file in;
      /*
                1     ---- x1
               / \
               F T
      */
      { // Garbage collect writer to free write-lock
        node_writer nw(in);
        nw << node(1, node::MAX_ID, terminal_F, terminal_T);
      }

      label_file labels;
      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1;
      }

      __zdd out = zdd_change(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });
  });
 });
