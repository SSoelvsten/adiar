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

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { Ø } [zdd_null]", [&]() {
                zdd res = zdd_null();
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create Ø [zdd_sink]", [&]() {
                zdd res = zdd_sink(false);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create Ø [zdd_empty]", [&]() {
                zdd res = zdd_empty();
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);
                AssertThat(ms.can_pull(), Is().False());
              });
          });

        describe("zdd_ithvar", [&]() {
            it("can create { {x0} }", [&]() {
                zdd res = zdd_ithvar(0);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(0,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });

            it("can create { {x42} }", [&]() {
                zdd res = zdd_ithvar(42);
                node_test_stream ns(res);

                AssertThat(ns.can_pull(), Is().True());
                AssertThat(ns.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
                AssertThat(ns.can_pull(), Is().False());

                meta_test_stream<node_t, 1> ms(res);

                AssertThat(ms.can_pull(), Is().True());
                AssertThat(ms.pull(), Is().EqualTo(create_meta(42,1u)));
                AssertThat(ms.can_pull(), Is().False());
              });
          });
      });
  });
