#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/build.cpp", []() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    describe("bdd_terminal(v)", [&]() {
      it("can create true terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(true);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can create true terminal [bdd_true]", [&]() {
        bdd res = bdd_true();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can create false terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(false);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("can create false terminal [bdd_false]", [&]() {
        bdd res = bdd_false();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(0u));
      });
    });

    describe("bdd_ithvar(i)", [&]() {
      it("can create x0", [&]() {
        bdd res = bdd_ithvar(0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can create x42", [&]() {
        bdd res = bdd_ithvar(42);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("throws exception for a too large label", []() {
        AssertThrows(invalid_argument, bdd_nithvar(bdd::MAX_LABEL+1));
      });
    });

    describe("bdd_nithvar(i)", [&]() {
      it("can create !x1", [&]() {
        bdd res = bdd_nithvar(1);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can create !x3", [&]() {
        bdd res = bdd_nithvar(3);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("throws exception for a too large label", []() {
        AssertThrows(invalid_argument, bdd_nithvar(bdd::MAX_LABEL+1));
      });
    });

    describe("bdd_and(vars)", [&]() {
      // TODO
    });

    describe("bdd_and(begin, end)", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                terminal_F,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(5, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can create {} as trivially true", [&]() {
        std::vector<int> vars;

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("skips duplicates", [&]() {
        std::vector<int> vars = { 1, 2, 2, 5 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                terminal_F,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(5, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("throws exception for non-ascending list", []() {
        std::vector<int> vars = { 3, 2 };

        AssertThrows(invalid_argument, bdd_and(vars.rbegin(), vars.rend()));
      });
    });

    describe("bdd_or(vars)", [&]() {
      // TODO
    });

    describe("bdd_or(begin, end)", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                terminal_F,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                ptr_uint64(5, ptr_uint64::MAX_ID),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                ptr_uint64(2, ptr_uint64::MAX_ID),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("can create {} as trivially false", [&]() {
        std::vector<int> vars;

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("skips duplicates", [&]() {
        std::vector<int> vars = { 1, 2, 2, 5 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                terminal_F,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                ptr_uint64(5, ptr_uint64::MAX_ID),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                ptr_uint64(2, ptr_uint64::MAX_ID),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("throws exception for non-ascending list", []() {
        std::vector<int> vars = { 3, 2 };

        AssertThrows(invalid_argument, bdd_or(vars.rbegin(), vars.rend()));
      });
    });
  });
 });
