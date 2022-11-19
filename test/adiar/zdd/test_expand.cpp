#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/expand.cpp", [&]() {
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

    node_file zdd_x1;
    /*
             1              ---- x1
            / \
            F T
    */

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_x1);
      nw << node(1, node::MAX_ID, terminal_F, terminal_T);
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

    const node n1_7 = node(10, zdd::MAX_ID,   terminal_F,   terminal_T);
    const node n1_6 = node(8,  zdd::MAX_ID,   terminal_T,   terminal_T);
    const node n1_5 = node(8,  zdd::MAX_ID-1, n1_7.uid(), terminal_T);
    const node n1_4 = node(6,  zdd::MAX_ID,   n1_5.uid(), n1_6.uid());
    const node n1_3 = node(4,  zdd::MAX_ID,   n1_4.uid(), terminal_T);
    const node n1_2 = node(4,  zdd::MAX_ID-1, terminal_F,   n1_4.uid());
    const node n1_1 = node(2,  zdd::MAX_ID,   n1_2.uid(), n1_3.uid());

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

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_T, terminal_T)));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(42,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("returns don't care chain for { Ø } on (0,2)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_expand(zdd_T, labels);

      nodeest_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, terminal_T, terminal_T)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());

      level_info_test_stream<node> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("adds new root for { { 1 } } on (0)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(1, node::MAX_ID, terminal_F, terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

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

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds node chain for { { 3 }, { 3,4 } } on (0,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(4, node::MAX_ID, terminal_F, terminal_T)
          << node(3, node::MAX_ID, terminal_T, ptr_uint64(4, ptr_uint64::MAX_ID))
          ;
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("adds nodes before and after for { Ø, { 3 } } on (0,2,4)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(3, node::MAX_ID, terminal_T, terminal_T);
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 2 << 4;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("adds nodes for T terminal but not F terminal for { { 1,2 }, { 1,3 } } on (4,5,6)", [&]() {
      // Alternative title: Only creates one terminal-chain.

      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(3, node::MAX_ID, terminal_F, terminal_T)
          << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), terminal_T)
          << node(1, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 4 << 5 << 6;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), ptr_uint64(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), ptr_uint64(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(5,0), ptr_uint64(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(5,0)), ptr_uint64(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(6,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(6,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

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

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("adds node in between levels { { 0,2 } } on (1)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

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
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds nodes in between levels { { 0,3 } } on (1,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(3, node::MAX_ID, terminal_F, terminal_T)
          << node(0, node::MAX_ID, terminal_F, ptr_uint64(3, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1 << 2;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

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
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("adds different don't care nodes on different arcs cut for { {0}, {2}, { 0,2 } } on (1,2)", [&]() {
      node_file in;

      { // Garbage collect writer to free write-lock
        node_writer w(in);
        w << node(2, node::MAX_ID,   terminal_T, terminal_T)
          << node(2, node::MAX_ID-1, terminal_F, terminal_T)
          << node(0, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID));
      }

      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 1;
      }

      __zdd out = zdd_expand(in, labels);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(level_info.can_pull(), Is().True());
      AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(level_info.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(4u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
    });

    it("adds don't care nodes before, in between, and after for [1] on (0,1,3,5,7,9,11)", [&]() {
      label_file labels;

      { // Garbage collect writer to free write-lock
        label_writer w(labels);
        w << 0 << 1 << 3 << 5 << 7 << 9 << 11;
      }

      __zdd out = zdd_expand(zdd_1, labels);

      node_arcest_stream node_arcs(out);

      // Pre-chain
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

      // (1) and its x3 cut
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

      // (2) and (3) and their x5 cut
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(4,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(4,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), ptr_uint64(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,1), ptr_uint64(5,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,1)), ptr_uint64(5,1) }));

      // (4) and the x7 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(5,0), ptr_uint64(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(5,0)), ptr_uint64(6,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(6,0), ptr_uint64(7,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(6,0)), ptr_uint64(7,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(5,1), ptr_uint64(7,2) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(5,1)), ptr_uint64(7,2) }));

      // (5) and (6) and their x9 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(7,0), ptr_uint64(8,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(7,0)), ptr_uint64(8,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(7,1), ptr_uint64(8,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(7,1)), ptr_uint64(8,1) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(8,0), ptr_uint64(9,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(7,2), ptr_uint64(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(7,2)), ptr_uint64(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(8,0)), ptr_uint64(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(8,1), ptr_uint64(9,1) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(8,1)), ptr_uint64(9,1) }));

      // (7) and the x11 cuts
      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(9,0), ptr_uint64(10,0) }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(9,0)), ptr_uint64(10,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(9,1), ptr_uint64(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(9,1)), ptr_uint64(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().True()); // T chain
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(10,0)), ptr_uint64(11,0) }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(10,0), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(11,0), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(11,0)), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> level_info(out);

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

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(6u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });
  });
 });
