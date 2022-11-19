#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/complement.cpp", []() {
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << node(false);

      node_writer nw_T(zdd_T);
      nw_T << node(true);
    }

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    node_file zdd_pow_24;
    // { Ø, { 2 }, { 4 }, { 2,4 } }
    /*
            1      ---- x2
            / \
            | |
            \ /
            2      ---- x4
            / \
            T T
    */
    { // Garbage collect writer early
      const node n2 = node(4, node::MAX_ID,   terminal_T, terminal_T);
      const node n1 = node(2, node::MAX_ID,   n2.uid(), n2.uid());

      node_writer nw(zdd_pow_24);
      nw << n2 << n1;
    }

    label_file dom_0123;
    {
      label_writer w(dom_0123);
      w << 0 << 1 << 2 << 3;
    }

    label_file dom_1234;
    {
      label_writer w(dom_1234);
      w << 1 << 2 << 3 << 4;
    }

    describe("zdd_complement(const zdd&, const label_file&)", [&]() {
      it("produces Ø on Ø and U = Ø", [&]() {
        label_file dom_empty;
        __zdd out = zdd_complement(zdd_F, dom_empty);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("produces { Ø } on { Ø } and U = Ø", [&]() {
        label_file dom_empty;
        __zdd out = zdd_complement(zdd_F, dom_empty);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(zdd_F._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("produces pow(U) on F terminal and U = { 0, 1, 2, 3 }", [&]() {
        __zdd out = zdd_complement(zdd_F, dom_0123);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                      terminal_T,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      ptr_uint64(3, ptr_uint64::MAX_ID),
                                                      ptr_uint64(3, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                      ptr_uint64(1, ptr_uint64::MAX_ID),
                                                      ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

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

      it("produces pow(U) on F terminal and U = { 1, 2, 3, 4 }", [&]() {
        __zdd out = zdd_complement(zdd_F, dom_1234);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                      terminal_T,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                      ptr_uint64(4, ptr_uint64::MAX_ID),
                                                      ptr_uint64(4, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      ptr_uint64(3, ptr_uint64::MAX_ID),
                                                      ptr_uint64(3, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("produces pow(U) \\ Ø on T terminal and U = { 0, 2 }", [&]() {
        label_file U02;
        {
          label_writer w(U02);
          w << 0 << 2;
        }


        __zdd out = zdd_complement(zdd_T, U02);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      terminal_F,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

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

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("produces pow(U) \\ Ø on T terminal and U = { 1, 2, 3, 4 }", [&]() {
        __zdd out = zdd_complement(zdd_T, dom_1234);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                      terminal_F,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                      ptr_uint64(4, ptr_uint64::MAX_ID),
                                                      ptr_uint64(4, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      ptr_uint64(3, ptr_uint64::MAX_ID),
                                                      ptr_uint64(3, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("adds out-of-set chain above root on { { 3 } } and U = { 0, 1, 2, 3 }", [&]() {
        node_file zdd_x3;
        {
          node_writer nw(zdd_x3);
          /*
                1        ---- x3
               / \
               F T
          */
          nw << node(3, node::MAX_ID, terminal_F, terminal_T);
        }

        __zdd out = zdd_complement(zdd_x3, dom_0123);
        /*
                     *    ---- x0
                    / \
                   *   *  ---- x1
                  / \ //
                  *   *   ---- x2
                 / \ //
                1   *     ---- x3
               / \  ||
               T F  T
         */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // root chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // root chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // T chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // root chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // T chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // input node (flipped)
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // T chain
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("adds out-of-set chain above and below root on { { 2 } } and U = { 0, 1, 2, 3 }", [&]() {
        node_file zdd_x2;
        {
          node_writer nw(zdd_x2);
          nw << node(2, node::MAX_ID, terminal_F, terminal_T);
        }

        __zdd out = zdd_complement(zdd_x2, dom_0123);

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // root chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // root chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // T chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // original node
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // T chain
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // F chain
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // T chain
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("adds out-of-set chain above and below root on { Ø, { 1 } } and U = { 1, 2, 3, 4 }", [&]() {
        node_file zdd_x1_null;
        /*
                1        ---- x1
              / \
              T T
        */
        {
          node_writer nw(zdd_x1_null);
          nw << node(1, node::MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_complement(zdd_x1_null, dom_0123);
        /*
                1        ---- x0
                / \
                2 3       ---- x1
              || ||
                4_ 5      ---- x2
              |  \||
                6_ 7      ---- x3
              |  \||
              F   T
        */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // 2
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 3
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 4
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 5
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 6
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 7
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));


        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 6
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 7
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("computes U \\ { { 2 }, { 3 }, { 4 }, { 2,4 }, { 3,4 } } with U = { 1, 2, 3, 4 }", [&]() {
        node_file in;
        // { { 2 }, { 3 }, { 4 }, { 2,4 }, { 3,4 } }
        /*
                1       ---- x2
              / \
              2 |      ---- x3
              / \|
              3  4      ---- x4
            / \/ \
            F T  T
        */
        { // Garbage collect writer early
          const node n4 = node(4, node::MAX_ID,   terminal_T, terminal_T);
          const node n3 = node(4, node::MAX_ID-1, terminal_F, terminal_T);
          const node n2 = node(3, node::MAX_ID,   n3.uid(), n4.uid());
          const node n1 = node(2, node::MAX_ID,   n2.uid(), n4.uid());

          node_writer nw(in);
          nw << n4 << n3 << n2 << n1;
        }

        __zdd out = zdd_complement(in, dom_1234);
        /*
                  _1_      ---- x1
                  /   \
                  2   3     ---- x2
                / \  ||
                4 5_ 6     ---- x3
                / \| \||
                7  8  9     ---- x4
              / \/ \ ||
              T F  F T
        */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // 2
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 3
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 4
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 5
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 6
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,2) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,2) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 7
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(4,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 8
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 9
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(4,2) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,2), ptr_uint64(4,2) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), ptr_uint64(4,2) }));

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 7
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 8
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,1), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,1)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 9
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,2), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,2)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,3u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,3u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(6u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("computes pow(U) \\ { Ø, { 1,2 }, { 2,3 }, { 2,4 }, { 1,2,3 }, { 1,2,4 } } with U = { 1, 2, 3, 4 }", [&]() {
        node_file in;
        /*
                _1_      ---- x1
              /   \
              2   3     ---- x2
              / \ / \
              T 4 F 5    ---- x3
              / \ / \
              6 T 7 T   ---- x4
              / \ / \
              F T T T
        */
        { // Garbage collect writer early
          const node n7 = node(4, node::MAX_ID,   terminal_T, terminal_T);
          const node n6 = node(4, node::MAX_ID-1, terminal_F, terminal_T);
          const node n5 = node(3, node::MAX_ID,   n7.uid(), terminal_T);
          const node n4 = node(3, node::MAX_ID-1, n6.uid(), terminal_T);
          const node n3 = node(2, node::MAX_ID,   terminal_F, n5.uid());
          const node n2 = node(2, node::MAX_ID-1, terminal_T, n4.uid()); // <-- breaks canonicity
          const node n1 = node(1, node::MAX_ID,   n2.uid(), n3.uid());

          node_writer nw(in);
          nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        __zdd out = zdd_complement(in, dom_1234);
        /*
                  _1_                ---- x1
                  /   \
                  2   3               ---- x2
                  X__ X_______
                /   X____    \
                /   /     \    \
                4   5     6    7      ---- x3
              / \ / \___ |\__ ||
              /   X______\\   \||
            /   /       \\\  |||
            8  9         10   11     ---- x4
            / \/ \       /  \  ||
            T F  F       F  T  T
        */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // 2
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 3
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 4
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 5
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 6
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,2) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 7
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,3) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 8
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(4,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 9
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 10
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,2) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(4,2) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,2), ptr_uint64(4,2) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 11
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), ptr_uint64(4,3) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,3), ptr_uint64(4,3) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,3)), ptr_uint64(4,3) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 8
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 9
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,1), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,1)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 10
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,2), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,2)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 11
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,3), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,3)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,4u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,4u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(8u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(4u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(4u));
      });

    it("computes pow(U) \\ pow(U) with U = { 2, 4 }", [&]() {

        label_file U;
        {
          label_writer w(U);
          w << 2 << 4 ;
        }

        __zdd out = zdd_complement(zdd_pow_24, U);
        /*
            1      ---- x2
            / \
            | |
            \ /
            2      ---- x4
            / \
            F F
        */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // 2
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(4,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(4,0) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 2
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(0u));
      });

    it("computes pow(U) \\ pow({ 2, 4 }) with U = { 1, 2, 3, 4 }", [&]() {
        __zdd out = zdd_complement(zdd_pow_24, dom_1234);
        /*
              _1_       ---- x1
            /   \
            2   3      ---- x2
            ||  ||
            4   5      ---- x3
            / \__||
            6    7      ---- x4
          / \  / \
          F F  T T
        */

        node_arcest_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True()); // 2
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 3
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 4
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 5
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 6
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,0), ptr_uint64(4,0) }));

        AssertThat(node_arcs.can_pull(), Is().True()); // 7
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { ptr_uint64(3,1), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), ptr_uint64(4,1) }));

        AssertThat(node_arcs.can_pull(), Is().False());

        terminal_arcest_stream terminal_arcs(out);

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 6
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,0), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,0)), terminal_F }));

        AssertThat(terminal_arcs.can_pull(), Is().True()); // 7
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { ptr_uint64(4,1), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().True());
        AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(ptr_uint64(4,1)), terminal_T }));

        AssertThat(terminal_arcs.can_pull(), Is().False());

        level_info_test_stream<arc> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,2u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
      });
    });

    describe("zdd_complement(const zdd&)", [&]() {
      it("produces pow(U) on F terminal with set domain U = { 0, 1, 2, 3 }", [&]() {
        adiar_set_domain(dom_0123);

        __zdd out = zdd_complement(zdd_F);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                      terminal_T,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      ptr_uint64(3, ptr_uint64::MAX_ID),
                                                      ptr_uint64(3, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                      ptr_uint64(1, ptr_uint64::MAX_ID),
                                                      ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

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

      it("produces pow(U) \\ Ø on T terminal with set domain U = { 1, 2, 3, 4 }", [&]() {
        adiar_set_domain(dom_1234);

        __zdd out = zdd_complement(zdd_T);

        nodeest_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                      terminal_F,
                                                      terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                      ptr_uint64(4, ptr_uint64::MAX_ID),
                                                      ptr_uint64(4, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                      ptr_uint64(3, ptr_uint64::MAX_ID),
                                                      ptr_uint64(3, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                      ptr_uint64(2, ptr_uint64::MAX_ID),
                                                      ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node> level_info(out);

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(level_info.can_pull(), Is().True());
        AssertThat(level_info.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(level_info.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
      });
    });
  });
});
