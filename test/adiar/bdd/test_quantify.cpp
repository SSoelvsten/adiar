go_bandit([]() {
  describe("adiar/bdd/quantify.cpp", []() {
    ////////////////////////////////////////////////////////////////////////
    // Sink only BDDs
    node_file terminal_F;

    { // Garbage collect writer to free write-lock}
      node_writer nw_F(terminal_F);
      nw_F << node(false);
    }

    node_file terminal_T;

    { // Garbage collect writer to free write-lock
      node_writer nw_T(terminal_T);
      nw_T << node(true);
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 1
    /*
    //   1     ---- x0
    //  / \
    //  T 2    ---- x1
    //   / \
    //   F T
    */
    node_file bdd_1;

    node n1_2 = node(1, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
    node n1_1 = node(0, node::MAX_ID, ptr_uint64(true), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_1(bdd_1);
      nw_1 << n1_2 << n1_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 2
    /*
    //       1       ---- x0
    //      / \
    //     2   3     ---- x1
    //    / \ / \
    //   4   5  F    ---- x2
    //  / \ / \
    //  T F F T
    */
    node_file bdd_2;

    node n2_5 = node(2, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
    node n2_4 = node(2, node::MAX_ID-1, ptr_uint64(true), ptr_uint64(false));
    node n2_3 = node(1, node::MAX_ID, n2_5.uid(), ptr_uint64(false));
    node n2_2 = node(1, node::MAX_ID-1, n2_4.uid(), n2_5.uid());
    node n2_1 = node(0, node::MAX_ID, n2_2.uid(), n2_3.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_2(bdd_2);
      nw_2 << n2_5 << n2_4 << n2_3 <<n2_2 << n2_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 3
    /*
    //       1       ---- x0
    //      / \
    //      |  2     ---- x1
    //      \ / \
    //       3   4   ---- x2
    //      / \ / \
    //      T F F T
    */
    node_file bdd_3;

    node n3_4 = node(2, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
    node n3_3 = node(2, node::MAX_ID-1, ptr_uint64(true), ptr_uint64(false));
    node n3_2 = node(1, node::MAX_ID, n3_3.uid(), n3_4.uid());
    node n3_1 = node(0, node::MAX_ID, n3_3.uid(), n3_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_3(bdd_3);
      nw_3 << n3_4 << n3_3 << n3_2 << n3_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 4
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
    node_file bdd_4;

    node n4_5 = node(3, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
    node n4_4 = node(2, node::MAX_ID, n4_5.uid(), ptr_uint64(true));
    node n4_3 = node(2, node::MAX_ID-1, ptr_uint64(false), n4_5.uid());
    node n4_2 = node(1, node::MAX_ID, n4_3.uid(), n4_4.uid());
    node n4_1 = node(0, node::MAX_ID, n4_3.uid(), n4_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_4(bdd_4);
      nw_4 << n4_5 << n4_4 << n4_3 << n4_2 << n4_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 5
    /*
    //    1      ---- x0
    //   / \
    //   F 2     ---- x1
    //    / \
    //   3   4   ---- x2
    //  / \ / \
    //  F T T F
    */
    node_file bdd_5;

    node n5_4 = node(2, node::MAX_ID, ptr_uint64(true), ptr_uint64(false));
    node n5_3 = node(2, node::MAX_ID-1, ptr_uint64(false), ptr_uint64(true));
    node n5_2 = node(1, node::MAX_ID, n5_3.uid(), n5_4.uid());
    node n5_1 = node(0, node::MAX_ID, ptr_uint64(false), n5_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_5(bdd_5);
      nw_5 << n5_4 << n5_3 << n5_2 << n5_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // x2 variable BDD
    node_file bdd_x2;

    { // Garbage collect writer to free write-lock
      node_writer nw_x2(bdd_x2);
      nw_x2 << node(2, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 6
    /*
    //         ___1__        ---- x0
    //        /      \
    //      _3__   __2__     ---- x1
    //     /    \ /     \
    //     4     6      5    ---- x2
    //    / \   / \    / \
    //    F  \ /  T   /  F
    //        8       7      ---- x3
    //       / \     / \
    //       F T     T F
    */
    node_file bdd_6;

    { // Garbage collect writer to free write-lock
      node_writer nw_6(bdd_6);
      nw_6 << node(3, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))       // 8
           << node(3, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))      // 7
           << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(true))       // 6
           << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID-1), ptr_uint64(false))      // 5
           << node(2, node::MAX_ID-2, ptr_uint64(false),      ptr_uint64(3, ptr_uint64::MAX_ID))   // 4
           << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-2), ptr_uint64(2, ptr_uint64::MAX_ID))   // 3
           << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(2, ptr_uint64::MAX_ID-1)) // 2
           << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(1, ptr_uint64::MAX_ID-1)) // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 7
    /*
    //          _1_        ---- x0
    //         /   \
    //         3   2       ---- x1
    //        / \ / \
    //        \_ | __\
    //           |    \
    //           4    5    ---- x2
    //          / \  / \
    //          T F  F T
    */
    node_file bdd_7;

    { // Garbage collect writer to free write-lock
      node_writer nw_7(bdd_7);
      nw_7 << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))       // 5
           << node(2, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(false))      // 4
           << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(2, ptr_uint64::MAX_ID-1)) // 3
           << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))   // 2
           << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(1, ptr_uint64::MAX_ID-1)) // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 8 (b is mirrored on the nodes for x2)
    /*
    //           __1__         ---- x0
    //          /     \
    //         _2_     \       ---- x1
    //        /   \     \
    //        3    4    |      ---- x2
    //       / \  / \   |
    //       T  \ F  \  |
    //           \____\ |
    //                 \|
    //                  5      ---- x3
    //                 / \
    //                 F T
    */
    node_file bdd_8a, bdd_8b;

    { // Garbage collect writer to free write-lock
      node_writer nw_8a(bdd_8a);
      nw_8a << node(3, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))       // 5
            << node(2, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(3, ptr_uint64::MAX_ID))   // 4
            << node(2, node::MAX_ID-1, ptr_uint64(true),       ptr_uint64(3, ptr_uint64::MAX_ID))   // 3
            << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))   // 2
            << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(3, ptr_uint64::MAX_ID))   // 1
        ;

      node_writer nw_8b(bdd_8b);
      nw_8b << node(3, node::MAX_ID,   ptr_uint64(false),      ptr_uint64(true))       // 5
            << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(false))      // 4
            << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(true))       // 3
            << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))   // 2
            << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(3, ptr_uint64::MAX_ID))   // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    describe("bdd_exists", [&]() {
      it("should quantify T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_T, 42);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_T._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_F, 21);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_F._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut quantification of root into T terminal [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 0);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should shortcut quantification of root into T terminal [x2]", [&]() {
        __bdd out = bdd_exists(bdd_x2, 2);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should shortcut quantification on non-existent label in input [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 42);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_1._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify bottom-most nodes [BDD 1]", [&]() {
        tpie::file_stream<arc> reduce_node_arcs;
        __bdd out = bdd_exists(bdd_1, 1);

        node_arcest_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should quantify root without terminal arcs [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 0);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should quantify nodes with terminal or nodes as children [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 2);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 5.high()
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // false due to its own leaf
                   Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should keep nodes as is when skipping quantified level [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 1);

        node_arcest_stream node_arcs(out);

        // Note, that node (2,0) reflects (3,NIL) since while n4 < NIL we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // n3
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // n3
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 4.high()
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 2);

        node_arcest_stream node_arcs(out);

        // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 4.high()
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should resolve terminal-terminal requests in [BDD 5]", [&]() {
        __bdd out = bdd_exists(bdd_5, 1);

        node_arcest_stream node_arcs(out);

        // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 3.high()
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can shortcut/prune irrelevant subtrees [OR-chain]", [&]() {
        node_file bdd_chain;

        node n4 = node(3, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
        node n3 = node(2, node::MAX_ID, n4.uid(), ptr_uint64(true));
        node n2 = node(1, node::MAX_ID, n3.uid(), ptr_uint64(true));
        node n1 = node(0, node::MAX_ID, n2.uid(), ptr_uint64(true));

        { // Garbage collect writer to free write-lock
          node_writer bdd_chain_w(bdd_chain);
          bdd_chain_w << n4 << n3 << n2 << n1;
        }

        __bdd out = bdd_exists(bdd_chain, 2);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to quantification of x2
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_exists(bdd_6, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (4,6)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (5,6)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (7,8)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (8,F)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);
        AssertThat(terminal_arcs.can_pull(), Is().True()); // (4,6)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (5,6)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (7,8)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(true) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (8,F)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(true) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(5u));
      });

      it("can forward multiple arcs to the same node across a level [BDD 7]", [&]() {
        __bdd out = bdd_exists(bdd_7, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (4,5)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);
        AssertThat(terminal_arcs.can_pull(), Is().True()); // (4,5)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));
        AssertThat(terminal_arcs.can_pull(), Is().True()); // (4,5)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_exists(bdd_8a, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(3,0) }));
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(terminal_arcs.pull(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(true) }));


        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8b]", [&]() {
        __bdd out = bdd_exists(bdd_8b, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(3,0) }));
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(terminal_arcs.pull(), // true due to 3.low()
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(true) }));


        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can quantify list [x1, x2] in terminal-only BDD [&&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2;
        }

        __bdd out = bdd_exists(terminal_T, labels);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_T._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("can quantify list [x1, x2] in terminal-only BDD [const &bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2;
        }

        bdd in_bdd = terminal_T;
        __bdd out = bdd_exists(in_bdd, labels);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_T._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("can quantify list [x1, x2] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("can quantify list [x2, x1] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 2 << 1;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("should quantify singleton list [x2] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 2;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("should quantify list [x1, x3] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 3;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                              ptr_uint64(2, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("should quantify list [x0, x2] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("should quantify list [x0, x2] [BDD 4 : const &bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2;
        }

        bdd in_bdd = bdd_4;
        bdd out = bdd_exists(in_bdd, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                              ptr_uint64(3, ptr_uint64::MAX_ID),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("should quantify list [x3, x1, x0, x2] where it is terminal-only already before x2 [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 3 << 1 << 0 << 2;
        }

        bdd out = bdd_exists(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("should quantify list [x2, x1] into T terminal [x2 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 2 << 1;
        }

        bdd out = bdd_exists(bdd_x2, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("should quantify list [] into the original file [BDD 3 : &&bdd]", [&]() {
        label_file labels;

        // __bdd is used to access the node_file
        __bdd out = bdd_exists(bdd_3, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_3._file_ptr));
      });

      it("should quantify list [] into the original file [BDD 3 : const &bdd]", [&]() {
        label_file labels;

        bdd in_bdd = bdd_3;
        __bdd out = bdd_exists(in_bdd, labels);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_3._file_ptr));
      });
    });

    ////////////////////////////////////////////////////////////////////////////
    // We will not test the Forall operator as much, since it is the same
    // underlying algorithm, but just with the AND operator.
    describe("bdd_forall", [&]() {
      it("should quantify T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_T, 42);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_T._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_F, 21);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(terminal_F._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify root with non-shortcutting terminal [BDD 1]", [&]() {
        __bdd out = bdd_forall(bdd_1, 0);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should quantify root of [BDD 3]", [&]() {
        __bdd out = bdd_forall(bdd_3, 0);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should prune shortcuttable requests [BDD 4]", [&]() {
        __bdd out = bdd_forall(bdd_4, 2);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // false due to 5.low()
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), // true due to 5.high() and 4.high()
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(true) }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_forall(bdd_6, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (4,6)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (5,6)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (7,8)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (8,T)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);
        AssertThat(terminal_arcs.can_pull(), Is().True()); // (4,6)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (5,6)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (7,8)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (8,T)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(true) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_forall(bdd_8a, 1);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(3,0) }));
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (3,4)
        AssertThat(terminal_arcs.pull(), // false due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(false) }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // (5,_)
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(false) }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(),
                   Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(true) }));


        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should quantify list [x0, x2, x1] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 1;
        }

        __bdd out = bdd_forall(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should quantify list [x0, x2, x1] [BDD 4 : const &bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 1;
        }

        bdd in_bdd = bdd_4;
        __bdd out = bdd_forall(in_bdd, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should quantify list [x1] [BDD 4 : &&bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1;
        }

        bdd out = bdd_forall(bdd_4, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (5,_) / (5,T)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (3,_) / (3,4)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(3, ptr_uint64::MAX_ID))));

        // The node (1,_) is reduced away due to its low child is (3,_)
        // and its high child is (3,4)

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("should quantify list [x1] [BDD 4 : const &bdd]", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1;
        }

        bdd in_bdd = bdd_4;
        bdd out = bdd_forall(in_bdd, labels);

        nodeest_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (5,_) / (5,T)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (3,_) / (3,4)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                              ptr_uint64(false),
                                                              ptr_uint64(3, ptr_uint64::MAX_ID))));

        // The node (1,_) is reduced away due to its low child is (3,_)
        // and its high child is (3,4)

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node> ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3u,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2u,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("should quantify list [] into the original file [BDD 3 : &&bdd]", [&]() {
        label_file labels;

        // __bdd is used to access the node_file
        __bdd out = bdd_forall(bdd_3, labels);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_3._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify list [] into the original file [BDD 3 : const &bdd]", [&]() {
        label_file labels;

        bdd in_bdd = bdd_3;
        __bdd out = bdd_forall(in_bdd, labels);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd_3._file_ptr));
        AssertThat(out.negate, Is().False());
      });
    });
  });
 });
