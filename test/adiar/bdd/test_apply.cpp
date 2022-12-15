#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/apply.cpp", []() {
    shared_levelized_file<bdd::node_t> bdd_F;
    shared_levelized_file<bdd::node_t> bdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(bdd_F);
      nw_F << node(false);

      node_writer nw_T(bdd_T);
      nw_T << node(true);
    }

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_t> bdd_x0;
    shared_levelized_file<bdd::node_t> bdd_not_x0;
    shared_levelized_file<bdd::node_t> bdd_x1;
    shared_levelized_file<bdd::node_t> bdd_x2;

    { // Garbage collect writers early
      node_writer nw_x0(bdd_x0);
      nw_x0 << node(0, node::MAX_ID, terminal_F, terminal_T);

      node_writer nw_not_x0(bdd_not_x0);
      nw_not_x0 << node(0, node::MAX_ID, terminal_T, terminal_F);

      node_writer nw_x1(bdd_x1);
      nw_x1 << node(1, node::MAX_ID, terminal_F, terminal_T);

      node_writer nw_x2(bdd_x2);
      nw_x2 << node(2, node::MAX_ID, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_t> bdd_1;
    /*
            1        ---- x0
           / \
           | 2       ---- x1
           |/ \
           3   4     ---- x2
          / \ / \
          F T T 5    ---- x3
               / \
               F T
    */

    node n1_5 = node(3, node::MAX_ID, terminal_F, terminal_T);
    node n1_4 = node(2, node::MAX_ID, terminal_T, n1_5.uid());
    node n1_3 = node(2, node::MAX_ID-1, terminal_F, terminal_T);
    node n1_2 = node(1, node::MAX_ID, n1_3.uid(), n1_4.uid());
    node n1_1 = node(0, node::MAX_ID, n1_3.uid(), n1_2.uid());

    { // Garbage collect early and free write-lock
      node_writer nw_1(bdd_1);
      nw_1 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    shared_levelized_file<bdd::node_t> bdd_2;
    /*
                   ---- x0

             1     ---- x1
            / \
            | T    ---- x2
            |
            2      ---- x3
           / \
           T F
    */

    node n2_2 = node(3, node::MAX_ID, terminal_T, terminal_F);
    node n2_1 = node(1, node::MAX_ID, n2_2.uid(), terminal_T);

    { // Garbage collect early and free write-lock
      node_writer nw_2(bdd_2);
      nw_2 << n2_2 << n2_1;
    }

    shared_levelized_file<bdd::node_t> bdd_3;
    /*
                1         ---- x0
               / \
               2 3        ---- x1
             _/ X \_
            | _/ \_ |
             X     X
            / \   / \
           4  5  6  7     ---- x2
          / \/ \/ \/ \
          F T  8  T  F    ---- x3
              / \
              F T

    */

    node n3_8 = node(3, node::MAX_ID, terminal_F, terminal_T);
    node n3_7 = node(2, node::MAX_ID, terminal_T, terminal_F);
    node n3_6 = node(2, node::MAX_ID - 1, n3_8.uid(), terminal_T);
    node n3_5 = node(2, node::MAX_ID - 2, terminal_T, n3_8.uid());
    node n3_4 = node(2, node::MAX_ID - 3, terminal_F, terminal_T);
    node n3_3 = node(1, node::MAX_ID, n3_4.uid(), n3_6.uid());
    node n3_2 = node(1, node::MAX_ID - 1, n3_5.uid(), n3_7.uid());
    node n3_1 = node(0, node::MAX_ID, n3_2.uid(), n3_3.uid());

    { // Garbage collect early and free write-lock
      node_writer nw_3(bdd_3);
      nw_3 << n3_8 << n3_7 << n3_6 << n3_5 << n3_4 << n3_3 << n3_2 << n3_1;
    }

    /* The product construction of bbd_1 and bdd_2 above is as follows in sorted order.

                                            (1,1)                       ---- x0
                                            \_ _/
                                             _X_                        // Match in fst, but not coordinatewise
                                            /   \
                                        (3,1)   (2,1)                   ---- x1
                                       /    \_ _/    \
                                      /       X       \
                                     /_______/ \       \
                                     |          \       \
                                 (3,2)          (3,T)   (4,T)           ---- x2
                                  \ \           /   \   /   \
                                   \ \      (F,T)   (T,T)   /
                                    \ \________ ___________/
                                     \________ X________
                                              X_________\ _______
                                             /           \       \
                                            /             \       \
                                         (5,T)         (F,2)     (T,2)   ---- x3
                                         /   \         /   \     /   \
                                      (F,T) (T,T)   (F,T)(F,F)  (T,T)(T,F)
    */

    describe("bdd_and(f,g)", [&]() {
      it("should resolve F /\\ T terminal-only BDDs", [&]() {
        __bdd out = bdd_and(bdd_F, bdd_T);
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should resolve T /\\ T terminal-only BDDs", [&]() {
        shared_levelized_file<bdd::node_t> bdd_T2;
        {
          node_writer w(bdd_T2);
          w << node(true);
        }

        __bdd out = bdd_and(bdd_T, bdd_T2);
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

      it("should shortcut on irrelevance on x0 /\\ T", [&]() {
        __bdd out = bdd_and(bdd_x0, bdd_T);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut F /\\ x0", [&]() {
        __bdd out = bdd_and(bdd_F, bdd_x0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should x0 and !x0", [&]() {
        /*
                   1     ---- x0
                  / \
                  F F
        */

        __bdd out = bdd_and(bdd_x0, bdd_not_x0);

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

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should shortcut F /\\ [2]", [&]() {
        __bdd out = bdd_and(bdd_F, bdd_2);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should compute (and shortcut) BBD 1 /\\ [2]", [&]() {
        /*
                            1                        ---- x0
                            X
                           / \
                          2   3                      ---- x1
                         / \ / \
                        /   X   \
                       /___/ \   \
                      /      |    \
                     4       5     6                 ---- x2
                    / \     / \_ _/ \
                    F 7     F   T   8                ---- x3
                     / \           / \
                     T F           F T
        */

        __bdd out = bdd_and(bdd_1, bdd_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,1) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,0) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,2) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,0) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(4u));
      });

      it("should return input on being given the same BDD twice", [&]() {
        __bdd out = bdd_and(bdd_1, bdd_1);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
        AssertThat(out.negate, Is().False());
      });

      it("should group all recursion requests together", [&]() {
        // This is a counter-example to the prior "break ties on fst() with
        // snd()" approach. Here we will have three requests to the level of
        // x2, but in the following order:
        //
        // [((2,0),(2,1)), ((2,1),(2,0)), ((2,0),(2,1))]
        //
        // which all are tied, and hence the prior version would create
        // three nodes on this level rather than just two.

        //TODO: This drawing seems to not fit - fix it?

        /*
                 1    ---- x0
                / \
                2 |   ---- x1
               / \|
               3  4   ---- x2
              / \/ \
              T  F T
        */

        // The second version is the same but has the nodes 3 and 4 mirrored
        // and the T terminals are replaced with an arc to a node for x3.

        shared_levelized_file<bdd::node_t> bdd_group_1, bdd_group_2;
        { // Garbage collect writers to free write-lock
          node_writer w1(bdd_group_1);
          w1 << node(2,1, ptr_uint64(false), ptr_uint64(true))
             << node(2,0, ptr_uint64(true), ptr_uint64(false))
             << node(1,0, ptr_uint64(2,0), ptr_uint64(2,1))
             << node(0,1, ptr_uint64(1,0), ptr_uint64(2,1));

          node_writer w2(bdd_group_2);
          w2 << node(3,0, ptr_uint64(false), ptr_uint64(true))
             << node(2,1, ptr_uint64(3,0), ptr_uint64(false))
             << node(2,0, ptr_uint64(false), ptr_uint64(3,0))
             << node(1,0, ptr_uint64(2,1), ptr_uint64(2,0))
             << node(0,1, ptr_uint64(1,0), ptr_uint64(2,0));
        }

        __bdd out = bdd_and(bdd_group_1, bdd_group_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,1) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5) i.e. the added node
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));

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

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(3u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });

    describe("bdd_nand(f,g)", [&]() {
      it("should shortcut on negating on T and x0", [&]() {
        __bdd out = bdd_nand(bdd_x0, bdd_T);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().True());
      });

      it("should shortcut on negating on T and x0", [&]() {
        __bdd out = bdd_nand(bdd_x0, bdd_T);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().True());
      });

      it("should collapse on the same BDD twice, where one is negated [1]", [&]() {
        __bdd out = bdd_nand(bdd_2, bdd_not(bdd_2));

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
    });

    describe("bdd_or(f,g)", [&]() {
      it("should resolve T \\/ F terminal-only BDDs", [&]() {
        __bdd out = bdd_or(bdd_T, bdd_F);
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

      it("should resolve F \\/ F terminal-only BDDs", [&]() {
        shared_levelized_file<bdd::node_t> bdd_F2;

        {
          node_writer w(bdd_F2);
          w << node(false);
        }

        __bdd out = bdd_or(bdd_F, bdd_F2);
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should shortcut on irrelevance on x0 \\/ F", [&]() {
        __bdd out = bdd_or(bdd_x0, bdd_F);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().False());
      });

      it("should OR shortcut on irrelevance F \\/ x0", [&]() {
        __bdd out = bdd_or(bdd_F, bdd_x0);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut on x0 \\/ x2", [&]() {
        /*
                   1     ---- x0
                  / \
                  | T
                  |
                  2      ---- x2
                 / \
                 F T
        */

        __bdd out = bdd_or(bdd_x0, bdd_x2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), terminal_T }));

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

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should shortcut [1] \\/ T", [&]() {
        __bdd out = bdd_or(bdd_1, bdd_T);

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

      it("should compute (and shortcut) [2] \\/ T", [&]() {
        __bdd out = bdd_or(bdd_2, bdd_T);

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

      it("should compute (and shortcut) [1] \\/ [2]", [&]() {
        /*
                           1       ---- x0
                          / \
                         2   3     ---- x1
                        / \ / \
                        | T | T
                        \_ _/
                          4        ---- x2
                         / \
                         5  T      ---- x3
                        / \
                        T F
        */

        __bdd out = bdd_or(bdd_1, bdd_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(4u));
      });

    });

    describe("bdd_nor(f,g)", [&]() {
      it("should collapse on the same BDD twice to a terminal, where one is negated [2]", [&]() {
        __bdd out = bdd_nor(bdd_not(bdd_3), bdd_3);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });
    });


    describe("bdd_xor(f,g)", [&]() {
      it("should resolve F ^ T terminal-only BDDs", [&]() {
        __bdd out = bdd_xor(bdd_F, bdd_T);
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

      it("should resolve T ^ T terminal-only BDDs", [&]() {
        __bdd out = bdd_xor(bdd_T, bdd_T);
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()) ;
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should shortcut on negating on x0 ^ T", [&]() {
        __bdd out = bdd_xor(bdd_x0, bdd_T);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().True());
      });

      it("should shortcut on negating on T ^ x0", [&]() {
        __bdd out = bdd_xor(bdd_x0, bdd_T);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().True());
      });

      it("should compute x0 ^ x1", [&]() {
        /* The order on the leaves are due to the sorting of terminal requests
           after evaluating x0

                   1     ---- x0
                  / \
                 2   3   ---- x1
                / \ / \
                F T T F
        */

        __bdd out = bdd_xor(bdd_x0, bdd_x1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should compute [2] ^ x2", [&]() {
        /*
                                         ---- x0

                         (1,1)           ---- x1
                         /   \
                     (2,1)   (T,1)       ---- x2
                     /   \   /   \
                    /     \  T   F
                    |     |
                (2,F)     (2,T)          ---- x3
                /   \     /   \
                T   F     F   T
        */
        __bdd out = bdd_xor(bdd_2, bdd_x2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));

        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should compute [1] ^ [2]", [&]() {
        /* There is no shortcutting possible on an XOR, so see the product
           construction above. */

        __bdd out = bdd_xor(bdd_1, bdd_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,2) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(5u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(4u));
      });

      it("should compute [3] ^ [1]", [&]() {
        /* The queue appD_data is used to forward data across the level. When
           [1] and 3 are combined, this is needed

                  The product between the [3] and [1] then is

                                   (1,1)                      ---- x0
                           ________/   \_______
                          /                    \
                        (2,3)                  (3,2)          ---- x1
                        /   \_________ ________/   \
                        |             X            |          //      (5,3) (7,3) (4,3) (6,4)
                        \__ _________/ \__________ /          // min:   0     0     0     1
                        ___X___                   X           // max:   1     3     0     2
                       /       \            _____/ \          // coord: 2     3     1     4
                      /         \          /        \
                   (4,3)       (5,3)    (6,4)     (7,3)       ---- x2
                   /   \       /   \    /   \     /   \
                (F,F) (T,T) (T,F)  |   /     \  (T,F) (F,T)
                                   |  /       \
                                   | /        |
                                   |/         |
                                 (8,T)      (T,5)             ---- x3
                                 /   \      /   \
                              (F,T) (T,T) (T,F) (T,T)
        */

        __bdd out = bdd_xor(bdd_3, bdd_1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (6,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (7,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,3) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,3)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,4u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(5u));
      });

      it("should collapse on the same BDD twice", [&]() {
        __bdd out = bdd_xor(bdd_1, bdd_1);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should collapse on the same BDD twice to a terminal, when both are negated", [&]() {
        __bdd out = bdd_xor(bdd_not(bdd_1), bdd_not(bdd_1));

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });


    });

    describe("bdd_xnor(f,g)", [&]() {
      // TODO
    });

    describe("bdd_imp(f,g)", [&]() {
      it("should resolve F -> T terminal-only BDDs", [&]() {
        __bdd out = bdd_imp(bdd_F, bdd_T);
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

      it("should resolve T -> F terminal-only BDDs", [&]() {
        __bdd out = bdd_imp(bdd_T, bdd_F);
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should resolve T -> T terminal-only BDDs", [&]() {
        __bdd out = bdd_imp(bdd_T, bdd_T);
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

      it("should shortcut on irrelevance on T -> x0", [&]() {
        __bdd out = bdd_imp(bdd_T, bdd_x0);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut on x0 -> x1", [&]() {
        /* The order on the leaves are due to the sorting of terminal requests
           after evaluating x0

                   1     ---- x0
                  / \
                  T 2    ---- x1
                   / \
                   F T
        */

        __bdd out = bdd_imp(bdd_x0, bdd_x1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_T }));

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

        AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should shortcut F -> [1]", [&]() {
        __bdd out = bdd_imp(bdd_F, bdd_1);

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

      it("should return the input when given the same BDD twice, where one is negated [1]", [&]() {
        __bdd out = bdd_imp(bdd_not(bdd_2), bdd_2);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_2));
        AssertThat(out.negate, Is().False()); // negated the already negated input doubly-negating
      });

      it("should return input when given the same BDD twice, where one is negated [2]", [&]() {
        __bdd out = bdd_imp(bdd_2, bdd_not(bdd_2));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_2));
        AssertThat(out.negate, Is().True()); // negated the first of the two
      });
    });

    describe("bdd_invimp(f,g)", [&]() {
      // TODO
    });

    describe("bdd_equiv(f,g)", [&]() {
      // TODO
    });

    describe("bdd_diff(f,g)", [&]() {
      // TODO
    });

    describe("bdd_less(f,g)", [&]() {
      // TODO
    });
  });
 });
