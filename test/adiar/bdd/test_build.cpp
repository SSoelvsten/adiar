#include <adiar/adiar.h>

go_bandit([]() {
  describe("adiar/bdd/build.cpp", []() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    describe("bdd_terminal", [&]() {
      it("can create true terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(true);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(is_canonical(res), Is().True());

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

      it("can create true terminal [bdd_true]", [&]() {
        bdd res = bdd_true();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create false terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(false);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can create false terminal [bdd_false]", [&]() {
        bdd res = bdd_false();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });
    });

    describe("bdd_ithvar", [&]() {
      it("can create x0", [&]() {
        bdd res = bdd_ithvar(0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create x42", [&]() {
        bdd res = bdd_ithvar(42);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_nithvar", [&]() {
      it("can create !x1", [&]() {
        bdd res = bdd_nithvar(1);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, MAX_ID, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create !x3", [&]() {
        bdd res = bdd_nithvar(3);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, MAX_ID, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_and", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        bdd res = bdd_and(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, MAX_ID,
                                                       terminal_F,
                                                       ptr_uint64(5,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, MAX_ID,
                                                       terminal_F,
                                                       ptr_uint64(2,MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can create {} as trivially true", [&]() {
        label_file labels;

        bdd res = bdd_and(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_or", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        bdd res = bdd_or(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, MAX_ID,
                                                       ptr_uint64(5,MAX_ID),
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, MAX_ID,
                                                       ptr_uint64(2,MAX_ID),
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(3u));
      });

      it("can create {} as trivially false", [&]() {
        label_file labels;

        bdd res = bdd_or(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(is_canonical(res), Is().True());

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(0u));
      });
    });

    describe("bdd_counter", [&]() {
      it("collapses impossible counting to 10 in [0,8] to F", [&]() {
        bdd res = bdd_counter(0, 8, 10);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
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

      it("collapses impossible counting to 2 in [10,10] to F", [&]() {
        bdd res = bdd_counter(10, 10, 2);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
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

      it("collapses counting to 1 in [42,42] to x42", [&]() {
        bdd res = bdd_counter(42, 42, 1);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, MAX_ID, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(42,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("collapses counting to 0 in [10, 10] to ~x10", [&]() {
        bdd res = bdd_counter(10, 10, 0);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(10, MAX_ID, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(10,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("creates counting to 0 in [1,5]", [&]() {
        bdd res = bdd_counter(1, 5, 0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 0,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                       ptr_uint64(5,0),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                       ptr_uint64(4,0),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                       ptr_uint64(3,0),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                       ptr_uint64(2,0),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(5u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(5u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("creates counting to 5 in [1,5]", [&]() {
        bdd res = bdd_counter(1, 5, 5);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 4,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 3,
                                                       terminal_F,
                                                       ptr_uint64(5,4))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                       terminal_F,
                                                       ptr_uint64(4,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                       terminal_F,
                                                       ptr_uint64(3,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                       terminal_F,
                                                       ptr_uint64(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(5u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(5u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("creates counting to 2 in [2,5]", [&]() {
        bdd res = bdd_counter(2, 5, 2);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 2,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 1,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 2,
                                                       ptr_uint64(5,2),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 1,
                                                       ptr_uint64(5,1),
                                                       ptr_uint64(5,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 0,
                                                       terminal_F,
                                                       ptr_uint64(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 1,
                                                       ptr_uint64(4,1),
                                                       ptr_uint64(4,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 0,
                                                       ptr_uint64(4,0),
                                                       ptr_uint64(4,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 0,
                                                       ptr_uint64(3,0),
                                                       ptr_uint64(3,1))));

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,3u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(6u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(5u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(6u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(5u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(6u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(4u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("creates counting to 3 in [0,8]", [&]() {
        bdd res = bdd_counter(0, 8, 3);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 3,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(8, 2,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 3,
                                                       ptr_uint64(8,3),
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 2,
                                                       ptr_uint64(8,2),
                                                       ptr_uint64(8,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(7, 1,
                                                       terminal_F,
                                                       ptr_uint64(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(6, 3,
                                                       ptr_uint64(7,3),
                                                       terminal_F)));

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
                                                       terminal_F,
                                                       ptr_uint64(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, 3,
                                                       ptr_uint64(6,3),
                                                       terminal_F)));

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
                                                       terminal_F)));

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
                                                       terminal_F)));

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

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(8,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(7,3u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,4u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,4u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,4u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,4u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,3u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(7u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(11u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(7u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(11u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(8u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(11u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(8u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(11u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(9u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("creates counting to 4 in [0,4]", [&]() {
        bdd res = bdd_counter(0, 4, 4);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 4,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, 3,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 3,
                                                       ptr_uint64(4,3),
                                                       ptr_uint64(4,4))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, 2,
                                                       terminal_F,
                                                       ptr_uint64(4,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 2,
                                                       ptr_uint64(3,2),
                                                       ptr_uint64(3,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, 1,
                                                       terminal_F,
                                                       ptr_uint64(3,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 1,
                                                       ptr_uint64(2,1),
                                                       ptr_uint64(2,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                       terminal_F,
                                                       ptr_uint64(2,1))));

        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                       ptr_uint64(1,0),
                                                       ptr_uint64(1,1))));

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(5u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(7u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(4u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(6u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(7u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(5u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });

      it("creates counting to 1 in [0,1]", [&]() {
        bdd res = bdd_counter(0, 1, 1);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 1,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, 0,
                                                       terminal_F,
                                                       terminal_T)));

        AssertThat(ns.pull(), Is().EqualTo(node(0, 0,
                                                       ptr_uint64(1,0),
                                                       ptr_uint64(1,1))));

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(res->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[1], Is().EqualTo(2u));
      });
    });
  });
 });
