#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/build.cpp", [&]() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    describe("zdd_terminal", [&]() {
      it("can create { Ø } [zdd_terminal]", [&]() {
        zdd res = zdd_terminal(true);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->canonical, Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { Ø } [zdd_null]", [&]() {
        zdd res = zdd_null();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create Ø [zdd_terminal]", [&]() {
        zdd res = zdd_terminal(false);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can create Ø [zdd_empty]", [&]() {
        zdd res = zdd_empty();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });
    });

    describe("zdd_ithvar", [&]() {
      it("can create { {0} }", [&]() {
        zdd res = zdd_ithvar(0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { {42} }", [&]() {
        zdd res = zdd_ithvar(42);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("zdd_vars", [&]() {
      it("can create { Ø } on empty list", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        zdd res = zdd_vars(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { {42} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        zdd res = zdd_vars(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { {1,2,5} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        zdd res = zdd_vars(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, terminal_F, ptr_uint64(5, ptr_uint64::MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID, terminal_F, ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("zdd_singletons", [&]() {
      it("can create Ø on empty list", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        zdd res = zdd_singletons(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can create { {42} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        zdd res = zdd_singletons(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { {1}, {2}, {5} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        zdd res = zdd_singletons(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, ptr_uint64(5, ptr_uint64::MAX_ID), terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
      });
    });

    describe("zdd_powerset", [&]() {
      it("can create { Ø } on empty list", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create { Ø, {42} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, node::MAX_ID, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can create { Ø, {1}, {2}, {5}, {1,2}, {1,5}, {2,5}, {1,2,5} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, node::MAX_ID, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                ptr_uint64(5, ptr_uint64::MAX_ID),
                                                ptr_uint64(5, ptr_uint64::MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                ptr_uint64(2, ptr_uint64::MAX_ID),
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

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(adiar::is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });
    });

    describe("zdd_sized_sets", [&]() {
      // Edge cases
      it("can compute { s <= Ø | |s| <= 0 } to be { Ø }", [&]() {
        adiar::shared_file<zdd::label_t> labels;
        zdd res = zdd_sized_sets(labels, 0, std::less_equal<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can compute { s <= Ø | |s| > 0 } to be Ø", [&]() {
        adiar::shared_file<zdd::label_t> labels;
        zdd res = zdd_sized_sets(labels, 0, std::greater<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can compute { s <= {1,2,3} | |s| < 0 } to be Ø", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3;
        }

        zdd res = zdd_sized_sets(labels, 0, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can compute { s <= {0,2,4,6} | |s| <= 0 } to be { Ø }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        zdd res = zdd_sized_sets(labels, 0, std::less_equal<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can compute { s <= {0,2,4,6} | |s| < 42 } to be the powerset", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        zdd res = zdd_sized_sets(labels, 42, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, node::MAX_ID, terminal_T, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                ptr_uint64(6, ptr_uint64::MAX_ID),
                                                ptr_uint64(6, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                ptr_uint64(4, ptr_uint64::MAX_ID),
                                                ptr_uint64(4, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                ptr_uint64(2, ptr_uint64::MAX_ID),
                                                ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can compute { s <= {0,2,4,6} | |s| > 42 } to be Ø", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        zdd res = zdd_sized_sets(labels, 42, std::greater<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can compute { s <= {0,1,2} | |s| <= 3 } to be the powerset", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        zdd res = zdd_sized_sets(labels, 3, std::less_equal<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, terminal_T, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                ptr_uint64(2, ptr_uint64::MAX_ID),
                                                ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                ptr_uint64(1, ptr_uint64::MAX_ID),
                                                ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can compute { s <= {0,1,2} | |s| == 3 } to be the { {0,1,2} }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        zdd res = zdd_sized_sets(labels, 3, std::equal_to<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                terminal_F,
                                                ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can compute { s <= {0,2,3} | |s| > 3 } to be Ø", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 3;
        }

        zdd res = zdd_sized_sets(labels, 3, std::greater<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can compute { s <= {0,1,2} | |s| < 1 } to be { Ø }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        zdd res = zdd_sized_sets(labels, 1, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      // TODO: More edge cases
      //  - Always true predicate
      //  - always false predicate

      // TODO: Compute maximum 1-level cuts and turn 'GreaterThanOrEqualTo'
      // tests below into 'EqualTo'.

      // General case
      it("can create { s <= {1,3,5} | |s| == 2 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 3 << 5;
        }

        zdd res = zdd_sized_sets(labels, 2, std::equal_to<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(5,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                terminal_F,
                                                ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(5u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(5u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(5u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(5u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can create { s <= {1,2,3,4,6} | |s| == 3 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3 << 4 << 6;
        }

        zdd res = zdd_sized_sets(labels, 3, std::equal_to<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 2, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 2,
                                                ptr_uint64(6,2),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                terminal_F,
                                                ptr_uint64(6,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                ptr_uint64(4,2),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(4,1),
                                                ptr_uint64(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                terminal_F,
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(3,1),
                                                ptr_uint64(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(10u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(10u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(10u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(10u));

        // TODO

        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[0], Is().EqualTo(3u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| >= 2 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        zdd res = zdd_sized_sets(labels, 2, std::greater_equal<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 2, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 1, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 2,
                                                ptr_uint64(8,2),
                                                ptr_uint64(8,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 1,
                                                ptr_uint64(8,1),
                                                ptr_uint64(8,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 0,
                                                terminal_F,
                                                ptr_uint64(8,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 2,
                                                ptr_uint64(6,2),
                                                ptr_uint64(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(6,1),
                                                ptr_uint64(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(6,0),
                                                ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(4,1),
                                                ptr_uint64(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(8,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(12u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,1,2,3,4,5} | |s| > 0 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4 << 5;
        }

        zdd res = zdd_sized_sets(labels, 0, std::greater<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 0, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(5,1),
                                                ptr_uint64(5,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(5,0),
                                                ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(4,1),
                                                ptr_uint64(4,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(3,1),
                                                ptr_uint64(3,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 1,
                                                ptr_uint64(2,1),
                                                ptr_uint64(2,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(1,0),
                                                ptr_uint64(1,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(12u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,1,2,3,5} | |s| > 1 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 5;
        }

        zdd res = zdd_sized_sets(labels, 1, std::greater<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 2, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                ptr_uint64(5,2),
                                                ptr_uint64(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(5,1),
                                                ptr_uint64(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                terminal_F,
                                                ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 2,
                                                ptr_uint64(3,2),
                                                ptr_uint64(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(3,1),
                                                ptr_uint64(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 1,
                                                ptr_uint64(2,1),
                                                ptr_uint64(2,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(1,0),
                                                ptr_uint64(1,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(12u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(12u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| < 2 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        zdd res = zdd_sized_sets(labels, 2, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 0,
                                                terminal_T,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 0,
                                                ptr_uint64(8,0),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(6,0),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(4,0),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(2,0),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(8,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(6u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(6u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(6u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(6u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(6u));
      });

      it("can create { s <= {0,2,4,6} | |s| < 3 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        zdd res = zdd_sized_sets(labels, 3, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 1,
                                                terminal_T,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(6,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(6,1),
                                                ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(4,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(7u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(7u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(5u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(7u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(7u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(4u));
      });

      it("can create { s <= {1,2,3,4,5,6,7,8,9} | |s| < 5 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        }

        zdd res = zdd_sized_sets(labels, 5, std::less<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(9, 3,
                                                terminal_T,
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 3,
                                                ptr_uint64(9,3),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 2,
                                                ptr_uint64(9,3),
                                                ptr_uint64(9,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 3,
                                                ptr_uint64(8,3),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 2,
                                                ptr_uint64(8,2),
                                                ptr_uint64(8,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 1,
                                                ptr_uint64(8,2),
                                                ptr_uint64(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 3,
                                                ptr_uint64(7,3),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 2,
                                                ptr_uint64(7,2),
                                                ptr_uint64(7,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 1,
                                                ptr_uint64(7,1),
                                                ptr_uint64(7,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 0,
                                                ptr_uint64(7,1),
                                                ptr_uint64(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 3,
                                                ptr_uint64(6,3),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 2,
                                                ptr_uint64(6,2),
                                                ptr_uint64(6,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1,
                                                ptr_uint64(6,1),
                                                ptr_uint64(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 0,
                                                ptr_uint64(6,0),
                                                ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 3,
                                                ptr_uint64(5,3),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 2,
                                                ptr_uint64(5,2),
                                                ptr_uint64(5,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(5,1),
                                                ptr_uint64(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(5,0),
                                                ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                ptr_uint64(4,2),
                                                ptr_uint64(4,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(4,1),
                                                ptr_uint64(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(3,1),
                                                ptr_uint64(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(9,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(8,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(7,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(7u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(25u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(7u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(25u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(10u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(25u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(10u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(25u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(7u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| <= 2 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        zdd res = zdd_sized_sets(labels, 2, std::less_equal<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 1, terminal_T, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 1,
                                                ptr_uint64(8,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 0,
                                                ptr_uint64(8,1),
                                                ptr_uint64(8,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(6,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(6,0),
                                                ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(4,1),
                                                terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(8,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(9u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(9u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(9u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(9u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(5u));
      });

      it("can create { s <= {0,1,...,9} | |s| != 4 }", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        }

        zdd res = zdd_sized_sets(labels, 4, std::not_equal_to<zdd::label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(9, 5, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(9, 4, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 5,
                                                ptr_uint64(9,5),
                                                ptr_uint64(9,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 4,
                                                ptr_uint64(9,4),
                                                ptr_uint64(9,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 3,
                                                terminal_T,
                                                ptr_uint64(9,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 2,
                                                ptr_uint64(9,5),
                                                terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 5,
                                                ptr_uint64(8,5),
                                                ptr_uint64(8,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 4,
                                                ptr_uint64(8,4),
                                                ptr_uint64(8,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 3,
                                                ptr_uint64(8,3),
                                                ptr_uint64(8,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 2,
                                                ptr_uint64(8,2),
                                                ptr_uint64(8,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 1,
                                                ptr_uint64(8,5),
                                                ptr_uint64(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 5,
                                                ptr_uint64(7,5),
                                                ptr_uint64(7,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 4,
                                                ptr_uint64(7,4),
                                                ptr_uint64(7,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 3,
                                                ptr_uint64(7,3),
                                                ptr_uint64(7,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 2,
                                                ptr_uint64(7,2),
                                                ptr_uint64(7,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 1,
                                                ptr_uint64(7,1),
                                                ptr_uint64(7,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 0,
                                                ptr_uint64(7,5),
                                                ptr_uint64(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 5,
                                                ptr_uint64(6,5),
                                                ptr_uint64(6,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 4,
                                                ptr_uint64(6,4),
                                                ptr_uint64(6,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 3,
                                                ptr_uint64(6,3),
                                                ptr_uint64(6,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 2,
                                                ptr_uint64(6,2),
                                                ptr_uint64(6,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1,
                                                ptr_uint64(6,1),
                                                ptr_uint64(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 0,
                                                ptr_uint64(6,0),
                                                ptr_uint64(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 4,
                                                ptr_uint64(5,4),
                                                ptr_uint64(5,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 3,
                                                ptr_uint64(5,3),
                                                ptr_uint64(5,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 2,
                                                ptr_uint64(5,2),
                                                ptr_uint64(5,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                ptr_uint64(5,1),
                                                ptr_uint64(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                ptr_uint64(5,0),
                                                ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 3,
                                                ptr_uint64(4,3),
                                                ptr_uint64(4,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                ptr_uint64(4,2),
                                                ptr_uint64(4,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                ptr_uint64(4,1),
                                                ptr_uint64(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                ptr_uint64(4,0),
                                                ptr_uint64(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 2,
                                                ptr_uint64(3,2),
                                                ptr_uint64(3,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                ptr_uint64(3,1),
                                                ptr_uint64(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                ptr_uint64(3,0),
                                                ptr_uint64(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 1,
                                                ptr_uint64(2,1),
                                                ptr_uint64(2,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                ptr_uint64(2,0),
                                                ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                ptr_uint64(1,0),
                                                ptr_uint64(1,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(9,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(8,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(7,5u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(6,6u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5,6u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,5u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(12u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(39u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(12u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(39u));

        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(12u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(39u));

        AssertThat(res->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(12u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(39u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(5u));
      });

      it("can create { s <= {0,1,2,3,4} | T } = Pow({ 0,1,2,3,4 })", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4;
        }

        zdd res = zdd_sized_sets(labels, 2, [](const zdd::label_t&, const zdd::label_t&){ return true; });
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::MAX_ID, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                ptr_uint64(4, node::MAX_ID),
                                                ptr_uint64(4, node::MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                ptr_uint64(3, node::MAX_ID),
                                                ptr_uint64(3, node::MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                ptr_uint64(2, node::MAX_ID),
                                                ptr_uint64(2, node::MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                ptr_uint64(1, node::MAX_ID),
                                                ptr_uint64(1, node::MAX_ID))));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("can create { s <= {0,1,2,3,4} | F } = Ø", [&]() {
        adiar::shared_file<zdd::label_t> labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4;
        }

        zdd res = zdd_sized_sets(labels, 2, [](const zdd::label_t&, const zdd::label_t&){ return false; });
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        // TODO

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });
    });
  });
 });
