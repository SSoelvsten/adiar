go_bandit([]() {
    describe("ZDD: Build", [&]() {
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        describe("zdd_sink", [&]() {
            it("can create { Ø } [zdd_sink]", [&]() {
                zdd res = zdd_sink(true);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { Ø } [zdd_null]", [&]() {
                zdd res = zdd_null();
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create Ø [zdd_sink]", [&]() {
                zdd res = zdd_sink(false);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create Ø [zdd_empty]", [&]() {
                zdd res = zdd_empty();
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("zdd_ithvar", [&]() {
            it("can create { {0} }", [&]() {
                zdd res = zdd_ithvar(0);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(0,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {42} }", [&]() {
                zdd res = zdd_ithvar(42);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(42,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("zdd_vars", [&]() {
            it("can create { Ø } on empty list", [&]() {
                label_file labels;

                zdd res = zdd_vars(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {42} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 42;
                }

                zdd res = zdd_vars(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(42,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {1,2,5} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 2 << 5;
                }

                zdd res = zdd_vars(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, create_node_ptr(5,MAX_ID))));
                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID, sink_F, create_node_ptr(2,MAX_ID))));

                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(5,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(2,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(1,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("zdd_singletons", [&]() {
            it("can create Ø on empty list", [&]() {
                label_file labels;

                zdd res = zdd_singletons(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {42} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 42;
                }

                zdd res = zdd_singletons(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(42,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {1}, {2}, {5} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 2 << 5;
                }

                zdd res = zdd_singletons(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(5, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID, create_node_ptr(5,MAX_ID), sink_T)));
                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));

                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(5,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(2,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(1,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("zdd_powerset", [&]() {
            it("can create { Ø } on empty list", [&]() {
                label_file labels;

                zdd res = zdd_powerset(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { Ø, {42} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 42;
                }

                zdd res = zdd_powerset(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_T, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(42,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { Ø, {1}, {2}, {5}, {1,2}, {1,5}, {2,5}, {1,2,5} }", [&]() {
                label_file labels;

                { // Garbage collect writer to free write-lock
                  label_writer lw(labels);
                  lw << 1 << 2 << 5;
                }

                zdd res = zdd_powerset(labels);
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

                meta_test_stream<node_t, NODE_FILE_COUNT> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(5,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(2,1u)));
                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(1,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });
          });
      });
  });
