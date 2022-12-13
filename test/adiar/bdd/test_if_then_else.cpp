#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/if_then_else.cpp", []() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_t> bdd_F;
    shared_levelized_file<bdd::node_t> bdd_T;
    shared_levelized_file<bdd::node_t> bdd_x0;
    shared_levelized_file<bdd::node_t> bdd_not_x0;
    shared_levelized_file<bdd::node_t> bdd_x1;
    shared_levelized_file<bdd::node_t> bdd_not_x1;
    shared_levelized_file<bdd::node_t> bdd_x2;
    shared_levelized_file<bdd::node_t> bdd_x0_xor_x1;
    shared_levelized_file<bdd::node_t> bdd_x0_xor_x2;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(bdd_F);
      nw_F << node(false);

      node_writer nw_T(bdd_T);
      nw_T << node(true);

      node_writer nw_x0(bdd_x0);
      nw_x0 << node(0,0,terminal_F,terminal_T);

      node_writer nw_not_x0(bdd_not_x0);
      nw_not_x0 << node(0,0,terminal_T,terminal_F);

      node_writer nw_x1(bdd_x1);
      nw_x1 << node(1,0,terminal_F,terminal_T);

      node_writer nw_not_x1(bdd_not_x1);
      nw_not_x1 << node(1,0,terminal_T,terminal_F);

      node_writer nw_x2(bdd_x2);
      nw_x2 << node(2,0,terminal_F,terminal_T);

      node_writer nw_x0_xor_x1(bdd_x0_xor_x1);
      nw_x0_xor_x1 << node(1,1,terminal_T,terminal_F)
                   << node(1,0,terminal_F,terminal_T)
                   << node(0,0,ptr_uint64(1,0),ptr_uint64(1,1));

      node_writer nw_x0_xor_x2(bdd_x0_xor_x2);
      nw_x0_xor_x2 << node(2,1,terminal_T,terminal_F)
                   << node(2,0,terminal_F,terminal_T)
                   << node(0,0,ptr_uint64(2,0),ptr_uint64(2,1));
    }

    shared_levelized_file<bdd::node_t> bdd_1;
    /*
                            _1_              ---- x0
                           /   \
                           2   3             ---- x1
                          / \ / \
                          6  5  4            ---- x2
                         / \/ \/ \
                         F T  7  T           ---- x3
                             / \
                             F T
    */

    { // Garbage collect writers to free write-lock
      node_writer nw_1(bdd_1);
      nw_1 << node(3,0,terminal_F,terminal_T)           // 7
           << node(2,2,terminal_F,terminal_T)           // 6
           << node(2,1,terminal_T,ptr_uint64(3,0))      // 5
           << node(2,0,ptr_uint64(3,0),terminal_T)      // 4
           << node(1,1,ptr_uint64(2,1),ptr_uint64(2,0)) // 3
           << node(1,0,ptr_uint64(2,2),ptr_uint64(2,1)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(1,1)) // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_2;
    /*
                               __1__         ---- x0
                              /     \
                            _2_     _3_      ---- x1
                           /   \   /   \
                           4   5   6   7     ---- x2
                          / \ / \ / \ / \
                          F 8 F 9 T F F T    ---- x3
                           / \ / \
                           T F F T
    */

    { // Garbage collect writers to free write-lock
      node_writer nw_2(bdd_2);
      nw_2 << node(3,1,terminal_F,terminal_T)            // 9
           << node(3,0,terminal_T,terminal_F)            // 8
           << node(2,3,terminal_F,terminal_T)            // 7
           << node(2,2,terminal_T,terminal_F)            // 6
           << node(2,1,terminal_F,ptr_uint64(3,1))       // 5
           << node(2,0,terminal_F,ptr_uint64(3,0))       // 4
           << node(1,1,ptr_uint64(2,2),ptr_uint64(2,3))  // 3
           << node(1,0,ptr_uint64(2,0),ptr_uint64(2,1))  // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(1,1))  // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_3;
    /*
                              __1__         ---- x0
                             /     \
                           _2_      \       ---- x1
                          /   \      \
                          3   4      5      ---- x2
                         / \ / \    / \
                         T F F 6    F T     ---- x3
                              / \
                              F T
    */

    { // Garbage collect writers to free write-lock
      node_writer nw_3(bdd_3);
      nw_3 << node(3,0,terminal_F,terminal_T)           // 6
           << node(2,2,terminal_F,terminal_T)           // 5
           << node(2,1,terminal_F,ptr_uint64(3,0))      // 4
           << node(2,0,terminal_T,terminal_F)           // 3
           << node(1,0,ptr_uint64(2,0),ptr_uint64(2,1)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(2,2)) // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_4;
    /*
                               __1__         ---- x0
                              /     \
                            _2_      \       ---- x1
                           /   \      \
                           4   5      3      ---- x2
                          / \ / \    / \
                          6 T F 7    T F     ---- x3
                         / \   / \
                         T F   F T
    */
    { // Garbage collect writers to free write-lock
      node_writer nw_4(bdd_4);
      nw_4 << node(3,1,terminal_F,terminal_T)           // 7
           << node(3,0,terminal_T,terminal_F)           // 6
           << node(2,2,terminal_F,ptr_uint64(3,1))      // 5
           << node(2,1,ptr_uint64(3,0),terminal_T)      // 4
           << node(2,0,terminal_T,terminal_F)           // 3
           << node(1,0,ptr_uint64(2,1),ptr_uint64(2,2)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(2,0)) // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_5;
    /*
                               __1__         ---- x0
                              /     \
                            _2_      \       ---- x1
                           /   \      \
                           5   3      4      ---- x2
                          / \ / \    / \
                          F 6 T F    F T     ---- x3
                           / \
                           T F
    */
    { // Garbage collect writers to free write-lock
      node_writer nw_5(bdd_5);
      nw_5 << node(3,0,terminal_T,terminal_F)           // 6
           << node(2,2,terminal_F,ptr_uint64(3,0))      // 5
           << node(2,1,terminal_F,terminal_T)           // 4
           << node(2,0,terminal_T,terminal_F)           // 3
           << node(1,0,ptr_uint64(2,2),ptr_uint64(2,0)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(2,1)) // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_6;
    /*
                                    1         ---- x0
                                   / \
                                   F _2_      ---- x1
                                    /   \
                                    3   4     ---- x2
                                   / \ / \
                                   F T T F

    */
    { // Garbage collect writers to free write-lock
      node_writer nw_6(bdd_6);
      nw_6 << node(2,1,terminal_T,terminal_F)
           << node(2,0,terminal_F,terminal_T)
           << node(1,0,ptr_uint64(2,0),ptr_uint64(2,1))
           << node(0,0,terminal_F,ptr_uint64(1,0))
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_not_6;
    /*
                                    1         ---- x0
                                   / \
                                   T _2_      ---- x1
                                    /   \
                                    3   4     ---- x2
                                   / \ / \
                                   T F F T

    */
    { // Garbage collect writers to free write-lock
      node_writer nw_not_6(bdd_not_6);
      nw_not_6 << node(2,1,terminal_F,terminal_T)
               << node(2,0,terminal_T,terminal_F)
               << node(1,0,ptr_uint64(2,0),ptr_uint64(2,1))
               << node(0,0,terminal_T,ptr_uint64(1,0))
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_7;
    /*
                                     1         ---- x0
                                    / \
                                    2__\       ---- x1
                                   /   |
                                   3   4       ---- x2
                                  / \ / \
                                  F  5  T      ---- x3
                                    / \
                                    F T
    */

    {
      node_writer nw_7(bdd_7);
      nw_7 << node(3,0,terminal_F,terminal_T)           // 5
           << node(2,1,ptr_uint64(3,0),terminal_T)      // 4
           << node(2,0,terminal_F,ptr_uint64(3,0))      // 3
           << node(1,0,ptr_uint64(2,0),ptr_uint64(2,1)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(2,1)) // 1
        ;
    }

    shared_levelized_file<bdd::node_t> bdd_8;
    /*
                                   1         ---- x0
                                  / \
                                  2 |        ---- x1
                                 / \|
                                 3  |        ---- x2
                                / \/
                                T 4          ---- x3
                                 / \
                                 T F
    */
    {
      node_writer nw_8(bdd_8);
      nw_8 << node(3,0,terminal_T,terminal_F)            // 4
           << node(2,0,terminal_T,ptr_uint64(3,0))      // 3
           << node(1,0,ptr_uint64(2,0),ptr_uint64(3,0)) // 2
           << node(0,0,ptr_uint64(1,0),ptr_uint64(3,0)) // 1
        ;
    }

    describe("Trivial evaluations", [&]() {
      // Trivial evaluations by given a terminal
      it("should give back first file on if-true (true ? x0 : x1)", [&]() {
        __bdd out = bdd_ite(bdd_T, bdd_x0, bdd_x1);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().False());
      });

      it("should give back first file with negation flag on if-true (true ? ~x0 : (~x0))", [&]() {
        // Notice, they are equivalent then-and-else cases, but in two
        // different files.
        __bdd out = bdd_ite(bdd_T, bdd_not(bdd_x0), bdd_not_x0);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
        AssertThat(out.negate, Is().True());
      });

      it("should give back second file on if-false (false ? x0 : x1)", [&]() {
        __bdd out = bdd_ite(bdd_F, bdd_x0, bdd_x1);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x1));
        AssertThat(out.negate, Is().False());
      });

      it("should give back second file on if-false (false ? (~x1) : ~x1)", [&]() {
        // Notice, they are equivalent then-and-else cases, but in two
        // different files.
        __bdd out = bdd_ite(bdd_F, bdd_not_x1, bdd_not(bdd_x1));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x1));
        AssertThat(out.negate, Is().True());
      });

      // Trivial inputs with duplicate file inputs
      it("should return 'then' file if 'else' file is the same [1]", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_x1);

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x1));
        AssertThat(out.negate, Is().False());
      });

      it("should return 'then' file if 'else' file is the same [2]", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_not(bdd_x1), bdd_not(bdd_x1));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x1));
        AssertThat(out.negate, Is().True());
      });
    });

    describe("Inputs boiling down to an apply", [&]() {
      it("should create XNOR of x0 and x1 (x0 ? x1 : ~x1) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_not(bdd_x1));

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create XNOR of x0 and ~x1 (x0 ? ~x1 : x1) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_not(bdd_x1), bdd_x1);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create OR of x0 and x1 (x0 ? x0 : x1) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x0, bdd_x1);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create AND of x0 (negated) and x1 (x0 ? ~x0 : x1) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_not(bdd_x0), bdd_x1);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should create AND of x0 and x1 (x0 ? x1 : x0) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_x0);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should create IMPLIES of x0 and x1 (x0 ? x1 : ~x0) due to same file", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_not(bdd_x0));

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create OR of x0 and x1 (x0 ? T : x1)", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_T, bdd_x1);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create AND of x0 (negated) and x1 (x0 ? F : x1)", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_F, bdd_x1);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should create IMPLIES of x0 and x1 (x0 ? x1 : T)", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_T);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should create AND of x0 and x1 (x0 ? x1 : F)", [&]() {
        __bdd out = bdd_ite(bdd_x0, bdd_x1, bdd_F);

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("Inputs that require the cross-product of all three BDDs", [&]() {
      it("should compute x0 ? ~x1 : x1", [&]() {
        /*
                       (x0, ~x1, x1)             ---- x0
                        /         \
                 (F, Nil, x1)  (T, ~x1, Nil)     ---- x1
                  /        \    /         \
                  F        T    T         F

                  The low arc is resolved first, since F < T
        */
        __bdd out = bdd_ite(bdd_x0, bdd_not_x1, bdd_x1);

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

        AssertThat((std::get<__bdd::shared_arcs_t>(out._union))->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should compute x1 ? ~x0 : x0", [&]() {
        /*
                      (x1, ~x0, x0)          ---- x0
                       /         \
                 (x1, T, F)  (x1, F, T)      ---- x1
                  /      \    /      \
                  F      T    T      F

                  The high arc is resolved first, since T > F on the second coordinate
        */
        __bdd out = bdd_ite(bdd_x1, bdd_not_x0, bdd_x0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should compute x1 ? x0 : ~x0", [&]() {
        /*
                       (x1, x0, ~x0)         ---- x0
                        /         \
                 (x1, F, T)  (x1, T, F)      ---- x1
                  /      \    /       \
                  T      F    F       T

                  The low arc is resolved first, since F < T on the second coordinate.
        */
        __bdd out = bdd_ite(bdd_x1, bdd_x0, bdd_not_x0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should compute ~x2 ? (x0^x1) : ~(x0^x1)", [&]() {
        // Create an XNOR input where one needs to forward data across the
        // level once to resolve the request
        shared_levelized_file<bdd::node_t> bdd_x0_xnor_x1;
        {
          node_writer nw_x0_xnor_x1(bdd_x0_xnor_x1);
          nw_x0_xnor_x1 << node(1,1,terminal_T,terminal_F)
                        << node(1,0,terminal_F,terminal_T)
                        << node(0,0,ptr_uint64(1,1),ptr_uint64(1,0));
        }

        /*
                                  ((2,0),(0,0),(0,0))                 ---- x0
                                   /               \
                        ((2,0),(1,0),(1,1))  ((2,0),(1,1),(1,0))      ---- x1
                               /   \              /      \
                              /     \             \ _____/
                              |      \ ____________X
                              |       X________     \
                              |      /         \     \
                             ((2,0),F,T)      ((2,0),T,F)             ---- x2
                                 / \               / \
                                 F T               T F
        */
        __bdd out = bdd_ite(bdd_not(bdd_x2), bdd_x0_xor_x1, bdd_x0_xnor_x1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // ((2,0),(1,0),(1,1))
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // ((2,0),(1,1),(1,0))
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // ((2,0),F,T)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // ((2,0),T,F)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // ((2,0),F,T)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // ((2,0),T,F)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should compute x3 ? (x1 & x2) : bdd_1", [&]() {
        shared_levelized_file<bdd::node_t> bdd_x3;
        /*
                      1     ---- x3
                      / \
                      F T
        */
        {
          node_writer nw_x3(bdd_x3);
          nw_x3 << node(3,0,terminal_F,terminal_T);
        }

        shared_levelized_file<bdd::node_t> bdd_x1_and_x2;
        /*
                      1     ---- x1
                      / \
                      F 2    ---- x2
                      / \
                      F T
        */
        {
          node_writer nw_x1_and_x2(bdd_x1_and_x2);
          nw_x1_and_x2 << node(2,1,terminal_F,terminal_T)
                      << node(1,0,terminal_F,ptr_uint64(2,1));
        }

        /*
                                          (1,1,1)                          ---- x0
                               ____________/   \___________
                              /                            \
                          (1,1,2)                      (1,1,3)            ---- x1
                        ____/   \____                ____/   \____
                      /             \              /             \
                    (1,F,6)       (1,2,5)       (1,F,5)        (1,2,4)     ---- x2
                    /   \         /   \         /   \          /   \
                    F    \       /     \       /     \        /    T
                          \     /       \     /       \_______\
                           \___/________ \ __/                 \
                            \             \                     \
                          (1,F,T)       (1,T,7)              (1,F,7)      ---- x3
                            /   \         /   \                /   \
                            T   F         F   T                F   F

                The drawing above is in-order for all but x2 and x3 where the
                order actually is.

                              (1,2,4), (1,2,5), (1,F,5), (1,F,6)   ---- x2

                                  (1,F,7), (1,F,T), (1,T,7)       ---- x3

                No forwarding across the level is needed due to the ids

                Furthermore notice, the F terminal of (1,F,6) is due to both the
                'then' and the 'else' case agree, so we don't recurse to obtain
                the value of the 'if' conditional. The same goes for T terminal of
                (1,2,4).
        */
        __bdd out = bdd_ite(bdd_x3, bdd_x1_and_x2, bdd_1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,1,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,1,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,2,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,2,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,F,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,F,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,3) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,F,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,F,T)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,1) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), ptr_uint64(3,1) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,3)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,T,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,2) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,2,4)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,F,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,F,7)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,F,T)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,T,7)
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
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,4u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(6u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("should compute bdd_3 ? bdd_4 : bdd_5", [&]() {
        /*
                                      (1,1,1)               ---- x0
                                  ______/ \______
                                 /               \
                             (2,2,2)              \         ---- x1
                          ____/   \____            \
                         /             \            \
                      (3,4,5)        (4,5,3)     (5,3,4)    ---- x2
                        \ /           /   \       /   \
                         X            T   |       F   F
                       _/ \_              |
                      /     \             |
                  (T,6,_) (F,_,6)      (6,7,F)              ---- x3
                   /   \   /   \        /   \
                   T   F   T   F        F   T

                  Forwarding for each node is needed for level x2 twice, but due
                  to the ids involved one will only obtain the nodes in question
                  one at a time.

                  The level for x3 equires to forward (6) in (6,7,F) once.
        */

        __bdd out = bdd_ite(bdd_3, bdd_4, bdd_5);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,5,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,3,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,6,_)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (6,7,F)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,2) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,3,4)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (T,6,_)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (6,7,F)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(3u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(4u));
      });

      it("should compute bdd_6 ? x0^x2 : bdd_not_6", [&]() {
        /*                      bdd_x0_xor_x2
                      _1_       ---- x1
                     /   \
                     2   3      ---- x2
                    / \ / \
                    F T T F
        */

        /*
                                  (1,1,1)               ---- x0
                                    /   \
                                   T  (2,2,3)           ---- x1
                                        /   \
                                  (3,3,3) (4,4,3)       ---- x2
                                    /   \   /   \
                                    T   F   T   T

                  On level x2 forwarding happens for (3,3,3) with the two nodes
                  3, 3 from the if-case and else-case. Both of these are
                  encountered simultaneously, so they are in one-go forwarded.
        */

        __bdd out = bdd_ite(bdd_6, bdd_x0_xor_x2, bdd_not_6);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,1,1)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,3,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,4,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(4u));
      });


      it("should compute bdd_not_6 ? bdd_6 : x0^x2", [&]() {
        /*                      bdd_x0_xor_x2
                      _1_       ---- x1
                     /   \
                     2   3      ---- x2
                    / \ / \
                    F T T F
        */

        /*
                                   (1,1,1)               ---- x0
                                    /   \
                                    F  (2,3,2)           ---- x1
                                        /   \
                                  (3,3,3) (4,3,4)       ---- x2
                                    /   \   /   \
                                    F   F   T   F

                  On level x2 forwarding happens for (3,3,3) with the two nodes
                  3, 3 from the if-case and then-case. Both of these are
                  encountered simultaneously, so they are in one-go forwarded.
        */

        __bdd out = bdd_ite(bdd_not_6, bdd_6, bdd_x0_xor_x2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,3,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (1,1,1)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,3,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,4,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(4u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("should compute ~(x0^x2) ? ~x2 : bdd_1", [&]() {
        shared_levelized_file<bdd::node_t> bdd_x0_xnor_x2;
        /*
                                  _1_
                                 /   \
                                 3   2
                                / \ / \
                                T F F T
        */
        {
          node_writer nw_x0_xnor_x2(bdd_x0_xnor_x2);
          nw_x0_xnor_x2 << node(2,1,terminal_T,terminal_F)                              // 3
                        << node(2,0,terminal_F,terminal_T)                              // 2
                        << node(0,0,ptr_uint64(2,1),ptr_uint64(2,0)); // 1
        }

        /*
                                   (1,1,1)                   ---- x0
                             _______/   \________
                            /                    \
                        (3,1,2)                (2,1,3)       ---- x1
                        /   \                  /   \
                    (3,1,6) (3,1,5)        (2,1,5) (2,1,4)   ---- x2
                    /   \   /   \          /   \   /   \
                    T   T   T    \         T   F  /    F
                                  \______   _____/
                                         \ /
                                        (F,_,7)              ---- x3
                                        /   \
                                        F   T

                Where the order for x2 is:

                        (2,1,4), (2,1,5), (3,1,5), (3,1,6)

                and (2,1,4) is resolved without forwarding, (2,1,5) forwards two
                elements (2 and 1) at once, and finally (3,1,5) and (3,1,6)
                require forwarding two elements one at a time.
        */

        __bdd out = bdd_ite(bdd_x0_xnor_x2, bdd_not(bdd_x2), bdd_1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,1,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,1,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,1,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,1,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,1,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,1,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,3) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (2,1,4)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (2,1,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,1,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,1,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,3)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,7)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,4u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(5u));
      });

      it("should compute (x1^x2) ? bdd_1 : bdd_2", [&]() {
        shared_levelized_file<bdd::node_t> bdd_x1_xor_x2_2;
        /*
                              _1_      ---- x1
                             /   \
                             3   2     ---- x2
                            / \ / \
                            F T T F
        */

        {
          node_writer nw_x1_xor_x2(bdd_x1_xor_x2_2);
          nw_x1_xor_x2 << node(2,1,terminal_F,terminal_T)                              // 3
                      << node(2,0,terminal_T,terminal_F)                              // 2
                      << node(1,0,ptr_uint64(2,1),ptr_uint64(2,0)); // 1
        }

        /*
                                   (1,1,1)                   ---- x0
                              ______/   \_____
                             /                \
                        (1,2,2)             (1,3,3)          ---- x1
                        /   \               /   \
                    (3,6,4) (2,5,5)     (3,5,6) (2,4,7)      ---- x2  Order of resolvement:
                    /   \   /   \       /   \   /   \                    (2,5,5)**, (3,5,6)***, (3,6,4)*, (2,4,7)*
                    F   T   T   |       T    \ /    T                        *   Forwarding two item once
                                |             |                              **  Forwarding one item once
                                |             |                              *** Forwarding one item twice
                              (F,_,9)       (T,7,_)          ---- x3
                              /   \         /   \                    (To reproduce: look at actual ids and sorting)
                              F   T         F   T
        */
        __bdd out = bdd_ite(bdd_x1_xor_x2_2, bdd_1, bdd_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,2,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1,3,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,5,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,5,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,6,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,4,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,3) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,7,_)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,3), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,9)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (2,5,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,5,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,6,4)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (2,4,7)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,3)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (T,7,_)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,9)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(6u));
      });

      it("should compute (~x0 & ~x1 & x2) ? bdd_2 : bdd_4", [&]() {
        shared_levelized_file<bdd::node_t> bdd_if;
        /*
                              1       ---- x0
                             / \
                             2 F      ---- x1
                             / \
                             3 F       ---- x2
                            / \
                            F T
        */

        {
          node_writer nw_if(bdd_if);
          nw_if << node(2,0,terminal_F,terminal_T)                // 3
                << node(1,0,ptr_uint64(2,0),terminal_F)  // 2
                << node(0,0,ptr_uint64(1,0),terminal_F); // 1
        }

        /*
                                        (1,1,1)                  ---- x0
                                 ________/   \
                                /             \
                            (2,2,2)            \                 ---- x1
                            /     \             \
                      (3,4,4)     (F,_,5)     (F,_,3)            ---- x2
                       /   \       /   \       /   \
                  (F,_,6) (T,8,_) F (F,_,7)    T   F             ---- x3
                    /   \   /   \     /   \
                    T   F   T   F     F   T

                  Where the order for x2 is (F,_,3), (3,4,4), (F,_,5) because the
                  (3,4,4) node needs to forward (3,4,_) information once.
        */

        __bdd out = bdd_ite(bdd_if, bdd_2, bdd_4);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,8,_)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), ptr_uint64(3,2) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (T,8,_)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,7)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,2)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,3u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(3u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(4u));
      });

      it("should compute (x0 | (x1 & x2)) ? bdd_8 : bdd_7", [&]() {
        shared_levelized_file<bdd::node_t> bdd_if;
        /*
                                1        ---- x0
                               / \
                               2 T       ---- x1
                              / \
                              F 3        ---- x2
                               / \
                               F T
        */
        {
          node_writer nw_if(bdd_if);
          nw_if << node(2,0,terminal_F,terminal_T)               // 3
                << node(1,0,terminal_F,ptr_uint64(2,0)) // 2
                << node(0,0,ptr_uint64(1,0),terminal_T) // 1
            ;
        }

        /*
                                      (1,1,1)        ---- x0
                              _________/   \
                             /             |
                         (2,2,2)           |         ---- x1
                          /   \            |
                     (F,_,3) (3,4,4)       |         ---- x2
                      /   \   /   \________|
                      F  (F,_,5)        (T,4,_)      ---- x3
                          /   \          /   \
                          F   T          T   F
        */

        __bdd out = bdd_ite(bdd_if, bdd_8, bdd_7);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (T,4,_)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(3,1) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,3)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (T,4,_)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should compute bdd_6 ? bdd_4 : bdd_2", [&]() {
        /*
                                        (1,1,1)                           ---- x0
                               __________/   \___________
                              /                          \
                           (F,_,2)                    (2,3,3)             ---- x1
                            /     \                     /   \
                      (F,_,4)   (F,_,5)           (3,3,6) (4,3,7)         ---- x2
                        /   \     /   \             /   \   /   \
                       F (F,_,8) F (F,_,9)         T   F   T   T          ---- x3
                          /   \     /   \
                          T   F     F   T
        */

        __bdd out = bdd_ite(bdd_6, bdd_4, bdd_2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2,3,3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,4)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3,6)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,2) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3,7)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,3) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,8)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,0)), ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (F,_,9)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(2,1)), ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,4)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,5)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,3,6)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,2)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,3,7)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(2,3)), terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,8)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), terminal_T }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,0)), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (F,_,9)
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), terminal_F }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(3,1)), terminal_T }));

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().EqualTo(4u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(5u));
      });
    });

    describe("Zipping", [&]() {
      it("should merely zip disjunct levels if possible [1]", [&]() {
        shared_levelized_file<bdd::node_t> bdd_x1_and_x3;
        /*
                           1      ---- x1
                          / \
                          F 2     ---- x3
                           / \
                           T F
        */

        {
          node_writer nw_x1_and_x3(bdd_x1_and_x3);
          nw_x1_and_x3 << node(3,42,terminal_F,terminal_T)                // 2
                       << node(1,0,terminal_F,ptr_uint64(3,42)); // 1
        }

        __bdd out = bdd_ite(bdd_x0, bdd_x2, bdd_x1_and_x3);
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->canonical, Is().False());

        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3,42,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2,0,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1,0,
                                                       terminal_F,
                                                       ptr_uint64(3,42))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0,0,
                                                       ptr_uint64(1,0),
                                                       ptr_uint64(2,0))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should merely zip disjunct levels if possible [2]", [&]() {
        shared_levelized_file<bdd::node_t> bdd_then;
        /*
                           _1_      ---- x2
                          /   \
                          2   3     ---- x3
                         / \ / \
                         T 4 T 5    ---- x4
                          / \ / \
                          F T T 6   ---- x6
                               / \
                               T F
        */

        {
          node_writer nw_then(bdd_then);
          nw_then << node(6,1,terminal_T,terminal_F)                              // 6
                  << node(4,1,terminal_T,ptr_uint64(6,1))                // 5
                  << node(4,0,terminal_F,terminal_T)                              // 4
                  << node(3,2,terminal_T,ptr_uint64(4,1))                // 3
                  << node(3,0,terminal_T,ptr_uint64(4,0))                // 2
                  << node(2,0,ptr_uint64(3,0),ptr_uint64(3,2)); // 1
        }

        shared_levelized_file<bdd::node_t> bdd_else;
        /*
                          _1_      ---- x5
                         /   \
                         2   3     ---- x8
                        / \ / \
                        T F F T
        */

        {
          node_writer nw_else(bdd_else);
          nw_else << node(8,1,terminal_T,terminal_F)                             // 3
                  << node(8,0,terminal_F,terminal_T)                             // 2
                  << node(5,0,ptr_uint64(8,0),ptr_uint64(8,1)) // 1
            ;
        }

        __bdd out = bdd_ite(bdd_not(bdd_x0_xor_x1), bdd_then, bdd_else);
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->canonical, Is().False());

        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8,1,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8,0,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6,1,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5,0,
                                                       ptr_uint64(8,0),
                                                       ptr_uint64(8,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4,1,
                                                       terminal_T,
                                                       ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4,0,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3,2,
                                                       terminal_T,
                                                       ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3,0,
                                                       terminal_T,
                                                       ptr_uint64(4,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2,0,
                                                       ptr_uint64(3,0),
                                                       ptr_uint64(3,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1,1,
                                                       ptr_uint64(5,0),
                                                       ptr_uint64(2,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1,0,
                                                       ptr_uint64(2,0),
                                                       ptr_uint64(5,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0,0,
                                                       ptr_uint64(1,0),
                                                       ptr_uint64(1,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(8,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(6,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(7u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(11u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(7u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(11u));

        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[0], Is().EqualTo(4u));
        AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[1], Is().EqualTo(7u));
      });

      it("can derive canonicity when zipping with one-node 'if'", [&]() {
        shared_levelized_file<bdd::node_t> bdd_if;
        {
          node_writer nw_if(bdd_if);
          nw_if << node(0, node::MAX_ID,terminal_T,terminal_F);
        }

        shared_levelized_file<bdd::node_t> bdd_a;
        {
          node_writer nw_a(bdd_a);
          nw_a << node(2, node::MAX_ID,terminal_F,terminal_T);
        }

        shared_levelized_file<bdd::node_t> bdd_b;
        {
          node_writer nw_b(bdd_b);
          nw_b << node(1, node::MAX_ID,terminal_T,terminal_F);
        }

        bdd out_1 = bdd_ite(bdd_if, bdd_a, bdd_b);
        AssertThat(adiar::is_canonical(out_1), Is().True());

        bdd out_1n = bdd_ite(bdd_not(bdd_if), bdd_a, bdd_b);
        AssertThat(adiar::is_canonical(out_1n), Is().True());

        bdd out_2 = bdd_ite(bdd_if, bdd_b, bdd_a);
        AssertThat(adiar::is_canonical(out_2), Is().True());

        bdd out_2n = bdd_ite(bdd_not(bdd_if), bdd_b, bdd_a);
        AssertThat(adiar::is_canonical(out_2n), Is().True());
      });


      it("can derive canonicity when zipping negated 'then' or 'else'", [&]() {
        shared_levelized_file<bdd::node_t> bdd_if;
        {
          node_writer nw_if(bdd_if);
          nw_if << node(0, node::MAX_ID,terminal_T,terminal_F);
        }
        AssertThat(adiar::is_canonical(bdd(bdd_if)), Is().True());

        shared_levelized_file<bdd::node_t> bdd_a;
        {
          node_writer nw_a(bdd_a);
          nw_a << node(2, node::MAX_ID,terminal_F,terminal_T);
        }
        AssertThat(adiar::is_canonical(bdd(bdd_a)), Is().True());

        shared_levelized_file<bdd::node_t> bdd_b;
        {
          node_writer nw_b(bdd_b);
          nw_b << node(3, node::MAX_ID,   terminal_F, terminal_T)
              << node(3, node::MAX_ID-1, terminal_T, terminal_F)
              << node(1, node::MAX_ID,   ptr_uint64(3, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID));
        }
        AssertThat(adiar::is_canonical(bdd(bdd_b)), Is().True());

        shared_levelized_file<bdd::node_t> bdd_c;
        {
          node_writer nw_c(bdd_c);
          nw_c << node(1, node::MAX_ID, terminal_T, terminal_F);
        }
        AssertThat(adiar::is_canonical(bdd(bdd_c)), Is().True());

        bdd out_1 = bdd_ite(bdd_if, bdd_not(bdd_a), bdd_b);
        AssertThat(adiar::is_canonical(out_1), Is().True());

        bdd out_2 = bdd_ite(bdd_if, bdd_a, bdd_not(bdd_b));
        AssertThat(adiar::is_canonical(out_2), Is().False());

        bdd out_3 = bdd_ite(bdd_if, bdd_a, bdd_not(bdd_c));
        AssertThat(adiar::is_canonical(out_3), Is().True());

        bdd out_4 = bdd_ite(bdd_if, bdd_not(bdd_b), bdd_not(bdd_a));
        AssertThat(adiar::is_canonical(out_4), Is().False());
      });

      it("can derive canonicity when zipping 'if' with multiple nodes on a level", [&]() {
        shared_levelized_file<bdd::node_t> bdd_if;
        {
          node_writer nw_if(bdd_if);
          nw_if << node(1, node::MAX_ID,   terminal_T,                      terminal_T)
                << node(1, node::MAX_ID-1, terminal_F,                      terminal_F)
                << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID-1), ptr_uint64(1, ptr_uint64::MAX_ID));
        }

        shared_levelized_file<bdd::node_t> bdd_a;
        {
          node_writer nw_a(bdd_a);
          nw_a << node(3, node::MAX_ID,terminal_F,terminal_T);
        }

        shared_levelized_file<bdd::node_t> bdd_b;
        {
          node_writer nw_b(bdd_b);
          nw_b << node(2, node::MAX_ID,terminal_T,terminal_F);
        }

        bdd out_1 = bdd_ite(bdd_if, bdd_a, bdd_b);
        AssertThat(adiar::is_canonical(out_1), Is().True());

        bdd out_2 = bdd_ite(bdd_not(bdd_if), bdd_a, bdd_b);
        AssertThat(adiar::is_canonical(out_2), Is().False());

        bdd out_3 = bdd_ite(bdd_not(bdd_if), bdd_b, bdd_a);
        AssertThat(adiar::is_canonical(out_3), Is().True());
      });

      it("should not zip if bdd_then is not beyond max_label of bdd_if", [&]() {
        __bdd out = bdd_ite(bdd_x1, bdd_x0, bdd_x2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), terminal_F }));

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("should not zip if bdd_else is not beyond max_label of bdd_if", [&]() {
        __bdd out = bdd_ite(bdd_x1, bdd_x2, bdd_x0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(0,0)), ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,0)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(ptr_uint64(1,1)), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), terminal_T }));

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

        AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[1], Is().EqualTo(2u));
      });
    });
  });
 });
