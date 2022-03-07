#include <adiar/adiar.h>

go_bandit([]() {
  describe("adiar/bdd/build.cpp", []() {
    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    describe("bdd_sink", [&]() {
      it("can create true sink [bdd_sink]", [&]() {
        __bdd res = bdd_sink(true);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create true sink [bdd_true]", [&]() {
        __bdd res = bdd_true();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create false sink [bdd_sink]", [&]() {
        __bdd res = bdd_sink(false);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("can create false sink [bdd_false]", [&]() {
        __bdd res = bdd_false();
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(0u));
      });
    });

    describe("bdd_ithvar", [&]() {
      it("can create x0", [&]() {
        __bdd res = bdd_ithvar(0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID, sink_F, sink_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create x42", [&]() {
        __bdd res = bdd_ithvar(42);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_nithvar", [&]() {
      it("can create !x1", [&]() {
        __bdd res = bdd_nithvar(1);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID, sink_T, sink_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create !x3", [&]() {
        __bdd res = bdd_nithvar(3);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, MAX_ID, sink_T, sink_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_and", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        __bdd res = bdd_and(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID,
                                                       sink_F,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                       sink_F,
                                                       create_node_ptr(5,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                       sink_F,
                                                       create_node_ptr(2,MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(1u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(3u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create {} as trivially true", [&]() {
        label_file labels;

        __bdd res = bdd_and(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });
    });

    describe("bdd_or", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        __bdd res = bdd_or(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID,
                                                       sink_F,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                       create_node_ptr(5,MAX_ID),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                       create_node_ptr(2,MAX_ID),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(1u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("can create {} as trivially false", [&]() {
        label_file labels;

        __bdd res = bdd_or(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(0u));
      });
    });

    describe("bdd_counter", [&]() {
      it("creates trivial counting to 10 in [0,8]", [&]() {
        __bdd res = bdd_counter(0, 8, 10);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("creates trivial counting to 10 in [10,18]", [&]() {
        __bdd res = bdd_counter(10, 18, 10);

        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("creates counting to 0 in [1,5]", [&]() {
        __bdd res = bdd_counter(1, 5, 0);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 0,
                                                       sink_T,
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(5,0),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       create_node_ptr(4,0),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                       create_node_ptr(2,0),
                                                       sink_F)));

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

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(5u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("creates counting to 2 in [2,5]", [&]() {
        __bdd res = bdd_counter(2, 5, 2);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 2,
                                                       sink_T,
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 1,
                                                       sink_F,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 2,
                                                       create_node_ptr(5,2),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(5,1),
                                                       create_node_ptr(5,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       sink_F,
                                                       create_node_ptr(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(4,1),
                                                       create_node_ptr(4,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

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

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(4u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(4u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("creates counting to 3 in [0,8]", [&]() {
        __bdd res = bdd_counter(0, 8, 3);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 3,
                                                       sink_T,
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 2,
                                                       sink_F,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 3,
                                                       create_node_ptr(8,3),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 2,
                                                       create_node_ptr(8,2),
                                                       create_node_ptr(8,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 1,
                                                       sink_F,
                                                       create_node_ptr(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 3,
                                                       create_node_ptr(7,3),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 2,
                                                       create_node_ptr(7,2),
                                                       create_node_ptr(7,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 1,
                                                       create_node_ptr(7,1),
                                                       create_node_ptr(7,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 0,
                                                       sink_F,
                                                       create_node_ptr(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 3,
                                                       create_node_ptr(6,3),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 2,
                                                       create_node_ptr(6,2),
                                                       create_node_ptr(6,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 1,
                                                       create_node_ptr(6,1),
                                                       create_node_ptr(6,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 0,
                                                       create_node_ptr(6,0),
                                                       create_node_ptr(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 3,
                                                       create_node_ptr(5,3),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 2,
                                                       create_node_ptr(5,2),
                                                       create_node_ptr(5,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(5,1),
                                                       create_node_ptr(5,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(5,0),
                                                       create_node_ptr(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 3,
                                                       create_node_ptr(4,3),
                                                       sink_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 2,
                                                       create_node_ptr(4,2),
                                                       create_node_ptr(4,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(4,1),
                                                       create_node_ptr(4,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 2,
                                                       create_node_ptr(3,2),
                                                       create_node_ptr(3,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(3,1),
                                                       create_node_ptr(3,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 1,
                                                       create_node_ptr(2,1),
                                                       create_node_ptr(2,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0,
                                                       create_node_ptr(1,0),
                                                       create_node_ptr(1,1))));

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

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(7u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(9u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });
    });
  });
 });
