#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/binop.cpp", []() {
    // Setup shared zdd's
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

    describe("simple cases without access mode requirements", [&]() {
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should shortcut on irrelevance for { {0} } U Ø", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F T
          */

          __zdd out_1 = zdd_union(zdd_x0, zdd_F);
          AssertThat(out_1.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));

          __zdd out_2 = zdd_union(zdd_F, zdd_x0);
          AssertThat(out_2.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_x0));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes (and shortcut) { {0} } ∩ Ø", [&]() {
          /*
          //           1       F              F          ---- x0
          //          / \           ==>
          //          F T
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });
      });
    });

    describe("access mode: priority queues", [&]() {
      // Set access mode to priority queues for this batch of tests
      access_mode = access_mode_t::PQ;

      describe("zdd_union", [&]() {
        it("computes { {0} } U { Ø }", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          T T
          */

          __zdd out = zdd_union(zdd_x0, zdd_T);

          arc_test_stream arcs(out);
          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes { {0} } U { {1} }", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(zdd_x0, zdd_x1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes { {0,1}, {0,3} } U { {0,2}, {2} }", [&]() {
          /*
          //            1           1                (1,1)         ---- x0
          //           / \         / \                 X
          //           F 2         | |            (2,2) \          ---- x1
          //            / \        \ /   ==>      /   \  \
          //            | T         2          (3,2)  T (F,2)      ---- x2
          //            |          / \         /   \     / \
          //            3          F T       (3,F) T     F T       ---- x3
          //           / \                    / \
          //           F T                    F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("computes { {0,1}, {1} } U { {0,2}, {2} }", [&]() {
          /*
          //           1           1             (1,1)
          //           ||         / \             | |
          //           2          | |            (2,2)
          //          / \         \ /   ==>       / \
          //          F T          2          (F,2) (T,F) <-- since 2nd (2) skipped level
          //                      / \          / \
          //                      F T          F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes { {0}, {1,3}, {2,3}, {1} } U { {0,3}, {3} }", [&]() {
          /*
          //                1        1                        (1,1)         ---- x0
          //               / \      / \                       /   \
          //               2 T      | |                    (2,2)   \        ---- x1
          //              / \       | |                    /   \    \
          //              3  \      | |     ==>         (3,2)  |     \      ---- x2
          //              \\ /      \ /                 /   \  |     |
          //                4        2              (4,2)   (4,F)  (T,2)    ---- x3
          //               / \      / \              / \     / \    / \
          //               F T      F T              F T     T T    T T
          //
          //           The high arc on (2) and (3) on the left is shortcutting the
          //           second ZDD, to compensate for the omitted nodes.
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true,  terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(4u));
        });
      });

      describe("zdd_intsec", [&]() {
        it("computes { {0} } ∩ { Ø }", [&]() {
          /*
          //           1       T              F       ---- x0
          //          / \           ==>
          //          F T
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes { Ø, {0} } ∩ { Ø }", [&]() {
          /*
          //           1       T              T       ---- x0
          //          / \           ==>
          //          T T
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes { {0}, {1} } ∩ { Ø }", [&]() {
          /*
          //               1        T             F          ---- x0
          //              / \
          //              2 T             ==>                ---- x1
          //             / \
          //             F T
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes (and shortcut) { {0,1}, {1} } ∩ { {0,1} }", [&]() {
          /*
          //            _1_        1            1        ---- x0
          //           /   \      / \          / \
          //           2   3      F 2    ==>   F 2       ---- x1
          //          / \ / \      / \          / \
          //          T T F T      F T          F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes (and skip to terminal) { {0}, {1}, {0,1} } ∩ { Ø }", [&]() {
          /*
          //            1        T          F     ---- x0
          //           / \
          //           \ /            ==>
          //            2                         ---- x1
          //           / \
          //           F T
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes (and skip to terminal) { {0,2}, {0}, {2} } \\ { {1}, {2}, Ø }", [&]() {
          /*
          //                1                             F         ---- x0
          //               / \
          //              /   \         _1_                         ---- x1
          //              |   |        /   \       =>
          //              2   3        2   3                        ---- x2
          //             / \ / \      / \ / \
          //             F T T T      T F F T
          //
          //             Where (2) and (3) are swapped in order on the right. Notice,
          //             that (2) on the right technically is illegal, but it makes
          //             for a much simpler counter-example that catches
          //             prod_pq_1.peek() throwing an error on being empty.
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes (and skips in) { {0,1,2}, {0,2}, {0}, {2} } } ∩ { {0,2}, {0}, {1}, {2} }", [&]() {
          /*
          //                1             1                 (1,1)      ---- x0
          //               / \           / \                /   \
          //               | _2_        2   \              /     \
          //               \/   \      / \  |     ==>      |     |
          //                3   4      3 T  4            (3,3) (3,4)
          //               / \ / \    / \  / \            / \   / \
          //               T T F T    F T  T T            F T   T T
          //
          //               where (3) and (4) are swapped in order on the left one.
          //
          //               (3,3) : ((2,1), (2,0))   ,   (3,4) : ((2,1), (2,1))
          //
          //               so (3,3) is forwarded while (3,4) is not and hence (3,3) is output first.
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));


          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("computes (and skip) { {0}, {1} } ∩ { {0,1} }", [&]() {
          /*
          //            1          1                  (1,1)         ---- x0
          //           / \        / \                 /   \
          //           2 T        F 2                 F   F         ---- x1
          //          / \          / \       ==>
          //          F T          F T
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
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("computes (and skip) { {0}, {1}, {2}, {1,2}, {0,2} } ∩ { {0}, {2}, {0,2}, {0,1,2} }", [&]() {
          /*
          //              1          1                 (1,1)         ---- x0
          //             / \        / \                /   \
          //             2 |        | 2               /     \        ---- x1
          //            / \|        |/ \       ==>    |     |
          //            3  4        3  4            (3,3) (4,3)      ---- x2
          //           / \/ \      / \/ \            / \   / \
          //           F T  T      F T  T            F T   F T
          //
          //           Notice, how (2,3) and (4,2) was skipped on the low and high of (1,1)
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes (and skip) { {0}, {1} } ∩ { {1}, {0,2} }", [&]() {
          /*
          //             1         1                (1,1)      ---- x0
          //            / \       / \               /   \
          //            2 T       2  \            (2,2) F      ---- x1
          //           / \       / \ |     =>      / \
          //           F T       F T 3             F T         ---- x2
          //                        / \
          //                        F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));


          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes (and skip) { {0,2}, {1,2}, Ø } ∩ { {0,1}, {0}, {1} }", [&]() {
          /*
          //             1         1                    (1,1)    ---- x0
          //            / \       / \                   /   \
          //            2  \      2 T                 (2,2) |    ---- x1
          //           / \ /     / \       =>         /   \ |
          //           T  3      T T                  T   F F    ---- x2
          //             / \
          //             F T
          //
          //           This shortcuts the (3,T) tuple twice.
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));


          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes (and shortcut) { {0,2}, {1,2}, Ø } ∩ { {0,2}, {0} }", [&]() {
          /*
          //             1            1               (1,1)     ---- x0
          //            / \          / \               / \
          //            2  \         F |               F |      ---- x1
          //           / \ /           |     ==>         |
          //           T  3            2               (2,3)    ---- x2
          //             / \          / \               / \
          //             F T          T T               F T
          //
          //           This shortcuts the (3,T) tuple twice.
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("zdd_diff", [&]() {
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

          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes { {0} } \\ { Ø }", [&]() {
          /*
          //           1      T            1       ---- x0
          //          / \          ==>    / \
          //          F T                 F T
          */
          __zdd out = zdd_diff(zdd_x0, zdd_T);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes { {0}, Ø } \\ { Ø }", [&]() {
          /*
          //           1      T            1       ---- x0
          //          / \          ==>    / \
          //          T T                 F T
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
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes (and skip) { {0,1}, {1} } \\ { {1}, Ø }", [&]() {
          /*
          //             1                      (1,1)       ---- x0
          //             ||                     /   \
          //             2      1     ==>    (2,1)  F       ---- x1
          //            / \    / \            / \
          //            F T    T T            F F
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes { {0,1}, {1,2}, {1} } \\ { {1}, Ø }", [&]() {
          /*
          //             _1_                        (1,1)       ---- x0
          //            /   \                       /   \
          //            3   2      1     ==>    (3,1)   (2,F)    ---- x1
          //           / \ / \    / \           /   \    / \
          //           F 4 F T    T T           F (3,T)  F T    ---- x2
          //            / \                        / \
          //            T T                        F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });
      });

      // Reset access mode
      access_mode = access_mode_t::AUTO;
    });
  
    describe("access mode: random access", [&]() {
      // Set access mode to random access for this batch of tests
      access_mode = access_mode_t::RA;

      describe("zdd_union", [&]() {
        it("computes { {0} } U { Ø, {0} } (same level)", [&]() {
            shared_levelized_file<zdd::node_t> zdd_maybe_x0;
            /*
            //      { Ø, {0} }
            //
            //          1          ---- x0
            //          ||
            //          T
            */

            { // Garbage collect early and free write-lock
              node_writer nw_m(zdd_maybe_x0);
              nw_m << node(0, node::MAX_ID, terminal_T, terminal_T);
            }

            /*
            // Result of { {0} } U { Ø, {0} }
            // 
            //                      (1,1)                   ---- x0
            //                      /   \
            //                  (F,T)   (T,T)
            */

            __zdd out = zdd_union(zdd_x0, zdd_maybe_x0);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(0u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(0u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes { {0} } U { {1} } (different levels, random access for first level)", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(zdd_x0, zdd_x1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });
        
        it("computes { {1} } U { {0} } (different levels, random access for second level)", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(zdd_x1, zdd_x0);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        shared_levelized_file<zdd::node_t> zdd_thin;
        /*
        //    { {2}, {0, 1}, {0, 2}, {0, 1, 2} }
        //
        //                  1          ---- x0
        //                 / \
        //                2   3        ---- x1
        //                X_ / \
        //               /  X   |
        //               |_/ \__|
        //               4      5      ---- x2
        //              / \    / \
        //              F T    T T
        */

        node nt_5 = node(2, node::MAX_ID, terminal_T, terminal_T);
        node nt_4 = node(2, node::MAX_ID - 1, terminal_F, terminal_T);
        node nt_3 = node(1, node::MAX_ID, nt_4, nt_5);
        node nt_2 = node(1, node::MAX_ID - 1, nt_5, nt_4);
        node nt_1 = node(0, node::MAX_ID, nt_2, nt_3);

        { // Garbage collect early and free write-lock
          node_writer nw_t(zdd_thin);
          nw_t << nt_5 << nt_4 << nt_3 << nt_2 << nt_1;
        }

        adiar_assert(zdd_thin->canonical == true, "Input validation failed");
        adiar_assert(zdd_thin->width == 2u, "Input validation failed");

        shared_levelized_file<zdd::node_t> zdd_wide;
        /*
        //  { {0}, {2}, {0, 3}, {0, 1, 2}, {1, 2, 3} }
        //
        //                1         ---- x0
        //               / \
        //              2   3       ---- x1
        //              X   X
        //             / \ / \
        //            4   5   6     ---- x2
        //           / \ / \ / \
        //           F 7 F T T T    ---- x3
        //            / \
        //            F T
        */
        
        node nw_7 = node(3, node::MAX_ID, terminal_F, terminal_T);
        node nw_6 = node(2, node::MAX_ID, terminal_T, terminal_T);
        node nw_5 = node(2, node::MAX_ID - 1, terminal_F, terminal_T);
        node nw_4 = node(2, node::MAX_ID - 2, terminal_F, nw_7);
        node nw_3 = node(1, node::MAX_ID, nw_6, nw_5);
        node nw_2 = node(1, node::MAX_ID - 1, nw_5, nw_4);
        node nw_1 = node(0, node::MAX_ID, nw_2, nw_3);

        { // Garbage collect early and free write-lock
          node_writer nw_w(zdd_wide);
          nw_w << nw_7 << nw_6 << nw_5 << nw_4 << nw_3 << nw_2 << nw_1;
        }

        adiar_assert(zdd_wide->canonical == true, "Input validation failed");
        adiar_assert(zdd_wide->width == 3u, "Input validation failed");
        
        /*
        // Result of [thin] U [wide]
        // 
        //                      (1,1)                   ---- x0
        //                      /   \
        //                  (2,2)   (3,3)               ---- x1
        //                    X       X
        //              _____/ \     / \_____
        //          (4,4)       (5,5)       (4,6)       ---- x2
        //          /   \       /   \       /   \
        //       (F,F) (T,7) (T,F) (T,T) (F,T) (T,T)    ---- x3
        //             /   \
        //          (T,F) (T,T)
        */

        it("should random access on the thinnest ([thin] U [wide])", [&]() {
            __zdd out = zdd_union(zdd_thin, zdd_wide);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(6u));
        });

        it("should random access on the thinnest ([wide] U [thin])", [&]() {
            __zdd out = zdd_union(zdd_thin, zdd_wide);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(6u));
        });

        shared_levelized_file<zdd::node_t> zdd_canon;
        /*
        //  { {0, 1}, {1, 2}, {2, 3}, {0, 1, 2}, {0, 2, 3} }
        //
        //                 1              ---- x0
        //                / \
        //               2   3            ---- x1
        //           ___/ \ / \___
        //          /    __X__    \
        //         4    5     6    7      ---- x2
        //        / \  / \   / \  / \
        //        F 8  F 9   F T  T T     ---- x3
        //         / \  / \
        //         F T  T T
        */
        
        node nc_9 = node(3, node::MAX_ID, terminal_T, terminal_T);
        node nc_8 = node(3, node::MAX_ID - 1, terminal_F, terminal_T);
        node nc_7 = node(2, node::MAX_ID, terminal_T, terminal_T);
        node nc_6 = node(2, node::MAX_ID - 1, terminal_F, terminal_T);
        node nc_5 = node(2, node::MAX_ID - 2, terminal_F, nc_9);
        node nc_4 = node(2, node::MAX_ID - 3, terminal_F, nc_8);
        node nc_3 = node(1, node::MAX_ID, nc_5, nc_7);
        node nc_2 = node(1, node::MAX_ID - 1, nc_4, nc_6);
        node nc_1 = node(0, node::MAX_ID, nc_2, nc_3);

        { // Garbage collect early and free write-lock
          node_writer nw_c(zdd_canon);
          nw_c << nc_9 << nc_8 << nc_7 << nc_6 << nc_5 << nc_4 << nc_3 << nc_2 << nc_1;
        }

        adiar_assert(zdd_canon->canonical == true, "Input validation failed");
        adiar_assert(zdd_canon->width == 4u, "Input validation failed");
        
        shared_levelized_file<zdd::node_t> zdd_non_canon;
        /*
        //  { {0}, {2}, {0, 2}, {1, 3}, {1, 2}, {0, 1, 2} }
        //
        //                1            ---- x0
        //               / \
        //              2   3          ---- x1
        //             / \ _X_
        //             |__X   \
        //             4   5   6       ---- x2
        //            / \ / \ / \
        //            F T 7 T T T      ---- x3
        //               / \
        //               F T
        */

        node nn_7 = node(3, node::MAX_ID, terminal_F, terminal_T);
        node nn_6 = node(2, node::MAX_ID, terminal_T, terminal_T);
        node nn_5 = node(2, node::MAX_ID - 1, nn_7, terminal_T);
        node nn_4 = node(2, node::MAX_ID - 2, terminal_F, terminal_T);
        node nn_3 = node(1, node::MAX_ID, nn_6, nn_4);
        node nn_2 = node(1, node::MAX_ID - 1, nn_4, nn_5);
        node nn_1 = node(0, node::MAX_ID, nn_2, nn_3);

        { // Garbage collect early and free write-lock
          node_writer nw_n(zdd_non_canon);
          nw_n << nn_7 << nn_6 << nn_5 << nn_4 << nn_3 << nn_2 << nn_1;
        }

        adiar_assert(zdd_non_canon->canonical == false, "Input validation failed");
        adiar_assert(zdd_non_canon->width == 3u, "Input validation failed");
        
        /*
        // Result of [canon] U [non_canon]
        //
        //                         (1,1)                              ---- x0
        //                   ______/   \______
        //                  /                 \
        //              (2,2)                 (3,3)                   ---- x1
        //              /   \________ __________X
        //            _/          ___X___        \___
        //           /           /       \           \ 
        //       (4,4)       (7,4)       (6,5)       (5,6)            ---- x2
        //       /   \       /   \       /   \       /   \
        //    (F,F) (8,T) (T,F) (T,T) (F,7) (T,T) (F,T) (9,T)         ---- x3
        //          /   \             /   \             /   \
        //      (F,T)   (T,T)     (F,F)   (F,T)     (T,T)   (T,T)
        */

        it("should random access on canonical ([canon] U [non_canon])", [&]() {
            __zdd out = zdd_union(zdd_canon, zdd_non_canon);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,3) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), false, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,3), true, ptr_uint64(3,2) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,4u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(9u));
        });

        it("should random access on canonical ([non_canon] U [canon])", [&]() {
            __zdd out = zdd_union(zdd_canon, zdd_non_canon);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,3) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), false, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,3), true, ptr_uint64(3,2) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,4u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(9u));
        });

        it("computes { {0}, {1,3}, {2,3}, {1} } U { {0,3}, {3} } in different order", [&]() {
            /*
            //                1        1                        (1,1)        ---- x0
            //               / \      / \                       /   \
            //               2 T      | |                    (2,2)   \       ---- x1
            //              / \       | |                    /   \    |
            //              3  \      | |     ==>         (3,2)   \   |      ---- x2
            //              || |      | |                 /   \____\ /  
            //              \\ /      \ /                /          X   
            //                4        2              (4,2)    (T,2) (4,F)   ---- x3
            //               / \      / \              / \      / \   / \
            //               F T      F T              F T      T T   T T
            //
            //           The high arc on (2) and (3) on the left is shortcutting the
            //           second ZDD, to compensate for the omitted nodes.
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

            adiar_assert(zdd_a->width == 1u, "Input validation failed");
            adiar_assert(zdd_a->canonical == true, "Input validation failed");

            adiar_assert(zdd_b->width == 1u, "Input validation failed");
            adiar_assert(zdd_b->canonical == true, "Input validation failed");

            __zdd out = zdd_union(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,1) }));
            
            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(3,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,2) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));
            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));
            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_F }));
            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true,  terminal_T }));

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

            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("should correctly generate missing nodes (bottom layers)", [&]() {
            shared_levelized_file<zdd::node_t> zdd_a;
            /*
            //      { {0}, {1} }
            //
            //           1             ---- x0
            //          / \
            //         2   T           ---- x1
            //        / \
            //       F   T
            */

            node na_2 = node(1, node::MAX_ID, terminal_F, terminal_T);
            node na_1 = node(0, node::MAX_ID, na_2, terminal_T);

            { // Garbage collect writers early
              node_writer nw_a(zdd_a);
              nw_a << na_2 << na_1;
            }

            adiar_assert(zdd_a->width == 1u, "Input validation failed");
            adiar_assert(zdd_a->canonical == true, "Input validation failed");

            shared_levelized_file<zdd::node_t> zdd_b;
            /*
            //    { {0}, {1}, {0, 1} }
            //
            //           1             ---- x0
            //          / \
            //         2   3           ---- x1
            //        / \ / \
            //       F   T   T
            */

            node nb_3 = node(1, node::MAX_ID, terminal_T, terminal_T);
            node nb_2 = node(1, node::MAX_ID - 1, terminal_F, terminal_T);
            node nb_1 = node(0, node::MAX_ID, nb_2, nb_3);

            { // Garbage collect writers early
              node_writer nw_b(zdd_b);
              nw_b << nb_3 << nb_2 << nb_1;
            }

            adiar_assert(zdd_b->width == 2u, "Input validation failed");
            adiar_assert(zdd_b->canonical == true, "Input validation failed");

            /*
            //    { {0}, {1} } U { {0}, {1}, {0, 1} }
            //
            //                 (1,1)              ---- x0
            //                 /   \
            //             (2,2)    (T,3)         ---- x1
            //             /   \    /   \
            //         (F,F)    (T,T)   (F,T)   
            */

            __zdd out = zdd_union(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("should correctly generate missing nodes (intermediate layers)", [&]() {
            shared_levelized_file<zdd::node_t> zdd_a;
            /*
            //      { {0}, {1}, {0, 3} }
            //
            //           1             ---- x0
            //          / \
            //         2   \           ---- x1
            //        / \   \
            //       F   T   3         ---- x2
            //              / \
            //             T   T
            */

            node na_3 = node(2, node::MAX_ID, terminal_T, terminal_T);
            node na_2 = node(1, node::MAX_ID, terminal_F, terminal_T);
            node na_1 = node(0, node::MAX_ID, na_2, na_3);

            { // Garbage collect writers early
              node_writer nw_a(zdd_a);
              nw_a << na_3 << na_2 << na_1;
            }

            adiar_assert(zdd_a->width == 1u, "Input validation failed");
            adiar_assert(zdd_a->canonical == true, "Input validation failed");

            shared_levelized_file<zdd::node_t> zdd_b;
            /*
            //    { {0}, {1}, {0, 1} }
            //
            //           1             ---- x0
            //          / \
            //         2   3           ---- x1
            //        / \ / \
            //       F   T   T
            */

            node nb_3 = node(1, node::MAX_ID, terminal_T, terminal_T);
            node nb_2 = node(1, node::MAX_ID - 1, terminal_F, terminal_T);
            node nb_1 = node(0, node::MAX_ID, nb_2, nb_3);

            { // Garbage collect writers early
              node_writer nw_b(zdd_b);
              nw_b << nb_3 << nb_2 << nb_1;
            }

            adiar_assert(zdd_b->width == 2u, "Input validation failed");
            adiar_assert(zdd_b->canonical == true, "Input validation failed");

            /*
            //    { {0}, {1}, {0, 3} } U { {0}, {1}, {0, 1} }
            //
            //                 (1,1)               ---- x0
            //               __/   \__
            //              /         \
            //          (2,2)         (3,3)        ---- x1
            //          /   \         /   \
            //       (F,F) (T,T)   (3,T) (F,T)     ---- x2
            //                      | |   
            //                     (T,T)   
            */

            __zdd out = zdd_union(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(4u));
        });
      });
      
      describe("zdd_intsec", [&]() {
        it("should shortcircuit intermediate nodes", [&]() {
            shared_levelized_file<zdd::node_t> zdd_a;
            /*
            //    { {0, 1}, {0, 2}, {0, 1, 2} }
            //
            //              1          ---- x0
            //             / \
            //            F   2        ---- x1
            //               / \
            //              3   4      ---- x2
            //             / \ / \
            //             F T T T
            */

            node na_4 = node(2, node::MAX_ID, terminal_T, terminal_T);
            node na_3 = node(2, node::MAX_ID - 1, terminal_F, terminal_T);
            node na_2 = node(1, node::MAX_ID, na_3, na_4);
            node na_1 = node(0, node::MAX_ID, terminal_F, na_2);

            { // Garbage collect early and free write-lock
              node_writer nw_a(zdd_a);
              nw_a << na_4 << na_3 << na_2 << na_1;
            }

            adiar_assert(zdd_a->canonical == true, "Input validation failed");
            adiar_assert(zdd_a->width == 2u, "Input validation failed");

            shared_levelized_file<zdd::node_t> zdd_b;
            /*
            //     { {1, 2}, {0, 1, 2} }
            //
            //               1          ---- x0
            //               ||
            //               2          ---- x1
            //              / \
            //             F   3        ---- x2
            //                / \
            //               F   T
            */

            node nb_3 = node(2, node::MAX_ID, terminal_F, terminal_T);
            node nb_2 = node(1, node::MAX_ID, terminal_F, nb_3);
            node nb_1 = node(0, node::MAX_ID, nb_2, nb_2);

            { // Garbage collect early and free write-lock
              node_writer nw_b(zdd_b);
              nw_b << nb_3 << nb_2 << nb_1;
            }

            adiar_assert(zdd_b->canonical == true, "Input validation failed");
            adiar_assert(zdd_b->width == 1u, "Input validation failed");
            
            /*
            // Result of { {0, 1}, {0, 2}, {0, 1, 2} } ∩ { {1, 2}, {0, 1, 2} }
            // 
            //                  (1,1)               ---- x0
            //                  /   \
            //             (F,2)     (2,2)          ---- x1
            //                       /   \
            //                   (3,F)   (4,3)      ---- x2
            //                           /   \
            //                       (T,F)   (T,T)
            //
            // Shorts on (F,2) and (3,F)
            */

            __zdd out = zdd_intsec(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(1u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(3u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should suppress nodes", [&]() {
            shared_levelized_file<zdd::node_t> zdd_a;
            /*
            //        { {0}, {1} }
            //
            //              1          ---- x0
            //             / \
            //            2   T        ---- x1
            //           / \
            //           F T
            */

            node na_2 = node(1, node::MAX_ID, terminal_F, terminal_T);
            node na_1 = node(0, node::MAX_ID, na_2, terminal_T);

            { // Garbage collect early and free write-lock
              node_writer nw_a(zdd_a);
              nw_a << na_2 << na_1;
            }

            adiar_assert(zdd_a->canonical == true, "Input validation failed");
            adiar_assert(zdd_a->width == 1u, "Input validation failed");

            shared_levelized_file<zdd::node_t> zdd_b;
            /*
            //         { Ø, {0} }
            //
            //             1          ---- x0
            //             ||
            //             T
            */

            node nb_1 = node(0, node::MAX_ID, terminal_T, terminal_T);

            { // Garbage collect early and free write-lock
              node_writer nw_b(zdd_b);
              nw_b << nb_1;
            }

            adiar_assert(zdd_b->canonical == true, "Input validation failed");
            adiar_assert(zdd_b->width == 1u, "Input validation failed");
            
            /*
            // Result of { {0}, {1} } ∩ { Ø, {0} }
            // 
            //                  (1,1)               ---- x0
            //                  /   \
            //             (2,T)     (T,T)          ---- x1
            //             /   \
            //         (F,T)   (F,F)
            //
            // Suppresses node (2,T), as F ∩ F = F to
            //
            //                  (1,1)               ---- x0
            //                  /   \
            //             (F,T)     (T,T)
            */

            __zdd out = zdd_intsec(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(0u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("zdd_diff", [&]() {
        it("should flip non-commutative operator", [&]() {
            // As the first input is non-canonical, the input order is swapped
            // Therefore the operator should be flipped to achieve the same behavior
            // This can be seen, as difference is a non-commutative operator

            shared_levelized_file<zdd::node_t> zdd_a;
            /*
            //          { Ø, {1}, {0, 1} }
            //
            //                  1          ---- x0
            //                 / \
            //                2   3        ---- x1
            //               / \ / \
            //               T T F T
            */

            node na_3 = node(1, node::MAX_ID, terminal_F, terminal_T);
            node na_2 = node(1, node::MAX_ID - 1, terminal_T, terminal_T);
            node na_1 = node(0, node::MAX_ID, na_2, na_3);

            { // Garbage collect early and free write-lock
              node_writer nw_a(zdd_a);
              nw_a << na_3 << na_2 << na_1;
            }

            adiar_assert(zdd_a->canonical == false, "Input validation failed");
            adiar_assert(zdd_a->width == 2u, "Input validation failed");

            shared_levelized_file<zdd::node_t> zdd_b;
            /*
            //         { {0}, {1}, {0, 1} }
            //
            //                  1          ---- x0
            //                 / \
            //                2   3        ---- x1
            //               / \ / \
            //               F T T T
            */

            node nb_3 = node(1, node::MAX_ID, terminal_T, terminal_T);
            node nb_2 = node(1, node::MAX_ID - 1, terminal_F, terminal_T);
            node nb_1 = node(0, node::MAX_ID, nb_2, nb_3);

            { // Garbage collect early and free write-lock
              node_writer nw_b(zdd_b);
              nw_b << nb_3 << nb_2 << nb_1;
            }

            adiar_assert(zdd_b->canonical == true, "Input validation failed");
            adiar_assert(zdd_b->width == 2u, "Input validation failed");
            
            /*
            // Result of { Ø, {1}, {0, 1} } \ { {0}, {1}, {0, 1} }
            // 
            //                      (1,1)               ---- x0
            //                    __/   \__
            //                   /         \
            //               (2,2)         (3,3)        ---- x1
            //               /   \         /   \
            //            (T,F) (T,T)   (F,T) (T,T)
            //
            // As T \ T = F, then layer x1 is skipped, to the following figure
            //
            //                      (1,1)               ---- x0
            //                      /   \
            //                  (T,F)   (F,T)
            */

            __zdd out = zdd_diff(zdd_a, zdd_b);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().False());

            level_info_test_stream levels(out);

            AssertThat(levels.can_pull(), Is().True());
            AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

            AssertThat(levels.can_pull(), Is().False());

            AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(0u));

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      // Reset access mode
      access_mode = access_mode_t::AUTO;
    });
  });
});
