go_bandit([]() {
  describe("adiar/zdd/binop.cpp", []() {
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_terminal(false);

      node_writer nw_T(zdd_T);
      nw_T << create_terminal(true);
    }

    ptr_t terminal_T = create_terminal_ptr(true);
    ptr_t terminal_F = create_terminal_ptr(false);

    node_file zdd_x0;
    node_file zdd_x1;

    { // Garbage collect writers early
      node_writer nw_x0(zdd_x0);
      nw_x0 << create_node(0,MAX_ID, terminal_F, terminal_T);

      node_writer nw_x1(zdd_x1);
      nw_x1 << create_node(1,MAX_ID, terminal_F, terminal_T);
    }

    describe("zdd_union", [&]() {
      it("should shortcut Ø on same file", [&]() {
        __zdd out = zdd_union(zdd_F, zdd_F);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
      });

      it("should shortcut { Ø } on same file", [&]() {
        __zdd out = zdd_union(zdd_T, zdd_T);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_T._file_ptr));
      });

      it("should shortcut { {0} } on same file", [&]() {
        __zdd out = zdd_union(zdd_x0, zdd_x0);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_x0._file_ptr));
      });

      it("should shortcut { {1} } on same file", [&]() {
        __zdd out = zdd_union(zdd_x1, zdd_x1);
        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
      });

      it("computes Ø U { {Ø} }", [&]() {
        __zdd out = zdd_union(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { Ø } U Ø", [&]() {
        __zdd out = zdd_union(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should shortcut on irrelevance for { {0} } U Ø", [&]() {
        /*
                   1     ---- x0
                  / \
                  F T
        */

        __zdd out_1 = zdd_union(zdd_x0, zdd_F);
        AssertThat(out_1.get<node_file>()._file_ptr, Is().EqualTo(zdd_x0._file_ptr));

        __zdd out_2 = zdd_union(zdd_F, zdd_x0);
        AssertThat(out_2.get<node_file>()._file_ptr, Is().EqualTo(zdd_x0._file_ptr));
      });

      it("computes { {0} } U { Ø }", [&]() {
        /*
                   1     ---- x0
                  / \
                  T T
        */

        __zdd out = zdd_union(zdd_x0, zdd_T);

        node_arc_test_stream node_arcs(out);
        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("computes { {0} } U { {1} }", [&]() {
        /*
                     1     ---- x0
                    / \
                    2 T    ---- x1
                   / \
                   F T
        */

        __zdd out = zdd_union(zdd_x0, zdd_x1);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("computes { {0,1}, {0,3} } U { {0,2}, {2} }", [&]() {
        /*
                    1           1                (1,1)         ---- x0
                   / \         / \                 X
                   F 2         | |            (2,2) \          ---- x1
                    / \        \ /   ==>      /   \  \
                    | T         2          (3,2)  T (F,2)      ---- x2
                    |          / \         /   \     / \
                    3          F T       (3,F) T     F T       ---- x3
                   / \                    / \
                   F T                    F T
        */
        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(3,MAX_ID, terminal_F, terminal_T)
               << create_node(1,MAX_ID, create_node_ptr(3,MAX_ID), terminal_T)
               << create_node(0,MAX_ID, terminal_F, create_node_ptr(1,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(4u));
      });

      it("computes { {0,1}, {1} } U { {0,2}, {2} }", [&]() {
        /*
                   1           1             (1,1)
                   ||         / \             | |
                   2          | |            (2,2)
                  / \         \ /   ==>       / \
                  F T          2          (F,2) (T,F) <-- since 2nd (2) skipped level
                              / \          / \
                              F T          F T
        */
        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(1,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("computes { {0}, {1,3}, {2,3}, {1} } U { {0,3}, {3} }", [&]() {
        /*
                        1        1                        (1,1)         ---- x0
                       / \      / \                       /   \
                       2 T      | |                    (2,2)   \        ---- x1
                      / \       | |                    /   \    \
                      3  \      | |     ==>         (3,2)  |     \      ---- x2
                      \\ /      \ /                 /   \  |     |
                        4        2              (4,2)   (4,F)  (T,2)    ---- x3
                       / \      / \              / \     / \    / \
                       F T      F T              T T     T T    T T

                   The high arc on (2) and (3) on the left is shortcutting the
                   second ZDD, to compensate for the omitted nodes.
        */
        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(3,MAX_ID, terminal_F, terminal_T)
               << create_node(2,MAX_ID, create_node_ptr(3,MAX_ID), create_node_ptr(3,MAX_ID))
               << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(3,MAX_ID))
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(3,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(3,MAX_ID), create_node_ptr(3,MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), create_node_ptr(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), create_node_ptr(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(3,2) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,1), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(3,2), terminal_T }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(3,2)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,3u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(4u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(4u));
      });
    });

    describe("zdd_intsec", [&]() {
      it("should shortcut on same file", [&]() {
        __zdd out_1 = zdd_intsec(zdd_x0, zdd_x0);
        AssertThat(out_1.get<node_file>()._file_ptr, Is().EqualTo(zdd_x0._file_ptr));

        __zdd out_2 = zdd_intsec(zdd_x1, zdd_x1);
        AssertThat(out_2.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
      });

      it("computes Ø ∩ { {Ø} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø } ∩ Ø", [&]() {
        __zdd out = zdd_intsec(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and shortcut) { {0} } ∩ Ø", [&]() {
        /*
                   1       F              F          ---- x0
                  / \           ==>
                  F T
        */

        __zdd out = zdd_intsec(zdd_x0, zdd_F);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and shortcut) Ø ∩ { {0} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { {0} } ∩ { Ø }", [&]() {
        /*
                   1       T              F       ---- x0
                  / \           ==>
                  F T
        */

        __zdd out = zdd_intsec(zdd_x0, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø, {0} } ∩ { Ø }", [&]() {
        /*
                   1       T              T       ---- x0
                  / \           ==>
                  T T
        */
        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(0,MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0}, {1} } ∩ { Ø }", [&]() {
        /*
                       1        T             F          ---- x0
                      / \
                      2 T             ==>                ---- x1
                     / \
                     F T
        */

        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), terminal_T)
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and shortcut) { {0,1}, {1} } ∩ { {0,1} }", [&]() {
        /*
                    _1_        1            1        ---- x0
                   /   \      / \          / \
                   2   3      F 2    ==>   F 2       ---- x1
                  / \ / \      / \          / \
                  T T F T      F T          F T
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(1,MAX_ID-1, terminal_T, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID-1), create_node_ptr(1,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, terminal_F, create_node_ptr(1,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes (and skip to terminal) { {0}, {1}, {0,1} } ∩ { Ø }", [&]() {
        /*
                    1        T          F     ---- x0
                   / \
                   \ /            ==>
                    2                         ---- x1
                   / \
                   F T
        */

        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(1,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and skip to terminal) { {0,2}, {0}, {2} } \\ { {1}, {2}, Ø }", [&]() {
        /*
                        1                             F       ---- x0
                       / \
                      /   \         _1_                         ---- x1
                      |   |        /   \       =>
                      2   3        2   3                        ---- x2
                     / \ / \      / \ / \
                     F T T T      T F F T

                     Where (2) and (3) are swapped in order on the right. Notice,
                     that (2) on the right technically is illegal, but it makes
                     for a much simpler counter-example that catches
                     prod_pq_1.peek() throwing an error on being empty.
        */

        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(2,MAX_ID, terminal_T, terminal_T)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(2,MAX_ID-1), create_node_ptr(2,MAX_ID))
            ;
        }

        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_T, terminal_F)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID-1))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and skips in) { {0,1,2}, {0,2}, {0}, {2} } } ∩ { {0,2}, {0}, {1}, {2} }", [&]() {
        /*
                        1             1                 (1,1)      ---- x0
                       / \           / \                /   \
                       | _2_        2   \              /     \
                       \/   \      / \  |     ==>      |     |
                        3   4      3 T  4            (3,3) (3,4)
                       / \ / \    / \  / \            / \   / \
                       T T F T    F T  T T            F T   T T

                       where (3) and (4) are swapped in order on the left one.

                       (3,3) : ((2,1), (2,0))   ,   (3,4) : ((2,1), (2,1))

                       so (3,3) is forwarded while (3,4) is not and hence (3,3) is output first.
        */
        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(2,MAX_ID, terminal_T, terminal_T)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID-1))
               << create_node(0,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))
            ;
        }

        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_T, terminal_T)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID-1), terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));
        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));


        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), terminal_T }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("computes { {0}, {1} } ∩ { {0,1} }", [&]() {
        /*
                    1          1                  (1,1)         ---- x0
                   / \        / \                 /   \
                   2 T        F 2                 F (T,2)       ---- x1
                  / \          / \       ==>         / \
                  F T          F T                   F F
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, terminal_F, create_node_ptr(1,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and skip) { {0}, {1}, {2}, {1,2}, {0,2} } ∩ { {0}, {2}, {0,2}, {0,1,2} }", [&]() {
        /*
                      1          1                 (1,1)         ---- x0
                     / \        / \                /   \
                     2 |        | 2               /     \        ---- x1
                    / \|        |/ \       ==>    |     |
                    3  4        3  4            (3,3) (4,3)      ---- x2
                   / \/ \      / \/ \            / \   / \
                   F T  T      F T  T            F T   F T

                   Notice, how (2,3) and (4,2) was skipped on the low and high of (1,1)
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(2,MAX_ID,   terminal_T, terminal_T)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(1,MAX_ID,   create_node_ptr(2,MAX_ID-1), create_node_ptr(2,MAX_ID))
               << create_node(0,MAX_ID,   create_node_ptr(1,MAX_ID),   create_node_ptr(2,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID,   terminal_T, terminal_T)
               << create_node(2,MAX_ID-1, terminal_F, terminal_T)
               << create_node(1,MAX_ID,   create_node_ptr(2,MAX_ID-1), create_node_ptr(2,MAX_ID))
               << create_node(0,MAX_ID,   create_node_ptr(2,MAX_ID-1), create_node_ptr(1,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,1), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("computes (and skip) { {0}, {1} } ∩ { {1}, {0,2} }", [&]() {
        /*
                     1         1                (1,1)      ---- x0
                    / \       / \               /   \
                    2 T       2  \            (2,2) F      ---- x1
                   / \       / \ |     =>      / \
                   F T       F T 3             F T         ---- x2
                                / \
                                F T
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_F, terminal_T)
               << create_node(1,MAX_ID, terminal_F, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));


        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes (and skip) { {0,2}, {1,2}, Ø } ∩ { {0,1}, {0}, {1} }", [&]() {
        /*
                     1         1                    (1,1)    ---- x0
                    / \       / \                   /   \
                    2  \      2 T                 (2,2) |    ---- x1
                   / \ /     / \       =>         /   \ |
                   T  3      T T                  T   F F    ---- x2
                     / \
                     F T

                   This shortcuts the (3,T) tuple twice.
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(2,MAX_ID, terminal_F, terminal_T)
               << create_node(1,MAX_ID, terminal_T, create_node_ptr(2,MAX_ID))
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(2,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(1,MAX_ID, terminal_T, terminal_T)
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), terminal_T)
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));


        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_T }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes (and shortcut) { {0,2}, {1,2}, Ø } ∩ { {0,2}, {0} }", [&]() {
        /*
                     1            1               (1,1)     ---- x0
                    / \          / \               / \
                    2  \         F |               F |      ---- x1
                   / \ /           |     ==>         |
                   T  3            2               (2,3)    ---- x2
                     / \          / \               / \
                     F T          T T               F T

                   This shortcuts the (3,T) tuple twice.
        */

        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(2,MAX_ID, terminal_F, terminal_T)
               << create_node(1,MAX_ID, terminal_T, create_node_ptr(2,MAX_ID))
               << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(2,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(2,MAX_ID, terminal_T, terminal_T)
               << create_node(0,MAX_ID, terminal_F, create_node_ptr(2,MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("zdd_diff", [&]() {
      it("should shortcut to Ø on same file for { {x0} }", [&]() {
        __zdd out = zdd_diff(zdd_x0, zdd_x0);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should shortcut to Ø on same file for { {x1} }", [&]() {
        __zdd out = zdd_diff(zdd_x1, zdd_x1);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø } \\ Ø", [&]() {
        __zdd out = zdd_diff(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes Ø \\ { Ø }", [&]() {
        __zdd out = zdd_diff(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should shortcut on irrelevance on { {x0} } \\ Ø", [&]() {
        __zdd out_1 = zdd_diff(zdd_x0, zdd_F);
        AssertThat(out_1.get<node_file>()._file_ptr, Is().EqualTo(zdd_x0._file_ptr));
      });

      it("should shortcut on irrelevance on { {x1} } \\ Ø", [&]() {
        __zdd out_2 = zdd_diff(zdd_x1, zdd_F);
        AssertThat(out_2.get<node_file>()._file_ptr, Is().EqualTo(zdd_x1._file_ptr));
      });

      it("computes (and shortcut) Ø  \\ { {0} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { {Ø} } \\ { {0} }", [&]() {
        __zdd out = zdd_diff(zdd_T, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<node_file>().meta_size(), Is().EqualTo(0u));

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0} } \\ { Ø }", [&]() {
        /*
                   1      T            1       ---- x0
                  / \          ==>    / \
                  F T                 F T
        */
        __zdd out = zdd_diff(zdd_x0, zdd_T);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0}, Ø } \\ { Ø }", [&]() {
        /*
                   1      T            1       ---- x0
                  / \          ==>    / \
                  T T                 F T
        */
        node_file zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << create_node(0,MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_T);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0,1}, {1} } \\ { {1}, Ø }", [&]() {
        /*
                     1                      (1,1)       ---- x0
                     ||                     /   \
                     2      1     ==>    (2,1) (2,F)    ---- x1
                    / \    / \            / \   / \
                    F T    T T            F F   F T
        */
        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a  << create_node(1,MAX_ID, terminal_F, terminal_T)
                << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID), create_node_ptr(1,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(1,MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0,1}, {1,2}, {1} } \\ { {1}, Ø }", [&]() {
        /*
                     _1_                        (1,1)       ---- x0
                    /   \                       /   \
                    3   2      1     ==>    (3,1)   (2,F)    ---- x1
                   / \ / \    / \           /   \    / \
                   F 4 F T    T T           F (3,T)  F T    ---- x2
                    / \                        / \
                    T T                        F T
        */
        node_file zdd_a;
        node_file zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a  << create_node(2,MAX_ID, terminal_T, terminal_T)
                << create_node(1,MAX_ID, terminal_F, terminal_T)
                << create_node(1,MAX_ID-1, terminal_F, create_node_ptr(2,MAX_ID))
                << create_node(0,MAX_ID, create_node_ptr(1,MAX_ID-1), create_node_ptr(1,MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << create_node(1,MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_b);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { create_node_ptr(0,0), create_node_ptr(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(0,0)), create_node_ptr(1,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,0)), create_node_ptr(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arc_test_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(1,1), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(1,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { create_node_ptr(2,0), terminal_F }));
        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(create_node_ptr(2,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });
    });
  });
 });
