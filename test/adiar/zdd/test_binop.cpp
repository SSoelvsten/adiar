#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/binop.cpp", []() {
    shared_levelized_file<zdd::node_t> zdd_F;
    shared_levelized_file<zdd::node_t> zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << node(false);

      node_writer nw_T(zdd_T);
      nw_T << node(true);
    }

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<zdd::node_t> zdd_x0;
    shared_levelized_file<zdd::node_t> zdd_x1;

    { // Garbage collect writers early
      node_writer nw_x0(zdd_x0);
      nw_x0 << node(0, node::MAX_ID, terminal_F, terminal_T);

      node_writer nw_x1(zdd_x1);
      nw_x1 << node(1, node::MAX_ID, terminal_F, terminal_T);
    }

    describe("zdd_union", [&]() {
      it("should shortcut Ø on same file", [&]() {
        __zdd out = zdd_union(zdd_F, zdd_F);
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_F));
      });

      it("should shortcut { Ø } on same file", [&]() {
        __zdd out = zdd_union(zdd_T, zdd_T);
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_T));
      });

      it("should shortcut { {0} } on same file", [&]() {
        __zdd out = zdd_union(zdd_x0, zdd_x0);
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));
      });

      it("should shortcut { {1} } on same file", [&]() {
        __zdd out = zdd_union(zdd_x1, zdd_x1);
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x1));
      });

      it("computes Ø U { {Ø} }", [&]() {
        __zdd out = zdd_union(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { Ø } U Ø", [&]() {
        __zdd out = zdd_union(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should shortcut on irrelevance for { {0} } U Ø", [&]() {
        /*
                   1     ---- x0
                  / \
                  F T
        */

        __zdd out_1 = zdd_union(zdd_x0, zdd_F);
        AssertThat(out_1.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));

        __zdd out_2 = zdd_union(zdd_F, zdd_x0);
        AssertThat(out_2.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));
      });

      it("computes { {0} } U { Ø }", [&]() {
        /*
                   1     ---- x0
                  / \
                  T T
        */

        __zdd out = zdd_union(zdd_x0, zdd_T);

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
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

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
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
        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(3, node::MAX_ID, terminal_F, terminal_T)
               << node(1, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), terminal_T)
               << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(4u));
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
        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
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
        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(3, node::MAX_ID, terminal_F, terminal_T)
               << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID))
               << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID))
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(3, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_union(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(3,2) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(4u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(4u));
      });
    });

    describe("zdd_intsec", [&]() {
      it("should shortcut on same file", [&]() {
        __zdd out_1 = zdd_intsec(zdd_x0, zdd_x0);
        AssertThat(out_1.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));

        __zdd out_2 = zdd_intsec(zdd_x1, zdd_x1);
        AssertThat(out_2.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x1));
      });

      it("computes Ø ∩ { {Ø} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø } ∩ Ø", [&]() {
        __zdd out = zdd_intsec(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
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
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and shortcut) Ø ∩ { {0} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
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
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø, {0} } ∩ { Ø }", [&]() {
        /*
                   1       T              T       ---- x0
                  / \           ==>
                  T T
        */
        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(0, node::MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0}, {1} } ∩ { Ø }", [&]() {
        /*
                       1        T             F          ---- x0
                      / \
                      2 T             ==>                ---- x1
                     / \
                     F T
        */

        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes (and shortcut) { {0,1}, {1} } ∩ { {0,1} }", [&]() {
        /*
                    _1_        1            1        ---- x0
                   /   \      / \          / \
                   2   3      F 2    ==>   F 2       ---- x1
                  / \ / \      / \          / \
                  T T F T      F T          F T
        */

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(1, node::MAX_ID-1, terminal_T, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
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

        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
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

        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(2, node::MAX_ID, terminal_T, terminal_T)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_T, terminal_F)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID-1))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
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
        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(2, node::MAX_ID, terminal_T, terminal_T)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID-1))
               << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;
        }

        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_T, terminal_T)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID-1), terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));


        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("computes { {0}, {1} } ∩ { {0,1} }", [&]() {
        /*
                    1          1                  (1,1)         ---- x0
                   / \        / \                 /   \
                   2 T        F 2                 F (T,2)       ---- x1
                  / \          / \       ==>         / \
                  F T          F T                   F F
        */

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, terminal_F, ptr_uint64(1, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(0u));
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

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(2, node::MAX_ID,   terminal_T, terminal_T)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
               << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(2, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID,   terminal_T, terminal_T)
               << node(2, node::MAX_ID-1, terminal_F, terminal_T)
               << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
               << node(0, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
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

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_F, terminal_T)
               << node(1, node::MAX_ID, terminal_F, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));


        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
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

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(2, node::MAX_ID, terminal_F, terminal_T)
               << node(1, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID))
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(1, node::MAX_ID, terminal_T, terminal_T)
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));


        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
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

        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(2, node::MAX_ID, terminal_F, terminal_T)
               << node(1, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID))
               << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(2, node::MAX_ID, terminal_T, terminal_T)
               << node(0, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))
            ;
        }

        __zdd out = zdd_intsec(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("zdd_diff", [&]() {
      it("should shortcut to Ø on same file for { {x0} }", [&]() {
        __zdd out = zdd_diff(zdd_x0, zdd_x0);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should shortcut to Ø on same file for { {x1} }", [&]() {
        __zdd out = zdd_diff(zdd_x1, zdd_x1);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { Ø } \\ Ø", [&]() {
        __zdd out = zdd_diff(zdd_T, zdd_F);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes Ø \\ { Ø }", [&]() {
        __zdd out = zdd_diff(zdd_F, zdd_T);

        node_test_stream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("should shortcut on irrelevance on { {x0} } \\ Ø", [&]() {
        __zdd out_1 = zdd_diff(zdd_x0, zdd_F);
        AssertThat(out_1.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));
      });

      it("should shortcut on irrelevance on { {x1} } \\ Ø", [&]() {
        __zdd out_2 = zdd_diff(zdd_x1, zdd_F);
        AssertThat(out_2.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x1));
      });

      it("computes (and shortcut) Ø  \\ { {0} }", [&]() {
        __zdd out = zdd_intsec(zdd_F, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("computes { {Ø} } \\ { {0} }", [&]() {
        __zdd out = zdd_diff(zdd_T, zdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0} } \\ { Ø }", [&]() {
        /*
                   1      T            1       ---- x0
                  / \          ==>    / \
                  F T                 F T
        */
        __zdd out = zdd_diff(zdd_x0, zdd_T);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0}, Ø } \\ { Ø }", [&]() {
        /*
                   1      T            1       ---- x0
                  / \          ==>    / \
                  T T                 F T
        */
        shared_levelized_file<zdd::node_t> zdd_a;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a << node(0, node::MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_T);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("computes { {0,1}, {1} } \\ { {1}, Ø }", [&]() {
        /*
                     1                      (1,1)       ---- x0
                     ||                     /   \
                     2      1     ==>    (2,1) (2,F)    ---- x1
                    / \    / \            / \   / \
                    F T    T T            F F   F T
        */
        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a  << node(1, node::MAX_ID, terminal_F, terminal_T)
                << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(1, node::MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
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
        shared_levelized_file<zdd::node_t> zdd_a;
        shared_levelized_file<zdd::node_t> zdd_b;

        { // Garbage collect writers early
          node_writer nw_a(zdd_a);
          nw_a  << node(2, node::MAX_ID, terminal_T, terminal_T)
                << node(1, node::MAX_ID, terminal_F, terminal_T)
                << node(1, node::MAX_ID-1, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))
                << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID))
            ;

          node_writer nw_b(zdd_b);
          nw_b << node(1, node::MAX_ID, terminal_T, terminal_T);
        }

        __zdd out = zdd_diff(zdd_a, zdd_b);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });
    });
  });
 });
