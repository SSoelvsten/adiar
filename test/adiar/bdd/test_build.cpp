go_bandit([]() {
    describe("BDD: Build", [&]() {
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        describe("bdd_sink", [&]() {
            it("can create true sink only", [&]() {
                bdd res = bdd_sink(true);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create false sink only", [&]() {
                bdd res = bdd_sink(false);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("bdd_ithvar", [&]() {
            it("can create x0", [&]() {
                bdd res = bdd_ithvar(0);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(0, 0, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 0 }));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create x42", [&]() {
                bdd res = bdd_ithvar(42);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, 0, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 42 }));
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("bdd_nithvar", [&]() {
            it("can create !x1", [&]() {
                bdd res = bdd_nithvar(1);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0, sink_T, sink_F)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 1 }));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create !x3", [&]() {
                bdd res = bdd_nithvar(3);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(3, 0, sink_T, sink_F)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 3 }));
                AssertThat(ms.can_pull(), Is().False());
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
                AssertThat(ns.pull(), Is().EqualTo(create_node(5, 0,
                                                               sink_F,
                                                               sink_T)));

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                               sink_F,
                                                               create_node_ptr(5,0))));

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                               sink_F,
                                                               create_node_ptr(2,0))));

                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 5 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 2 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 1 }));

                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create {} as trivially true", [&]() {
                label_file labels;

                bdd res = bdd_and(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
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
                AssertThat(ns.pull(), Is().EqualTo(create_node(5, 0,
                                                                      sink_F,
                                                                      sink_T)));

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(2, 0,
                                                                      create_node_ptr(5,0),
                                                                      sink_T)));

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(1, 0,
                                                                      create_node_ptr(2,0),
                                                                      sink_T)));

                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 5 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 2 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 1 }));

                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create {} as trivially false", [&]() {
                label_file labels;

                bdd res = bdd_or(labels);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("bdd_counter", [&]() {
            it("creates trivially do counting to 10 in [0,8]", [&]() {
                bdd res = bdd_counter(0, 8, 10);
                AssertThat(is_sink(res), Is().True());
                AssertThat(is_sink(res, is_true), Is().True());
              });

            it("creates trivially do counting to 10 in [10,18]", [&]() {
                bdd res = bdd_counter(10, 18, 10);
                AssertThat(is_sink(res), Is().True());
                AssertThat(is_sink(res, is_true), Is().True());
              });

            it("creates counting to 0 in [1,5]", [&]() {
                bdd res = bdd_counter(1, 5, 0);
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

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 5 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 4 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 3 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 2 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 1 }));

                AssertThat(ms.can_pull(), Is().False());
              });

            it("creates counting to 2 in [2,5]", [&]() {
                bdd res = bdd_counter(2, 5, 2);
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

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 5 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 4 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 3 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 2 }));

                AssertThat(ms.can_pull(), Is().False());
              });

            it("creates counting to 3 in [0,8]", [&]() {
                bdd res = bdd_counter(0, 8, 3);
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

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 8 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 7 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 6 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 5 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 4 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 3 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 2 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 1 }));

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(meta_t { 0 }));

                AssertThat(ms.can_pull(), Is().False());
              });
          });
      });
  });
