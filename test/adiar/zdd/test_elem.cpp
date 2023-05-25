#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/elem.cpp", [&]() {
    shared_levelized_file<zdd::node_t> zdd_F;
    shared_levelized_file<zdd::node_t> zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << node(false);

      node_writer nw_T(zdd_T);
      nw_T << node(true);
    }

    const ptr_uint64 terminal_T = ptr_uint64(true);
    const ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<zdd::node_t> zdd_1;
    // { { 0 }, { 1 }, { 0,2 }, { 1,2 } }
    /*
    //      1     ---- x0
    //     / \
    //     2 |    ---- x1
    //    / \|
    //    F  3    ---- x2
    //      / \
    //      T T
    */

    {
      const node n3 = node(2, node::MAX_ID, terminal_T, terminal_T);
      const node n2 = node(1, node::MAX_ID, terminal_F, n3.uid());
      const node n1 = node(0, node::MAX_ID, n2.uid(), n3.uid());

      node_writer nw(zdd_1);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_t> zdd_2;
    // { Ø, { 1 }, { 2 }, { 2,3 } }
    /*
    //       1      ---- x1
    //      / \
    //      2 T     ---- x2
    //     / \
    //     T 3      ---- x3
    //      / \
    //      T T
     */

    {
      const node n3 = node(3, node::MAX_ID, terminal_T, terminal_T);
      const node n2 = node(2, node::MAX_ID, terminal_T, n3.uid());
      const node n1 = node(1, node::MAX_ID, n2.uid(), terminal_T);

      node_writer nw(zdd_2);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_t> zdd_3;
    // { { 2,4 }, { 0,2 }, { 0,4 } }
    /*
    //      _1_      ---- x0
    //     /   \
    //     2   3     ---- x2
    //    / \ / \
    //    F  4  T    ---- x4
    //      / \
    //      F T
    */
    {
      const node n4 = node(4, node::MAX_ID,   terminal_F, terminal_T);
      const node n3 = node(2, node::MAX_ID,   n4.uid(), terminal_T);
      const node n2 = node(2, node::MAX_ID-1, terminal_F, n4.uid());
      const node n1 = node(1, node::MAX_ID,   n2.uid(), n3.uid());

      node_writer nw(zdd_3);
      nw << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_t> zdd_4;
    // { {1}, {0,1} }
    /*
    //      1      ---- x0
    //     | |
    //      2      ---- x1
    //     / \
    //     F T
    */
    {
      const node n2 = node(1, node::MAX_ID, terminal_F, terminal_T);
      const node n1 = node(0, node::MAX_ID, n2.uid(), n2.uid());

      node_writer nw(zdd_4);
      nw << n2 << n1;
    }

    describe("zdd_minelem", [&]() {
      it("finds Ø on Ø", [&]() {
        zdd out = zdd_minelem(zdd_F);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds { Ø } set on { Ø }", [&]() {
        zdd out = zdd_minelem(zdd_T);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds { {1} } on [1]", [&]() {
        zdd out = zdd_minelem(zdd_1);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(1, zdd::MAX_ID, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds { Ø } on [2]", [&]() {
        zdd out = zdd_minelem(zdd_2);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds {2,4} on [3]", [&]() {
        zdd out = zdd_minelem(zdd_3);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(4, zdd::MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(2, zdd::MAX_ID,
                                                              terminal_F,
                                                              zdd::ptr_t(4, zdd::MAX_ID))));


        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds {1} on [4]", [&]() {
        zdd out = zdd_minelem(zdd_4);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(1, zdd::MAX_ID, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });

    describe("zdd_maxelem", [&]() {
      it("finds Ø on Ø", [&]() {
        zdd out = zdd_maxelem(zdd_F);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds { Ø } on { Ø }", [&]() {
        zdd out = zdd_maxelem(zdd_T);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds {0,2} on [1]", [&]() {
        zdd out = zdd_maxelem(zdd_1);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(2, zdd::MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(0, zdd::MAX_ID,
                                                              terminal_F,
                                                              zdd::ptr_t(2, zdd::MAX_ID))));


        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds {1} on [2]", [&]() {
        zdd out = zdd_maxelem(zdd_2);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(1, zdd::MAX_ID, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("finds {0,1} on [4]", [&]() {
        zdd out = zdd_maxelem(zdd_4);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(1, zdd::MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_t(0, zdd::MAX_ID,
                                                              terminal_F,
                                                              zdd::ptr_t(1, zdd::MAX_ID))));


        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });
  });
 });
