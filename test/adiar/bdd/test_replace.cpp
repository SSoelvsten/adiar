#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/replace.cpp", []() {
    using mapping_type = function<bdd::label_type(bdd::label_type)>;

    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //        F
    */
    { // Garbage collect writers to free write-lock
      node_writer nw(bdd_F);
      nw << node(false);
    }

    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //        T
    */
    { // Garbage collect writers to free write-lock
      node_writer nw(bdd_T);
      nw << node(true);
    }

    const ptr_uint64 terminal_T = ptr_uint64(true);
    const ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_type> bdd_x0;
    /*
    //          1        ---- x0
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_writer nw(bdd_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_x1;
    /*
    //          1        ---- x1
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_writer nw(bdd_x1);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_x2;
    /*
    //          1        ---- x2
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_writer nw(bdd_x2);
      nw << node(2, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_1;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  T      ---- x4
    //      / \
    //      F T
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(4, node::max_id, terminal_F, terminal_T);
      const node n2 = node(2, node::max_id, n3.uid(), terminal_T);
      const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

      node_writer nw(bdd_1);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_2;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not).
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     F T T F
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(1, node::max_id,     terminal_T, terminal_F);
      const node n2 = node(1, node::max_id - 2, terminal_F, terminal_T);
      const node n1 = node(0, node::max_id,     n2.uid(), n3.uid());

      node_writer nw(bdd_2);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_3;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not)
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     |  F  |
    //      \   /
    //       \ /
    //        4         ---- x2
    //       / \
    //       T F
    */

    { // Garbage collect early and free write-lock
      const node n4 = node(2, node::max_id,     terminal_T, terminal_F);
      const node n3 = node(1, node::max_id,     terminal_F, n4.uid());
      const node n2 = node(1, node::max_id - 1, n4.uid(), terminal_F);
      const node n1 = node(0, node::max_id,     n2.uid(), n3.uid());

      node_writer nw(bdd_3);
      nw << n4 << n3 << n2 << n1;
    }

    describe("bdd_replace(const bdd&, <...>)", [&]() {
      describe("<non-monotonic>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          const bdd out = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          const bdd out = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          const bdd out = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().True());
        });

        it("returns 'x(4-0)' when given 'x0'", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          const bdd out = bdd_replace(bdd_x0, m);

          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(bdd_iscanonical(out), Is().True());

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_x0->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_x0->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_x0->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_x0->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_x0->number_of_terminals[true]));
        });

        it("throws exception if levels have to be swapped [bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          AssertThrows(invalid_argument, bdd_replace(bdd_1, m));
        });

        it("throws exception if levels have to be swapped [bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 4-x; };
          AssertThrows(invalid_argument, bdd_replace(bdd_2, m));
        });

        // TODO: Add more complex inputs that test for all relevant behaviours of applying the
        //       Nested Sweeping framework to move levels.
      });

      describe("<monotonic>", [&]() {
        // NOTE: To future-proof these tests against the introduction of constant time 'Affine' or
        //       'Shift' variable replacement, we test with quadratic variable replacement (and
        //       similarly more complex functions).

        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().True());
        });

        it("squares all variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_1, m);

          AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(16, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4,
                                                         bdd::max_id,
                                                         bdd::pointer_type(16, bdd::max_id),
                                                         terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,
                                                         bdd::max_id,
                                                         bdd::pointer_type(16, bdd::max_id),
                                                         bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(16, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_1->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_1->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_1->number_of_terminals[true]));
        });

        it("bakes negation into output when squaring of variables in 'BDD 3'", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_not(bdd_3), m);

          AssertThat(out->sorted, Is().EqualTo(bdd_3->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_3->indexable));

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1,
                                                         bdd::max_id,
                                                         terminal_T,
                                                         bdd::pointer_type(4, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1,
                                                         bdd::max_id - 1,
                                                         bdd::pointer_type(4, bdd::max_id),
                                                         terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,
                                                         bdd::max_id,
                                                         bdd::pointer_type(1, bdd::max_id - 1),
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_3->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_3->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_3->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_3->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_3->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_3->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_3->number_of_terminals[false]));
        });
      });

      describe("<affine>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 2*x; };
          const bdd out = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return 2*x+2; };
          const bdd out = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return 2*x+1; };
          const bdd out = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
          AssertThat(out.is_negated(), Is().True());
        });

        it("shifts 'x0' into 'x1'", [&]() {
          const mapping_type m = [](const int x) { return 2*x+1; };
          const bdd out = bdd_replace(bdd_x0, m);

          AssertThat(out->sorted, Is().EqualTo(bdd_x0->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_x0->indexable));

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_x0->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_x0->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_x0->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_x0->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_x0->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_x0->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_x0->number_of_terminals[false]));
        });

        it("shifts variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x+1; };
          const bdd out = bdd_replace(bdd_1, m);

          AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1,
                                                         bdd::max_id,
                                                         bdd::pointer_type(5, bdd::max_id),
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_1->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_1->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_1->number_of_terminals[true]));
        });

        it("doubles variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return 2*x; };
          const bdd out = bdd_replace(bdd_2, m);

          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,
                                                         bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id - 2),
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_2->number_of_terminals[true]));
        });
      });

      describe("<identity>", [&]() {
        it("returns the original file for 'x0'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out = bdd_replace(bdd_x0, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'x1'", [&]() {
          const mapping_type m = [](const int x) { return x*x; };
          const bdd out = bdd_replace(bdd_x1, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x1));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out = bdd_replace(bdd_1, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out = bdd_replace(bdd_2, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2));
          AssertThat(out.is_negated(), Is().False());
        });
      });
    });

    describe("bdd_replace(const bdd&, <...>, replace_type)", [&]() {
      it("returns the original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return 4-x; };
        const bdd out = bdd_replace(bdd_F, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return 4-x; };
        const bdd out = bdd_replace(bdd_T, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
        AssertThat(out.is_negated(), Is().False());
      });

      // TODO: Replace AssertThrows tests with checks for the output is correct (and reduced because
      //       it uses the most general algorithm).

      it("throws exception if 'replace_type' is 'Non_Monotone' [x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x+1; };

        AssertThrows(invalid_argument, bdd_replace(bdd_x0, m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4-x; };
        AssertThrows(invalid_argument, bdd_replace(bdd_1, m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_2]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x+1; };

        AssertThrows(invalid_argument, bdd_replace(bdd_2, m, replace_type::Non_Monotone));
      });

      it("shifts variables in 'BDD 1' if 'replace_type' is 'Monotone'", [&]() {
        const mapping_type m = [](const int x) { return x+1; };
        const bdd out = bdd_replace(bdd_1, m, replace_type::Monotone);

        AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
        AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,
                                                       bdd::max_id,
                                                       bdd::pointer_type(5, bdd::max_id),
                                                       bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(bdd_1->width));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_1->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_1->number_of_terminals[true]));
      });

      it("doubles variables in 'BDD 2' if 'replace_type' is 'Monotone'", [&]() {
        const mapping_type m = [](const int x) { return 2*x; };
        const bdd out = bdd_replace(bdd_2, m, replace_type::Monotone);

        AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
        AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0,
                                                       bdd::max_id,
                                                       bdd::pointer_type(2, bdd::max_id - 2),
                                                       bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(bdd_2->width));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(bdd_2->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_2->number_of_terminals[true]));
      });

      it("returns the original file if 'replace_type is 'Identity'", [&]() {
        // NOTE: This function is in fact *not* the identity!
        const mapping_type m = [](const int x) { return x+1; };
        const bdd out = bdd_replace(bdd_x0, m, replace_type::Identity);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_replace(__bdd&&, <...>)", [&]() {
      describe("<non-monotonic>", [&]() {
        // TODO
      });

      describe("<monotonic> / <affine>", [&]() {
        // TODO
      });

      describe("<identity>", [&]() {
        // TODO
      });
    });

    describe("bdd_replace(__bdd&&, <...>, replace_type)", [&]() {
      // TODO
    });
  });
 });
