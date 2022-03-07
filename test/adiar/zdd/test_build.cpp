go_bandit([]() {
  describe("adiar/zdd/build.cpp", [&]() {
    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    describe("zdd_sink", [&]() {
      it("can create { Ø } [zdd_sink]", [&]() {
        __zdd res = zdd_sink(true);
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

      it("can create { Ø } [zdd_null]", [&]() {
        __zdd res = zdd_null();
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

      it("can create Ø [zdd_sink]", [&]() {
        __zdd res = zdd_sink(false);
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

      it("can create Ø [zdd_empty]", [&]() {
        __zdd res = zdd_empty();
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

    describe("zdd_ithvar", [&]() {
      it("can create { {0} }", [&]() {
        __zdd res = zdd_ithvar(0);
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

      it("can create { {42} }", [&]() {
        __zdd res = zdd_ithvar(42);
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

    describe("zdd_vars", [&]() {
      it("can create { Ø } on empty list", [&]() {
        label_file labels;

        __zdd res = zdd_vars(labels);
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

      it("can create { {42} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        __zdd res = zdd_vars(labels);
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

      it("can create { {1,2,5} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        __zdd res = zdd_vars(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID, sink_F, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, create_node_ptr(5,MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID, sink_F, create_node_ptr(2,MAX_ID))));

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
    });

    describe("zdd_singletons", [&]() {
      it("can create Ø on empty list", [&]() {
        label_file labels;

        __zdd res = zdd_singletons(labels);
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

      it("can create { {42} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        __zdd res = zdd_singletons(labels);
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

      it("can create { {1}, {2}, {5} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        __zdd res = zdd_singletons(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID, sink_F, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, create_node_ptr(5,MAX_ID), sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));

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
    });

    describe("zdd_powerset", [&]() {
      it("can create { Ø } on empty list", [&]() {
        label_file labels;

        __zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can create { Ø, {42} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 42;
        }

        __zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(42,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can create { Ø, {1}, {2}, {5}, {1,2}, {1,5}, {2,5}, {1,2,5} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 5;
        }

        __zdd res = zdd_powerset(labels);
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                       create_node_ptr(5,MAX_ID),
                                                       create_node_ptr(5,MAX_ID))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                       create_node_ptr(2,MAX_ID),
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

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().EqualTo(2u));

        AssertThat(res.get<node_file>()._file_ptr->canonical, Is().True());

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });
    });

    describe("zdd_sized_sets", [&]() {
      // Edge cases
      it("can compute { s <= Ø | |s| <= 0 } to be { Ø }", [&]() {
        label_file labels;
        __zdd res = zdd_sized_sets(labels, 0, std::less_equal<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can compute { s <= Ø | |s| > 0 } to be Ø", [&]() {
        label_file labels;
        __zdd res = zdd_sized_sets(labels, 0, std::greater<label_t>());
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

      it("can compute { s <= {1,2,3} | |s| < 0 } to be Ø", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3;
        }

        __zdd res = zdd_sized_sets(labels, 0, std::less<label_t>());
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

      it("can compute { s <= {0,2,4,6} | |s| <= 0 } to be { Ø }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        __zdd res = zdd_sized_sets(labels, 0, std::less_equal<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can compute { s <= {0,2,4,6} | |s| < 42 } to be the powerset", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        __zdd res = zdd_sized_sets(labels, 42, std::less<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, MAX_ID, sink_T, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, MAX_ID,
                                                       create_node_ptr(6,MAX_ID),
                                                       create_node_ptr(6,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                       create_node_ptr(4,MAX_ID),
                                                       create_node_ptr(4,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                       create_node_ptr(2,MAX_ID),
                                                       create_node_ptr(2,MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can compute { s <= {0,2,4,6} | |s| > 42 } to be Ø", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        __zdd res = zdd_sized_sets(labels, 42, std::greater<label_t>());
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

      it("can compute { s <= {0,1,2} | |s| <= 3 } to be the powerset", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        __zdd res = zdd_sized_sets(labels, 3, std::less_equal<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_T, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                       create_node_ptr(2,MAX_ID),
                                                       create_node_ptr(2,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                       create_node_ptr(1,MAX_ID),
                                                       create_node_ptr(1,MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can compute { s <= {0,1,2} | |s| == 3 } to be the { {0,1,2} }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        __zdd res = zdd_sized_sets(labels, 3, std::equal_to<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                       sink_F,
                                                       create_node_ptr(2,MAX_ID))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                       sink_F,
                                                       create_node_ptr(1,MAX_ID))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(3u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can compute { s <= {0,2,3} | |s| > 3 } to be Ø", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 3;
        }

        __zdd res = zdd_sized_sets(labels, 3, std::greater<label_t>());
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

      it("can compute { s <= {0,1,2} | |s| < 1 } to be { Ø }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2;
        }

        __zdd res = zdd_sized_sets(labels, 1, std::less<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(1u));
      });

      // TODO: More edge cases
      //  - Always true predicate
      //  - always false predicate

      // General case
      it("can create { s <= {1,3,5} | |s| == 2 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 3 << 5;
        }

        __zdd res = zdd_sized_sets(labels, 2, std::equal_to<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 1, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(5,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       sink_F,
                                                       create_node_ptr(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can create { s <= {1,2,3,4,6} | |s| == 3 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3 << 4 << 6;
        }

        __zdd res = zdd_sized_sets(labels, 3, std::equal_to<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 2, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 2,
                                                       create_node_ptr(6,2),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       sink_F,
                                                       create_node_ptr(6,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 2,
                                                       create_node_ptr(4,2),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(4,1),
                                                       create_node_ptr(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       sink_F,
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(3,1),
                                                       create_node_ptr(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(4u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(3u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| >= 2 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        __zdd res = zdd_sized_sets(labels, 2, std::greater_equal<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 2, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 1, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 2,
                                                       create_node_ptr(8,2),
                                                       create_node_ptr(8,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 1,
                                                       create_node_ptr(8,1),
                                                       create_node_ptr(8,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 0,
                                                       sink_F,
                                                       create_node_ptr(8,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 2,
                                                       create_node_ptr(6,2),
                                                       create_node_ptr(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(6,1),
                                                       create_node_ptr(6,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(6,0),
                                                       create_node_ptr(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(4,1),
                                                       create_node_ptr(4,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(8,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(6u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,1,2,3,4,5} | |s| > 0 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4 << 5;
        }

        __zdd res = zdd_sized_sets(labels, 0, std::greater<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 1, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 0, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(5,1),
                                                       create_node_ptr(5,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(5,0),
                                                       create_node_ptr(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(4,1),
                                                       create_node_ptr(4,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(3,1),
                                                       create_node_ptr(3,1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 1,
                                                       create_node_ptr(2,1),
                                                       create_node_ptr(2,1))));
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
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,2u)));
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

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(4u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,1,2,3,5} | |s| > 1 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 5;
        }

        __zdd res = zdd_sized_sets(labels, 1, std::greater<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 2, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 1, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 2,
                                                       create_node_ptr(5,2),
                                                       create_node_ptr(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 1,
                                                       create_node_ptr(5,1),
                                                       create_node_ptr(5,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0,
                                                       sink_F,
                                                       create_node_ptr(5,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 2,
                                                       create_node_ptr(3,2),
                                                       create_node_ptr(3,2))));
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
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(6u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| < 2 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        __zdd res = zdd_sized_sets(labels, 2, std::less<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 0,
                                                       sink_T,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 0,
                                                       create_node_ptr(8,0),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(6,0),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(4,0),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0,
                                                       create_node_ptr(2,0),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(8,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(6u));
      });

      it("can create { s <= {0,2,4,6} | |s| < 3 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6;
        }

        __zdd res = zdd_sized_sets(labels, 3, std::less<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 1,
                                                       sink_T,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(6,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(6,1),
                                                       create_node_ptr(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(4,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(4u));
      });

      it("can create { s <= {1,2,3,4,5,6,7,8,9} | |s| < 5 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        }

        __zdd res = zdd_sized_sets(labels, 5, std::less<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(9, 3,
                                                       sink_T,
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 3,
                                                       create_node_ptr(9,3),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 2,
                                                       create_node_ptr(9,3),
                                                       create_node_ptr(9,3))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 3,
                                                       create_node_ptr(8,3),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 2,
                                                       create_node_ptr(8,2),
                                                       create_node_ptr(8,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 1,
                                                       create_node_ptr(8,2),
                                                       create_node_ptr(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 3,
                                                       create_node_ptr(7,3),
                                                       sink_T)));
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
                                                       create_node_ptr(7,1),
                                                       create_node_ptr(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 3,
                                                       create_node_ptr(6,3),
                                                       sink_T)));
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
                                                       sink_T)));
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
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(3,1),
                                                       create_node_ptr(3,2))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(3,0),
                                                       create_node_ptr(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(9,1u)));
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
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(7u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(7u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| <= 2 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 2 << 4 << 6 << 8;
        }

        __zdd res = zdd_sized_sets(labels, 2, std::less_equal<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 1, sink_T, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 1,
                                                       create_node_ptr(8,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 0,
                                                       create_node_ptr(8,1),
                                                       create_node_ptr(8,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 1,
                                                       create_node_ptr(6,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 0,
                                                       create_node_ptr(6,0),
                                                       create_node_ptr(6,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 1,
                                                       create_node_ptr(4,1),
                                                       sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                       create_node_ptr(4,0),
                                                       create_node_ptr(4,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0,
                                                       create_node_ptr(2,0),
                                                       create_node_ptr(2,1))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_stream<node_t> ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(8,1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(5u));
      });

      it("can create { s <= {0,2,4,6,8} | |s| != 4 }", [&]() {
        label_file labels;

        { // Garbage collect writer to free write-lock
          label_writer lw(labels);
          lw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        }

        __zdd res = zdd_sized_sets(labels, 4, std::not_equal_to<label_t>());
        node_test_stream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(9, 5, sink_T, sink_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(9, 4, sink_F, sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 5,
                                                       create_node_ptr(9,5),
                                                       create_node_ptr(9,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 4,
                                                       create_node_ptr(9,4),
                                                       create_node_ptr(9,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 3,
                                                       sink_T,
                                                       create_node_ptr(9,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(8, 2,
                                                       create_node_ptr(9,5),
                                                       sink_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 5,
                                                       create_node_ptr(8,5),
                                                       create_node_ptr(8,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 4,
                                                       create_node_ptr(8,4),
                                                       create_node_ptr(8,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 3,
                                                       create_node_ptr(8,3),
                                                       create_node_ptr(8,4))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 2,
                                                       create_node_ptr(8,2),
                                                       create_node_ptr(8,3))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(7, 1,
                                                       create_node_ptr(8,5),
                                                       create_node_ptr(8,2))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 5,
                                                       create_node_ptr(7,5),
                                                       create_node_ptr(7,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 4,
                                                       create_node_ptr(7,4),
                                                       create_node_ptr(7,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(6, 3,
                                                       create_node_ptr(7,3),
                                                       create_node_ptr(7,4))));
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
                                                       create_node_ptr(7,5),
                                                       create_node_ptr(7,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 5,
                                                       create_node_ptr(6,5),
                                                       create_node_ptr(6,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 4,
                                                       create_node_ptr(6,4),
                                                       create_node_ptr(6,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(5, 3,
                                                       create_node_ptr(6,3),
                                                       create_node_ptr(6,4))));
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
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 4,
                                                       create_node_ptr(5,4),
                                                       create_node_ptr(5,5))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(create_node(4, 3,
                                                       create_node_ptr(5,3),
                                                       create_node_ptr(5,4))));
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
                                                       create_node_ptr(4,4))));
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
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(9,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(8,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(7,5u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(6,6u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(5,6u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,5u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,4u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,3u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res.get<node_file>()._file_ptr->max_1level_cut, Is().GreaterThanOrEqualTo(12u));

        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(res.get<node_file>()._file_ptr->number_of_sinks[1], Is().EqualTo(5u));
      });
    });
  });
 });
