#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/apply.cpp", []() {
    // Setup shared bdd's
    shared_levelized_file<bdd::node_t> bdd_F;
    shared_levelized_file<bdd::node_t> bdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(bdd_F);
      nw_F << node(false);

      node_writer nw_T(bdd_T);
      nw_T << node(true);
    }

    // TODO: all of these may not be shared
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
    //        1        ---- x0
    //       / \
    //       | 2       ---- x1
    //       |/ \
    //       3   4     ---- x2
    //      / \ / \
    //      F T T 5    ---- x3
    //           / \
    //           F T
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
    //               ---- x0
    //
    //         1     ---- x1
    //        / \
    //        | T    ---- x2
    //        |
    //        2      ---- x3
    //       / \
    //       T F
    */

    node n2_2 = node(3, node::MAX_ID, terminal_T, terminal_F);
    node n2_1 = node(1, node::MAX_ID, n2_2.uid(), terminal_T);

    { // Garbage collect early and free write-lock
      node_writer nw_2(bdd_2);
      nw_2 << n2_2 << n2_1;
    }

    shared_levelized_file<bdd::node_t> bdd_3;
    /*
    //            1         ---- x0
    //           / \
    //           2 3        ---- x1
    //         _/ X \_
    //        | _/ \_ |
    //         X     X
    //        / \   / \
    //       4  5  6  7     ---- x2
    //      / \/ \/ \/ \
    //      F T  8  T  F    ---- x3
    //          / \
    //          F T
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

    // TODO: should these be defined here?
    shared_levelized_file<bdd::node_t> bdd_0xnor2;
    /*
    //           1     ---- x0
    //          / \
    //         /   \   ---- x1
    //         |   |
    //         2   3   ---- x2
    //        / \ / \
    //        T F F T
    */

    node nx_3 = node(2, node::MAX_ID, terminal_F, terminal_T);
    node nx_2 = node(2, node::MAX_ID - 1, terminal_T, terminal_F);
    node nx_1 = node(0, node::MAX_ID, nx_2, nx_3);

    { // Garbage collect early and free write-lock
      node_writer nw_x(bdd_0xnor2);
      nw_x << nx_3 << nx_2 << nx_1;
    }

    // Some are shared, and some are not - easier to find with all in one location - cluttering maybe though?
    shared_levelized_file<bdd::node_t> bdd_thin;
    /*
    //                  1          ---- x0
    //                 / \
    //                2   3        ---- x1
    //                X_ / \
    //               /  X   \
    //               |_/ \__|
    //               4      5      ---- x2
    //              / \    / \
    //              T F    F T
    */

    node nt_5 = node(2, node::MAX_ID, terminal_F, terminal_T);
    node nt_4 = node(2, node::MAX_ID - 1, terminal_T, terminal_F);
    node nt_3 = node(1, node::MAX_ID, nt_4, nt_5);
    node nt_2 = node(1, node::MAX_ID - 1, nt_5, nt_4);
    node nt_1 = node(0, node::MAX_ID, nt_2, nt_3);

    { // Garbage collect early and free write-lock
      node_writer nw_t(bdd_thin);
      nw_t << nt_5 << nt_4 << nt_3 << nt_2 << nt_1;
    }

    AssertThat(bdd_thin->canonical, Is().EqualTo(true));
    AssertThat(bdd_thin->width, Is().EqualTo(2u));

    shared_levelized_file<bdd::node_t> bdd_wide;
    /*
    //                1         ---- x0
    //               / \
    //              2   3       ---- x1
    //              X   X
    //             / \ / \
    //            4   5   6     ---- x2
    //           / \ / \ / \
    //           F 7 T F F T    ---- x3
    //            / \
    //            F T
    */

    node nw_7 = node(3, node::MAX_ID, terminal_F, terminal_T);
    node nw_6 = node(2, node::MAX_ID, terminal_F, terminal_T);
    node nw_5 = node(2, node::MAX_ID - 1, terminal_T, terminal_F);
    node nw_4 = node(2, node::MAX_ID - 2, terminal_F, nw_7);
    node nw_3 = node(1, node::MAX_ID, nw_6, nw_5);
    node nw_2 = node(1, node::MAX_ID - 1, nw_5, nw_4);
    node nw_1 = node(0, node::MAX_ID, nw_2, nw_3);

    { // Garbage collect early and free write-lock
      node_writer nw_w(bdd_wide);
      nw_w << nw_7 << nw_6 << nw_5 << nw_4 << nw_3 << nw_2 << nw_1;
    }

    // bdd_wide->canonical == true
    // bdd_wide->width == 3u

    shared_levelized_file<bdd::node_t> bdd_canon;
    /*
    //                 1              ---- x0
    //                / \
    //               2   3            ---- x1
    //           ___/ \ / \___
    //          /    __X__    \
    //         4    5     6    7      ---- x2
    //        / \  / \   / \  / \
    //        F 8  F 9   T F  F T     ---- x3
    //         / \  / \
    //         T F  F T
    */

    node nc_9 = node(3, node::MAX_ID, terminal_F, terminal_T);
    node nc_8 = node(3, node::MAX_ID - 1, terminal_T, terminal_F);
    node nc_7 = node(2, node::MAX_ID, terminal_F, terminal_T);
    node nc_6 = node(2, node::MAX_ID - 1, terminal_T, terminal_F);
    node nc_5 = node(2, node::MAX_ID - 2, terminal_F, nc_9);
    node nc_4 = node(2, node::MAX_ID - 3, terminal_F, nc_8);
    node nc_3 = node(1, node::MAX_ID, nc_5, nc_7);
    node nc_2 = node(1, node::MAX_ID - 1, nc_4, nc_6);
    node nc_1 = node(0, node::MAX_ID, nc_2, nc_3);

    { // Garbage collect early and free write-lock
      node_writer nw_c(bdd_canon);
      nw_c << nc_9 << nc_8 << nc_7 << nc_6 << nc_5 << nc_4 << nc_3 << nc_2 << nc_1;
    }

    // bdd_canon->canonical == true
    // bdd_canon->width == 4u

    shared_levelized_file<bdd::node_t> bdd_non_canon;
    /*
    //                1            ---- x0
    //               / \
    //              2   3          ---- x1
    //             / \ _X_
    //             |__X   \
    //             4   5   6       ---- x2
    //            / \ / \ / \
    //            T F 7 F F T      ---- x3
    //               / \
    //               T F
    */

    node nn_7 = node(3, node::MAX_ID, terminal_T, terminal_F);
    node nn_6 = node(2, node::MAX_ID, terminal_F, terminal_T);
    node nn_5 = node(2, node::MAX_ID - 1, nn_7, terminal_F);
    node nn_4 = node(2, node::MAX_ID - 2, terminal_T, terminal_F);
    node nn_3 = node(1, node::MAX_ID, nn_6, nn_4);
    node nn_2 = node(1, node::MAX_ID - 1, nn_4, nn_5);
    node nn_1 = node(0, node::MAX_ID, nn_2, nn_3);

    { // Garbage collect early and free write-lock
      node_writer nw_n(bdd_non_canon);
      nw_n << nn_7 << nn_6 << nn_5 << nn_4 << nn_3 << nn_2 << nn_1;
    }

    // bdd_non_canon->canonical == false
    // bdd_non_canon->width == 3u

    describe("simple cases without access mode requirements", [&]() {
      // Cases with the same file (same DAG) or at least one terminal does not need to run complicated algorithm

      describe("bdd_and(f,g)", [&]() {
        it("should resolve F /\\ T terminal-only BDDs", [&]() {
          __bdd out = bdd_and(bdd_F, bdd_T);
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("should shortcut F /\\ [2]", [&]() {
          __bdd out = bdd_and(bdd_F, bdd_2);

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("should return input on being given the same BDD twice", [&]() {
          __bdd out = bdd_and(bdd_1, bdd_1);

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_1));
          AssertThat(out.negate, Is().False());
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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

        it("should shortcut [1] \\/ T", [&]() {
          __bdd out = bdd_or(bdd_1, bdd_T);

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

        it("should collapse on the same BDD twice", [&]() {
          __bdd out = bdd_xor(bdd_1, bdd_1);

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });
      });

      describe("bdd_imp(f,g)", [&]() {
        it("should resolve F -> T terminal-only BDDs", [&]() {
          __bdd out = bdd_imp(bdd_F, bdd_T);
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should shortcut on irrelevance on T -> x0", [&]() {
          __bdd out = bdd_imp(bdd_T, bdd_x0);

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd_x0));
          AssertThat(out.negate, Is().False());
        });

        it("should shortcut F -> [1]", [&]() {
          __bdd out = bdd_imp(bdd_F, bdd_1);

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

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
    });

    describe("access mode: priority queues", [&]() {
      // Set access mode to priority queues for this batch of tests
      access_mode = access_mode_t::Priority_Queue;

      /* The product construction of bbd_1 and bdd_2 above is as follows in sorted order.
      //
      //                                        (1,1)                       ---- x0
      //                                        \_ _/
      //                                         _X_                        // Match in first, but not coordinatewise
      //                                        /   \
      //                                    (3,1)   (2,1)                   ---- x1
      //                                   /    \_ _/    \
      //                                  /       X       \
      //                                 /_______/ \       \
      //                                 |          \       \
      //                             (3,2)          (3,T)   (4,T)           ---- x2
      //                              \ \           /   \   /   \
      //                               \ \      (F,T)   (T,T)   /
      //                                \ \________ ___________/
      //                                 \________ X________
      //                                          X_________\ _______
      //                                         /           \       \
      //                                        /             \       \
      //                                     (5,T)         (F,2)     (T,2)   ---- x3
      //                                     /   \         /   \     /   \
      //                                  (F,T) (T,T)   (F,T)(F,F)  (T,T)(T,F)
      */

      describe("bdd_and(f,g)", [&]() {
        it("should x0 and !x0", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F F
          */

          __bdd out = bdd_and(bdd_x0, bdd_not_x0);

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

          AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(0u));
        });

        it("should compute (and shortcut) BBD 1 /\\ [2]", [&]() {
          /*
          //                    1                        ---- x0
          //                    X
          //                   / \
          //                  2   3                      ---- x1
          //                 / \ / \
          //                /   X   \
          //               /___/ \   \
          //              /      |    \
          //             4       5     6                 ---- x2
          //            / \     / \_ _/ \
          //            F 7     F   T   8                ---- x3
          //             / \           / \
          //             T F           F T
          */

          __bdd out = bdd_and(bdd_1, bdd_2);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,1) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,2) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), true,  ptr_uint64(3,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_F }));

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

        it("should group all recursion requests together", [&]() {
          // This is a counter-example to the prior "break ties on first() with
          // second()" approach. Here we will have three requests to the level of
          // x2, but in the following order:
          //
          // [((2,0),(2,1)), ((2,1),(2,0)), ((2,0),(2,1))]
          //
          // which all are tied, and hence the prior version would create
          // three nodes on this level rather than just two.

          // TODO: This drawing seems to not fit - fix it?

          /*
          //         1    ---- x0
          //        / \
          //        2 |   ---- x1
          //       / \|
          //       3  4   ---- x2
          //      / \/ \
          //      T  F T
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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5) i.e. the added node
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

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

          AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(3u));

          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("bdd_nand(f,g)", [&]() {
        // TODO
      });

      describe("bdd_or(f,g)", [&]() {
        // TODO: this one too?
        it("should shortcut on x0 \\/ x2", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          | T
          //          |
          //          2      ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(bdd_x0, bdd_x2);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

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

          AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("should compute (and shortcut) [1] \\/ [2]", [&]() {
          /*
          //                   1       ---- x0
          //                  / \
          //                 2   3     ---- x1
          //                / \ / \
          //                | T | T
          //                \_ _/
          //                  4        ---- x2
          //                 / \
          //                 5  T      ---- x3
          //                / \
          //                T F
          */

          __bdd out = bdd_or(bdd_1, bdd_2);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

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
        // TODO
      });

      describe("bdd_xor(f,g)", [&]() {
        it("should compute x0 ^ x1", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          //   after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(bdd_x0, bdd_x1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

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
          //                                 ---- x0
          //
          //                 (1,1)           ---- x1
          //                 /   \
          //             (2,1)   (T,1)       ---- x2
          //             /   \   /   \
          //            /     \  T   F
          //            |     |
          //        (2,F)     (2,T)          ---- x3
          //        /   \     /   \
          //        T   F     F   T
          */
          __bdd out = bdd_xor(bdd_2, bdd_x2);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_T }));

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
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), true,  ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true,  terminal_T }));

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
          /* The queue pq_2 is used to forward data across the level. When
          // [1] and 3 are combined, this is needed
          //
          //          The product between the [3] and [1] then is
          //
          //                           (1,1)                      ---- x0
          //                   ________/   \_______
          //                  /                    \
          //                (2,3)                  (3,2)          ---- x1
          //                /   \_________ ________/   \
          //                |             X            |          //      (5,3) (7,3) (4,3) (6,4)
          //                \__ _________/ \__________ /          // min:   0     0     0     1
          //                ___X___                   X           // max:   1     3     0     2
          //               /       \            _____/ \          // coord: 2     3     1     4
          //              /         \          /        \
          //           (4,3)       (5,3)    (6,4)     (7,3)       ---- x2
          //           /   \       /   \    /   \     /   \
          //        (F,F) (T,T) (T,F)  |   /     \  (T,F) (F,T)
          //                           |  /       \
          //                           | /        |
          //                           |/         |
          //                         (8,T)      (T,5)             ---- x3
          //                         /   \      /   \
          //                      (F,T) (T,T) (T,F) (T,T)
          */

          __bdd out = bdd_xor(bdd_3, bdd_1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,3)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,2)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,3)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,4)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(2,2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,3)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), false, ptr_uint64(3,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5)
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_F }));

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

        it("should compute in different order than random access", [&]() {
            /*
            // Result of [canon] ^ [non_canon]
            //
            //                         (1,1)                         ---- x0
            //                   ______/   \______
            //                  /                 \
            //              (2,2)                 (3,3)              ---- x1
            //             _/   \_               _/   \_
            //            /       \             /       \
            //        (4,4)       (6,5)     (5,6)       (7,4)        ---- x2
            //        /   \       /   \     /   \       /   \
            //    (F,T)    |     |     (F,F)     |  (F,T)   (T,F)
            //             /      \______ ______/
            //            /              X
            //           /              / \________
            //        (8,F)         (9,T)         (T,7)              ---- x3
            //        /   \         /   \         /   \
            //     (T,F) (F,F)   (F,T) (T,T)   (T,T) (T,F)
            */

            __bdd out = bdd_xor(bdd_canon, bdd_non_canon);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,3) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), true, ptr_uint64(3,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(3,2) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_F }));

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

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(5u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(6u));
        });
      });

      describe("bdd_xnor(f,g)", [&]() {
        // TODO
      });

      describe("bdd_imp(f,g)", [&]() {
        it("should shortcut on x0 -> x1", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          // after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //          T 2    ---- x1
          //           / \
          //           F T
          */

          __bdd out = bdd_imp(bdd_x0, bdd_x1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_T }));

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

          AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
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

      // Reset access mode
      access_mode = access_mode_t::Auto;
    });

    describe("access mode: random access", [&]() {
      // Set access mode to random access for this batch of tests
      access_mode = access_mode_t::Random_Access;

      // Trivial canonical: bdd_F, bdd_T, bdd_x0, bdd_not_x0, bdd_x1, bdd_x2
      // Canonical:     bdd_2
      // Non-Canonical: bdd_1, bdd_3

      describe("bdd_and(f,g)", [&]() {
        it("should compute (and shortcut) [1] /\\ [2]", [&]() {
          /*
          //                    1                        ---- x0
          //                    X
          //                   / \
          //                  2   3                      ---- x1
          //                 / \ / \
          //                /   X   \
          //               /___/ \   \
          //              /      |    \
          //             4       5     6                 ---- x2
          //            / \     / \_ _/ \
          //            F 7     F   T   8                ---- x3
          //             / \           / \
          //             T F           F T
          */

          __bdd out = bdd_and(bdd_1, bdd_2);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,1) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,2) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,2), true,  ptr_uint64(3,0) }));

          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true,  terminal_F }));

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
      });

      describe("bdd_xor(f,g)", [&]() {
        it("should compute x0 ^ !x0 (same level)", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //         T   T
          */

          __bdd out = bdd_xor(bdd_x0, bdd_not_x0);

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

        it("should compute x0 ^ x1 (different levels, random access for first level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          //   after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(bdd_x0, bdd_x1);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

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

        it("should compute x1 ^ x0 (different levels, random access for second level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          //   after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(bdd_x1, bdd_x0);

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

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

        it("handles requests where scanning index is on a later level", [&]() {
          shared_levelized_file<bdd::node_t> bdd_ra;
          /*
          //         1       ---- x0
          //        / \
          //        F 2      ---- x1
          //         / \
          //         F 3     ---- x2
          //          / \
          //          F T
          */

          node nra_3 = node(2, node::MAX_ID, terminal_F, terminal_T);
          node nra_2 = node(1, node::MAX_ID, terminal_F, nra_3);
          node nra_1 = node(0, node::MAX_ID, terminal_F, nra_2);

          { // Garbage collect early and free write-lock
            node_writer nw_ra(bdd_ra);
            nw_ra << nra_3 << nra_2 << nra_1;
          }

          // Since 'bdd_ra' is the thinnest, random access will be run on this one.
          //
          // If one always scans ahead to the requests scanning index (which is
          // reasonable to do due to the comparator), then one gets a bug in the
          // following edge case: a request (scan, ra) has only 'ra' be on the
          // current level. That is, if going to 'scan' in the input then you
          // will unintentionally jump over some nodes.
          /*
          //          (1,1)           ---- x0
          //          /   \
          //         /   (3,2)        ---- x1
          //        /     __X__
          //       /     /     \
          //     (2,F) (3,2)  (3,F)   ---- x2
          //      / \   / \    / \
          //      T F   F F    F T
           */

          __bdd out = bdd_xor(bdd_0xnor2, bdd_ra);


          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true,  terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<arc>>()->max_1level_cut, Is().EqualTo(3u));

          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("should sort requests first by scanning index then by random acces index", [&]() {
            shared_levelized_file<bdd::node_t> bdd_wide2;
            /*
            //           1          ---- x0
            //          / \
            //         2   3        ---- x1
            //        / \ / \
            //       /   X   \__
            //       |__/ \     \
            //       4     5     6  ---- x2
            //      / \   / \   / \
            //      F  7  T F   F T ---- x3
            //        / \
            //        F T
            */

            node nw_7 = node(3, node::MAX_ID, terminal_F, terminal_T);
            node nw_6 = node(2, node::MAX_ID, terminal_F, terminal_T);
            node nw_5 = node(2, node::MAX_ID - 1, terminal_T, terminal_F);
            node nw_4 = node(2, node::MAX_ID - 2, terminal_F, nw_7);
            node nw_3 = node(1, node::MAX_ID, nw_4, nw_6);
            node nw_2 = node(1, node::MAX_ID - 1, nw_4, nw_5);
            node nw_1 = node(0, node::MAX_ID, nw_2, nw_3);

            { // Garbage collect early and free write-lock
              node_writer nw_w(bdd_wide2);
              nw_w << nw_7 << nw_6 << nw_5 << nw_4 << nw_3 << nw_2 << nw_1;
            }

            AssertThat(bdd_wide->canonical, Is().True());
            AssertThat(bdd_wide->width, Is().EqualTo(3u));

            /*
            // Result of [0xnor2] ^ [wide2]
            //
            // Level x2 is not in the same order as pq, as (3,4) and (2,5) are in different order
            //
            //                       (1,1)               ---- x0
            //                      /     \
            //                    (2,2) (3,3)            ---- x1
            //                 ___/   \ /   \___
            //                /        X___     \
            //               /        /    \     \
            //           (2,4)     (3,4)   (2,5) (3,6)   ---- x2
            //           /   |     /   |    | |   | |
            //          T  (F,7)  F  (T,7)  F F   F F    ---- x3
            //              / \       / \
            //             F   T     T   F
            */

            __bdd out = bdd_xor(bdd_0xnor2, bdd_wide2);

            arc_test_stream arcs(out);

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true, ptr_uint64(1,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,1) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true, ptr_uint64(2,2) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true, ptr_uint64(2,3) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true, ptr_uint64(3,0) }));

            AssertThat(arcs.can_pull_internal(), Is().True());
            AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,1), true, ptr_uint64(3,1) }));

            AssertThat(arcs.can_pull_internal(), Is().False());

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_F }));

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

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(7u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(3u));
        });

        /*  TODO: Same is used in imp, move one level out?
        // Result of [thin] ^ [wide]
        //
        //                      (1,1)                   ---- x0
        //                      /   \
        //                  (2,2)   (3,3)               ---- x1
        //                    X       X
        //              _____/ \     / \_____
        //          (4,4)       (5,5)       (4,6)       ---- x2
        //          /   \       /   \       /   \
        //       (T,F) (F,7) (F,T) (T,F) (T,F) (F,T)    ---- x3
        //             /   \
        //          (F,F) (F,T)
        */

        it("should random access on the thinnest ([thin] ^ [wide])", [&]() {
            __bdd out = bdd_xor(bdd_thin, bdd_wide);

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
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

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

        it("should random access on the thinnest ([wide] ^ [thin])", [&]() {
            __bdd out = bdd_xor(bdd_wide, bdd_thin);

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
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

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

        /*
        // Result of [canon] ^ [non_canon]
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
        //    (F,T) (8,F) (F,T) (T,F) (T,7) (F,F) (F,F) (9,T)         ---- x3
        //          /   \             /   \             /   \
        //      (T,F)   (F,F)     (T,T)   (T,F)     (F,T)   (T,T)
        */

        it("should random access on canonical ([canon] ^ [non_canon])", [&]() {
            __bdd out = bdd_xor(bdd_canon, bdd_non_canon);

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
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true, terminal_F }));

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

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(5u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(6u));
        });

        it("should random access on canonical ([non_canon] ^ [canon])", [&]() {
            __bdd out = bdd_xor(bdd_non_canon, bdd_canon);

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
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,3), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,1), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,2), true, terminal_F }));

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

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(5u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(6u));
        });
      });

      describe("bdd_imp(f,g)", [&]() {
        // TODO: smaller example?
        it("should flip non-commutative operator", [&]() {
            // As the first input is wider than the second, the input order is swapped
            // Therefore the operator should be flipped to achieve the same behavior
            // This can be seen, as implication is a non-commutative operator

            __bdd out = bdd_imp(bdd_wide, bdd_thin);

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
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,2), true, terminal_F }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

            AssertThat(arcs.can_pull_terminal(), Is().True());
            AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true, terminal_F }));

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

            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[false], Is().EqualTo(3u));
            AssertThat(out.get<shared_levelized_file<arc>>()->number_of_terminals[true],  Is().EqualTo(4u));
        });
      });

      // Reset access mode
      access_mode = access_mode_t::Auto;
    });
  });
 });
