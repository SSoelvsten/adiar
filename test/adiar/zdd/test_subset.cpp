#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/subset.cpp", [&]() {
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

    // { {3}, {0,3}, {3,4}, {0,3,4}, {1,2,4}, {1,2,3}, {1,3,4}, {0,1,2,4}, {0,1,2,3}, {0,1,3,4} }
    /*
                  1      ---- x0
                  ||
                  2      ---- x1
                 / \
                 | 3     ---- x2
                 |/ \
                 4  5    ---- x3
                / \/ \
                F 6  T   ---- x4
                 / \
                 T T
     */
    shared_levelized_file<zdd::node_t> zdd_1;

    node n1_6 = node(4, node::MAX_ID,   terminal_T, terminal_T);
    node n1_5 = node(3, node::MAX_ID,   n1_6.uid(), terminal_T);
    node n1_4 = node(3, node::MAX_ID-1, terminal_F, n1_6.uid());
    node n1_3 = node(2, node::MAX_ID,   n1_4.uid(), n1_5.uid());
    node n1_2 = node(1, node::MAX_ID,   n1_4.uid(), n1_3.uid());
    node n1_1 = node(0, node::MAX_ID,   n1_2.uid(), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer w(zdd_1);
      w << n1_6 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    // { Ø, {0}, {0,2}, {1,2}, {0,2,3}, {1,2,3} }
    /*
           1       ---- x0
          / \
          2 |      ---- x1
         / \|
         T  3      ---- x2
           / \
           T 4     ---- x3
            / \
            F T
     */
    shared_levelized_file<zdd::node_t> zdd_2;

    const node n2_4 = node(3, node::MAX_ID, terminal_F, terminal_T);
    const node n2_3 = node(2, node::MAX_ID, terminal_T, n2_4.uid());
    const node n2_2 = node(1, node::MAX_ID, terminal_T, n2_3.uid());
    const node n2_1 = node(0, node::MAX_ID, n2_2.uid(), n2_3.uid());

    { // Garbage collect writer to free write-lock
      node_writer w(zdd_2);
      w << n2_4 << n2_3 << n2_2 << n2_1;
    }

    describe("zdd_offset(A, vars)", [&]() {
      // TODO
    });

    describe("zdd_offset(A, begin, end)", [&]() {
      it("should return input unchanged when given Ø", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_offset(zdd_F, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_F));
      });

      it("should return input unchanged when given { Ø }", [&]() {
        std::vector<int> labels = { 7, 42 };

        __zdd out = zdd_offset(zdd_T, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_T));
      });

      it("should return input unchanged when given empty set of labels", [&]() {
        std::vector<int> labels = { };

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_1));
      });

      it("should return input unchanged when given { { 2 }, { 1,2 } } without (0,3,4,5,6)", [&]() {
        std::vector<int> labels = { 0, 3, 4, 5, 6 };

        shared_levelized_file<zdd::node_t> in;

        {
          node_writer nw(in);
          nw << node(2, node::MAX_ID, terminal_F, terminal_T)
             << node(1, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID));
        }

        __zdd out = zdd_offset(in, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(in));
      });

      it("should return { Ø } when given { Ø, { 1,2 } } without (0,2)", [&]() {
        std::vector<int> labels = { 0, 2 };

        shared_levelized_file<zdd::node_t> in;

        node n2 = node(2, node::MAX_ID, terminal_F, terminal_T);
        node n1 = node(1, node::MAX_ID, terminal_T, n2.uid());;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should return { Ø, { 1 } } when given { Ø, { 1 }, { 2 }, { 1, 2 } } without (0,2)", [&]() {
        std::vector<int> labels = { 0, 2 };

        shared_levelized_file<zdd::node_t> in;

        node n2 = node(2, node::MAX_ID-1, terminal_T, terminal_T);
        node n1 = node(1, node::MAX_ID, n2.uid(), n2.uid());;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should return { Ø, { 2 } } when given { Ø, { 1 }, { 2 }, { 1, 2 } } without (1,3)", [&]() {
        std::vector<int> labels = { 1, 3};

        shared_levelized_file<zdd::node_t> in;

        node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
        node n1 = node(1, node::MAX_ID, n2.uid(), n2.uid());;

        {
          node_writer nw(in);
          nw << n2 << n1;
        }

        __zdd out = zdd_offset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should skip root of [1] without (0,42)", [&]() {
        std::vector<int> labels = { 0, 42 };

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), true,  n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), true,  n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_4.uid(), true,  n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_5.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should skip 'dead' nodes in [1] without (1)", [&]() {
        std::vector<int> labels = { 1 };

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), true,  n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_4.uid(), true,  n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should restrict to a terminal in [1] without (0,1,3)", [&]() {
        std::vector<int> labels = { 0, 1, 3 };

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should restrict to a terminal in { Ø, { 1 }, { 2, 3 } } without (1,2)", [&]() {
        /*
                  1       ---- x1
                 / \
                 2 T      ---- x2
                / \
                T 3
                 / \
                 F T
        */
        shared_levelized_file<zdd::node_t> in;

        {
          node_writer w(in);
          w << node(3, node::MAX_ID, terminal_F, terminal_T)
            << node(2, node::MAX_ID, terminal_T, ptr_uint64(3, ptr_uint64::MAX_ID))
            << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), terminal_T);
        }

        std::vector<int> labels = { 1,2 };

        __zdd out = zdd_offset(in, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should restrict to a terminal from root", [&]() {
        /*
                 1       ---- x1
                / \
                T 2      ---- x2
                 / \
                 T T
         */
        shared_levelized_file<zdd::node_t> in;

        {
          node_writer w(in);
          w << node(2, node::MAX_ID, terminal_T, terminal_T)
            << node(1, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID));
        }

        std::vector<int> labels = { 1 };

        __zdd out = zdd_offset(in, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should bridge levels in [2] on (3)", [&]() {
        std::vector<int> labels = { 3 };

        __zdd out = zdd_offset(zdd_2, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), false, n2_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_2.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });
    });

    describe("zdd_onset(A, vars)", [&]() {
    });

    describe("zdd_onset(A, begin, end)", [&]() {
      it("should return input unchanged when given Ø", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_onset(zdd_F, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_F));
      });

      it("should return input unchanged when given { Ø } for ()", [&]() {
        std::vector<int> labels = { };

        __zdd out = zdd_onset(zdd_T, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_T));
      });

      it("should return input unchanged when given [1] for ()", [&]() {
        std::vector<int> labels = { };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>(), Is().EqualTo(zdd_1));
      });

      it("should return Ø when given { Ø } for (0)", [&]() {
        std::vector<int> labels = { 0 };

        __zdd out = zdd_onset(zdd_T, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should return Ø when given { Ø } for (21,42)", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_onset(zdd_T, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should return Ø when given disjoint labels", [&]() {
        std::vector<int> labels = { 5, 6 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should return { { 0 } } when given { Ø, { 0 } } for (0)", [&]() {
        shared_levelized_file<zdd::node_t> in;

        const node n = node(0, node::MAX_ID, terminal_T, terminal_T);

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << n;
        }

        std::vector<int> labels = { 0 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should return Ø when given { Ø, { 0 } } for (0,1)", [&]() {
        shared_levelized_file<zdd::node_t> in;

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << node(0, node::MAX_ID, terminal_T, terminal_T);
        }


        std::vector<int> labels = { 0, 1 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should return { Ø } in [2] for (0,2,3)", [&]() {
        // One would normally expect this to return { Ø }, but for zdd_onset
        // this is not the case!
        std::vector<int> labels = { 0, 2, 3 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_3.uid(), true,  n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should return Ø in [2] for (0,2,3,4)", [&]() {
        // Similar to above, while it inevitably will be Ø, the top-down nature
        // of zdd_onset cannot know this before having output nodes. The later
        // Reduce will put it to this.
        //
        // Yet, it can still skip the bottom-most node (4) as it has not yet
        // output in-going arcs.

        std::vector<int> labels = { 0, 2, 3, 4 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should keep root of [1] but shortcut its low for (0)", [&]() {
        std::vector<int> labels = { 0 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), true,  n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), true,  n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), true,  n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_4.uid(), true,  n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_5.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should skip 'dead' nodes of [1] for (1,2)", [&]() {
        std::vector<int> labels = { 1, 2 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), false, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), true,  n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), true,  n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), true,  n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_5.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should return Ø in { Ø, {0}, {0,2} } for (1)", [&]() {
          /*
             1       ---- x0
            / \
            T |      ---- x1
              |
              2      ---- x2
             / \
             T T
          */
        shared_levelized_file<zdd::node_t> in;

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << node(2, node::MAX_ID, terminal_T, terminal_T)
            << node(0, node::MAX_ID, terminal_T, ptr_uint64(2, ptr_uint64::MAX_ID));
        }

        std::vector<int> labels = { 1 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("should cut edge going across onset label in { {2}, {0,1}, {0,2}, {0,1,2} } for (1)", [&]() {
          /*
               1       ---- x0
              / \
              |  2     ---- x1
              \ //
               3       ---- x2
              / \
              F T
          */
        shared_levelized_file<zdd::node_t> in;

        const node n3 = node(2, node::MAX_ID, terminal_F, terminal_T);
        const node n2 = node(1, node::MAX_ID, n3.uid(), n3.uid());
        const node n1 = node(0, node::MAX_ID, n3.uid(), n2.uid());

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << n3 << n2 << n1;
        }

        std::vector<int> labels = { 1 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2.uid(), true,  n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should cut edge and ignore 'dead' node in { {2}, {0,1}, {0,2} } for (1)", [&]() {
        /*
               1       ---- x0
              / \
              |  2     ---- x1
              \ / \
               3  T    ---- x2
              / \
              F T
        */
        shared_levelized_file<zdd::node_t> in;

        const node n3 = node(2, node::MAX_ID, terminal_F, terminal_T);
        const node n2 = node(1, node::MAX_ID, n3.uid(), terminal_T);
        const node n1 = node(0, node::MAX_ID, n3.uid(), n2.uid());

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << n3 << n2 << n1;
        }

        std::vector<int> labels = { 1 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should falsify early terminals in [2] for (3)", [&]() {
        std::vector<int> labels = { 3 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), false, n2_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_1.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_2.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_3.uid(), true,  n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should skip root in [2] due to cut on high edge for (1,3)", [&]() {
        std::vector<int> labels = { 1 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_2.uid(), true,  n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2_3.uid(), true,  n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2_4.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should falsify early terminal and bridge over removed node in [1] for (4)", [&]() {
        std::vector<int> labels = { 4 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), false, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_1.uid(), true,  n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), true,  n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_3.uid(), true,  n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1_4.uid(), true,  n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1_6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should cut high edge on restricted node, if it goes past the next label", [&]() {
        /*
                   _1_       ---- x0
                  /   \
                  2   3      ---- x1
                 / \ / \
                 T  |  4     ---- x2
                    \ / \
                     5  6    ---- x3
                    / \/ \
                    F T  T
        */
        shared_levelized_file<zdd::node_t> in;

        node n6 = node(3, node::MAX_ID,   terminal_T, terminal_T);
        node n5 = node(3, node::MAX_ID-1, terminal_F, terminal_T);
        node n4 = node(2, node::MAX_ID,   n5.uid(), n6.uid());
        node n3 = node(1, node::MAX_ID,   n5.uid(), n4.uid());
        node n2 = node(1, node::MAX_ID-1, terminal_T, n5.uid());
        node n1 = node(0, node::MAX_ID,   n2.uid(), n3.uid());

        { // Garbage collect writer to free write-lock
          node_writer w(in);
          w << n6 << n5 << n4 << n3 << n2 << n1;
        }

        std::vector<int> labels = { 1, 2 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n3.uid(), true,  n4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n4.uid(), true,  n6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n6.uid(), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
      });
    });
  });
 });
