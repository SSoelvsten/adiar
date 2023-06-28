#include "../../test.h"

#include <vector>

go_bandit([]() {
  describe("adiar/bdd/quantify.cpp", []() {
    ////////////////////////////////////////////////////////////////////////
    // Sink only BDDs
    shared_levelized_file<bdd::node_t> terminal_F;

    { // Garbage collect writer to free write-lock}
      node_writer nw_F(terminal_F);
      nw_F << node(false);
    }

    shared_levelized_file<bdd::node_t> terminal_T;

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
    shared_levelized_file<bdd::node_t> bdd_1;

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
    shared_levelized_file<bdd::node_t> bdd_2;

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
    shared_levelized_file<bdd::node_t> bdd_3;

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
    shared_levelized_file<bdd::node_t> bdd_4;

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
    shared_levelized_file<bdd::node_t> bdd_5;

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
    shared_levelized_file<bdd::node_t> bdd_x2;

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
    shared_levelized_file<bdd::node_t> bdd_6;

    { // Garbage collect writer to free write-lock
      node_writer nw_6(bdd_6);
      nw_6 << node(3, node::MAX_ID,   ptr_uint64(false),                   ptr_uint64(true))                    // 8
           << node(3, node::MAX_ID-1, ptr_uint64(true),                    ptr_uint64(false))                   // 7
           << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(true))                    // 6
           << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID-1), ptr_uint64(false))                   // 5
           << node(2, node::MAX_ID-2, ptr_uint64(false),                   ptr_uint64(3, ptr_uint64::MAX_ID))   // 4
           << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-2), ptr_uint64(2, ptr_uint64::MAX_ID))   // 3
           << node(1, node::MAX_ID-1, ptr_uint64(2, ptr_uint64::MAX_ID),   ptr_uint64(2, ptr_uint64::MAX_ID-1)) // 2
           << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID),   ptr_uint64(1, ptr_uint64::MAX_ID-1)) // 1
        ;
    }

    // BDD 6 with an 'x4' instead of T that can collapse to T during the
    // transposition sweep
    shared_levelized_file<bdd::node_t> bdd_6_x4T;

    { // Garbage collect writer to free write-lock
      node_writer nw_6(bdd_6_x4T);
      nw_6 << node(4, node::MAX_ID,   ptr_uint64(false),                   ptr_uint64(true))                    // T
           << node(3, node::MAX_ID,   ptr_uint64(false),                   ptr_uint64(4, ptr_uint64::MAX_ID))   // 8
           << node(3, node::MAX_ID-1, ptr_uint64(4, ptr_uint64::MAX_ID),   ptr_uint64(false))                   // 7
           << node(2, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID),   ptr_uint64(4, ptr_uint64::MAX_ID))   // 6
           << node(2, node::MAX_ID-1, ptr_uint64(3, ptr_uint64::MAX_ID-1), ptr_uint64(false))                   // 5
           << node(2, node::MAX_ID-2, ptr_uint64(false),                   ptr_uint64(3, ptr_uint64::MAX_ID))   // 4
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
    shared_levelized_file<bdd::node_t> bdd_7;

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
    shared_levelized_file<bdd::node_t> bdd_8a, bdd_8b;

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
    // BDD 9 (T terminal)
    /*
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (inner sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       |    7        ---- x5
    //       |   / \
    //       8   F T       ---- x6 (transposition sweep)
    //      / \
    //      F T
    */
    node n9T_8 = node(6, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n9T_7 = node(5, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n9T_6 = node(4, node::MAX_ID,   n9T_8.uid(),       n9T_7.uid());
    node n9T_5 = node(3, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n9T_4 = node(3, node::MAX_ID-1, n9T_6.uid(),       ptr_uint64(false));
    node n9T_3 = node(2, node::MAX_ID,   n9T_5.uid(),       n9T_7.uid());
    node n9T_2 = node(2, node::MAX_ID-1, n9T_4.uid(),       n9T_5.uid());
    node n9T_1 = node(1, node::MAX_ID,   n9T_2.uid(),       n9T_3.uid());

    shared_levelized_file<bdd::node_t> bdd_9T;
    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_9T);
      nw << n9T_8 << n9T_7 << n9T_6 << n9T_5 << n9T_4 << n9T_3 << n9T_2 << n9T_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 9 (F terminal)
    /* input
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (final sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       F    7        ---- x5
    //           / \
    //           F 8       ---- x6 (transposition sweep)
    //            / \
    //            F T
    */
    node n9F_8 = node(6, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n9F_7 = node(5, node::MAX_ID,   ptr_uint64(false), n9F_8.uid());
    node n9F_6 = node(4, node::MAX_ID,   ptr_uint64(false), n9F_7.uid());
    node n9F_5 = node(3, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n9F_4 = node(3, node::MAX_ID-1, n9F_6.uid(),       ptr_uint64(false));
    node n9F_3 = node(2, node::MAX_ID,   n9F_5.uid(),       n9F_7.uid());
    node n9F_2 = node(2, node::MAX_ID-1, n9F_4.uid(),       n9F_5.uid());
    node n9F_1 = node(1, node::MAX_ID,   n9F_2.uid(),       n9F_3.uid());

    shared_levelized_file<bdd::node_t> bdd_9F;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_9F);
      nw << n9F_8 << n9F_7 << n9F_6 << n9F_5 << n9F_4 << n9F_3 << n9F_2 << n9F_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 10 : node (8) dies when quantifying x2 and x3
    /*
    //          _1_           ---- x0
    //         /   \
    //         F   _2_        ---- x1
    //            /   \
    //            3   4       ---- x2
    //           / \ / \
    //           5  6  7      ---- x3
    //          / \| |/ \
    //          T   8   T     ---- x4
    //             / \
    //             F T
    */
    node n10_8 = node(4, node::MAX_ID,   ptr_uint64(false), ptr_uint64(true));
    node n10_7 = node(3, node::MAX_ID,   n10_8.uid(),       ptr_uint64(true));
    node n10_6 = node(3, node::MAX_ID-1, n10_8.uid(),       n10_8.uid());
    node n10_5 = node(3, node::MAX_ID-2, ptr_uint64(true),  n10_8.uid());
    node n10_4 = node(2, node::MAX_ID,   n10_6.uid(),       n10_7.uid());
    node n10_3 = node(2, node::MAX_ID-1, n10_5.uid(),       n10_6.uid());
    node n10_2 = node(1, node::MAX_ID,   n10_3.uid(),       n10_4.uid());
    node n10_1 = node(0, node::MAX_ID,   ptr_uint64(false), n10_2.uid());

    shared_levelized_file<bdd::node_t> bdd_10;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_10);
      nw << n10_8 << n10_7 << n10_6 << n10_5 << n10_4 << n10_3 << n10_2 << n10_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    describe("bdd_exists(const bdd&, bdd::label_t)", [&]() {
      it("should quantify T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_T, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(terminal_T));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_F, 21);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(terminal_F));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut quantification of root into T terminal [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should shortcut quantification of root into T terminal [x2]", [&]() {
        __bdd out = bdd_exists(bdd_x2, 2);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should shortcut quantification on non-existent label in input [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify bottom-most nodes [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 1);

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should quantify root without terminal arcs [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should quantify root with F terminal [BDD 5]", [&]() {
        __bdd out = bdd_exists(bdd_5, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to its own leaf
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));

        // TODO: meta variables...
      });

      it("should quantify nodes with terminal or nodes as children [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,   ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,   ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,   ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to its own leaf
                   Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should keep nodes as is when skipping quantified level [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 1);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,NIL) since while n4 < NIL we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // n3
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // n3
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.high()
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 2);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.high()
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should resolve terminal-terminal requests in [BDD 5]", [&]() {
        __bdd out = bdd_exists(bdd_5, 1);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,NIL) while n4 < NIL since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.high()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("can shortcut/prune irrelevant subtrees [OR-chain]", [&]() {
        shared_levelized_file<bdd::node_t> bdd_chain;

        node n4 = node(3, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
        node n3 = node(2, node::MAX_ID, n4.uid(), ptr_uint64(true));
        node n2 = node(1, node::MAX_ID, n3.uid(), ptr_uint64(true));
        node n1 = node(0, node::MAX_ID, n2.uid(), ptr_uint64(true));

        { // Garbage collect writer to free write-lock
          node_writer bdd_chain_w(bdd_chain);
          bdd_chain_w << n4 << n3 << n2 << n1;
        }

        __bdd out = bdd_exists(bdd_chain, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to quantification of x2
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_exists(bdd_6, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (8,F)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(true) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,F)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(5u));
      });

      it("can forward multiple arcs to the same node across a level [BDD 7]", [&]() {
        __bdd out = bdd_exists(bdd_7, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_exists(bdd_8a, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8b]", [&]() {
        __bdd out = bdd_exists(bdd_8b, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });
    });

    describe("bdd_exists(const bdd&, const std::function<bool(bdd::label_t)>&)", [&]() {
      it("returns input on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_exists(bdd_1, [](const bdd::label_t) { return false; });
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == SINGLETON", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("quantifies odd variables in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, 2 in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_t x) { return x == 1 || x == 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;
          const bdd out = bdd_exists(in, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          bdd out = bdd_exists(bdd_1, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 1 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_exists(bdd_1, [&calls](const bdd::label_t) {
            calls++;
            return true;
          });

          AssertThat(calls, Is().EqualTo(1));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_t) { return true; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          // TODO: meta variables...
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == PARTIAL", [&]() {
        quantify_mode = quantify_mode_t::PARTIAL;

        // TODO

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("quantifies odd variables in BDD 4", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1", [&]() {
          bdd out = bdd_exists(bdd_1, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_t) { return true; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd out = bdd_exists(bdd_9T, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(3, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd out = bdd_exists(bdd_9F, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant", [&bdd_6_x4T]() {
          bdd out = bdd_exists(bdd_6_x4T, [](const bdd::label_t x) { return x == 4 || x == 2 || x == 1; });

          // TODO predict output!
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution", [&bdd_10]() {
          bdd out = bdd_exists(bdd_10, [](const bdd::label_t x) { return x == 3 || x == 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solutions multiple times", [&]() {
          bdd::label_t var = 7;

          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out = bdd_exists(bdd_6, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == AUTO", [&]() {
        quantify_mode = quantify_mode_t::AUTO;

        it("quantifies even variables in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;
          const bdd out = bdd_exists(in, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
        bdd out = bdd_exists(bdd_4, [](const bdd::label_t) { return true; });

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());

        // TODO: meta variables...
      });
    });

    describe("bdd_exists(const bdd&, const std::function<bdd::label_t()>&)", [&]() {
      it("returns input on -1 generator BDD 1 [&&]", [&]() {
        __bdd out = bdd_exists(bdd_1, []() { return -1; });
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == SINGLETON", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("quantifies 3, 1, -1 in BDD 4 [&&]", [&]() {
          bdd::label_t var = 3;

          bdd out = bdd_exists(bdd_4, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 2, 0, -2 in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;

          bdd::label_t var = 2;

          bdd out = bdd_exists(in, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 variables in BDD 1 [&&]", [&]() {
          bdd::label_t var = 1;

          bdd out = bdd_exists(bdd_1, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out = bdd_exists(bdd_3, [&calls]() { return 2 - 2*(calls++); });

          // What could be expected is 3 calls: 2, 0, -2 . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("quantifies 3, 1, -1 in BDD 4 [&&]", [&]() {
          bdd::label_t var = 3;

          bdd out = bdd_exists(bdd_4, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 variables in BDD 1 [&&]", [&]() {
          bdd::label_t var = 1;

          bdd out = bdd_exists(bdd_1, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd::label_t var = 6;
          bdd out = bdd_exists(bdd_9T, [&var]() {
            const bdd::label_t res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(3, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd::label_t var = 6;
          bdd out = bdd_exists(bdd_9F, [&var]() {
            const bdd::label_t res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant", [&bdd_6_x4T]() {
          bdd::label_t var = 4;
          bdd out = bdd_exists(bdd_6_x4T, [&var]() {
            const bdd::label_t res = var;
            switch (var) {
            case 4:  { var = 2;  break; } // <-- 4: transposing
            case 2:  { var = 1;  break; } // <-- 2: shortuctting / irrelevant
            default: { var = -1; break; } // <-- 1: final sweep
            }
            return res;
          });

          // TODO predict output!
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution", [&bdd_10]() {
          bdd::label_t var = 3;
          bdd out = bdd_exists(bdd_10, [&var]() {
            const bdd::label_t res = var;
            if (2 < var) { var -= 1; }
            else         { var = -1; }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solutions multiple times", [&]() {
          bdd::label_t var = 7;

          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out = bdd_exists(bdd_6, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });

    describe("bdd_exists(const bdd&, IT begin, IT end)", [&]() {
      it("quantifies [1, 3].rbegin() in BDD 4 [&&]", [&]() {
        std::vector<bdd::label_t> vars = { 1 , 3 };

        bdd out = bdd_exists(bdd_4, vars.rbegin(), vars.rend());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (3)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (1)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(2, ptr_uint64::MAX_ID),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("quantifies [2, 0].begin() in BDD 4 [const &]", [&]() {
        const bdd in = bdd_4;
        const std::vector<bdd::label_t> vars = { 2, 0 };

        bdd out = bdd_exists(in, vars.begin(), vars.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (5)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (2')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(3, ptr_uint64::MAX_ID),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });
    });

    ////////////////////////////////////////////////////////////////////////////
    // We will not test the Forall operator as much, since it is the same
    // underlying algorithm, but just with the AND operator.
    describe("bdd_forall(const bdd&, bdd::label_t)", [&]() {
      it("quantifies T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_T, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(terminal_T));
        AssertThat(out.negate, Is().False());
      });

      it("quantifies F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_F, 21);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(terminal_F));
        AssertThat(out.negate, Is().False());
      });

      it("quantifies root with non-shortcutting terminal [BDD 1]", [&]() {
        __bdd out = bdd_forall(bdd_1, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("quantifies root of [BDD 3]", [&]() {
        __bdd out = bdd_forall(bdd_3, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should prune shortcuttable requests [BDD 4]", [&]() {
        __bdd out = bdd_forall(bdd_4, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 5.low()
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high() and 4.high()
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_forall(bdd_6, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,T)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_forall(bdd_8a, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // false due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });

    describe("bdd_forall(const bdd&, const std::function<bool(bdd::label_t)>&)", [&]() {
      it("returns input on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd_1, [](const bdd::label_t) { return false; });
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == SINGLETON", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("quantifies even variables in BDD 1 [const &]", [&]() {
          const bdd in = bdd_1;
          const bdd out = bdd_forall(in, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies <= 2 variables in BDD 4 [&&]", [&]() {
          const bdd out = bdd_forall(bdd_4, [](const bdd::label_t x) { return x <= 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 1 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_forall(bdd_5, [&calls](const bdd::label_t) {
            calls++;
            return true;
          });

          AssertThat(calls, Is().EqualTo(1));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == PARTIAL", [&]() {
        quantify_mode = quantify_mode_t::PARTIAL;

        // TODO

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("quantifies even variables in BDD 1", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd out = bdd_forall(bdd_not(bdd_9T), [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(3, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd out = bdd_forall(bdd_not(bdd_9F), [](const bdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == AUTO", [&]() {
        quantify_mode = quantify_mode_t::AUTO;

        it("quantifies odd variables in BDD 1", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_t x) { return x % 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });

    describe("bdd_forall(const bdd&, const std::function<bdd::label_t()>&)", [&]() {
      it("returns input on -1 geneator in BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd_1, []() { return -1; });
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == SINGLETON", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("quantifies 0, -2 in BDD 1 [const &]", [&]() {
          const bdd in = bdd_1;

          bdd::label_t var = 0;

          const bdd out = bdd_forall(in, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 in BDD 1 [&&]", [&]() {
          bdd::label_t var = 1;

          const bdd out = bdd_forall(bdd_1, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out = bdd_forall(bdd_3, [&calls]() { return 2 - 2*(calls++); });

          // What could be expected is 3 calls: 2, 0, -2 . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("quantifies 0, -2 in BDD 1", [&]() {
          bdd::label_t var = 0;

          const bdd out = bdd_forall(bdd_1, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 in BDD 1", [&]() {
          bdd::label_t var = 1;

          const bdd out = bdd_forall(bdd_1, [&var]() {
            const bdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd::label_t var = 6;
          bdd out = bdd_forall(bdd_not(bdd_9T), [&var]() {
            const bdd::label_t res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(false),
                                                         ptr_uint64(3, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd::label_t var = 6;
          bdd out = bdd_forall(bdd_not(bdd_9F), [&var]() {
            const bdd::label_t res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         ptr_uint64(5, node::MAX_ID),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });

    describe("bdd_forall(const bdd&, IT begin, IT end)", [&]() {
      it("quantifies [0].rbegin() in BDD 1 [const &]", [&]() {
        const bdd in = bdd_1;
        const std::vector<bdd::label_t> vars = { 0 };

        const bdd out = bdd_forall(in, vars.rbegin(), vars.rend());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (1')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("quantifies [1].begin() in BDD 1 [&&]", [&]() {
        const std::vector<bdd::label_t> vars = { 1 };
        const bdd out = bdd_forall(bdd_1, vars.begin(), vars.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (1')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(true),
                                                       ptr_uint64(false))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });
    });
  });
 });
