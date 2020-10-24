go_bandit([]() {
    describe("COOM: Negate", [&]() {
        it("should negate a T sink-only OBDD into an F sink-only OBDD", [&]() {
            node_file in_nodes;
            node_writer nw(in_nodes);

            nw << create_sink(true);

            // Negate it
            node_file out_nodes = bdd_not(in_nodes);

            // Check if it is correct
            node_test_stream ns(out_nodes);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
            AssertThat(ns.can_pull(), Is().False());
          });

        it("should negate a F sink-only OBDD into an T sink-only OBDD", [&]() {
            node_file in_nodes;
            node_writer nw(in_nodes);

            nw << create_sink(false);

            // Negate it
            node_file out_nodes = bdd_not(in_nodes);

            // Check if it is correct
            node_test_stream ns(out_nodes);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
            AssertThat(ns.can_pull(), Is().False());
          });

        it("should negate sink-children in OBDD [1]", [&]() {
            /*
                   1
                  / \
                  | 2
                  |/ \
                  3  |
                  |\ /
                  F T
            */

            node_file in_nodes;

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_writer nw(in_nodes);

            nw << create_node(2, MAX_ID, sink_F, sink_T)
               << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), sink_T)
               << create_node(0, MAX_ID, create_node_ptr(2, MAX_ID), create_node_ptr(1, MAX_ID));

            node_file out_nodes = bdd_not(in_nodes);

            // Check if it is correct
            node_test_stream ns(out_nodes);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                           sink_T,
                                                           sink_F)));

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                           create_node_ptr(2, MAX_ID),
                                                           sink_F)));

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                           create_node_ptr(2, MAX_ID),
                                                           create_node_ptr(1, MAX_ID))));

            AssertThat(ns.can_pull(), Is().False());
          });

        it("should negate sink-children in OBDD [2]", [&]() {
            /*
                    1
                   / \
                  2   3
                  |\ /|
                   \T/
                    6
                   / \
                   F T
            */

            node_file in_nodes;

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_writer nw(in_nodes);

            nw << create_node(2, MAX_ID, sink_F, sink_T)
               << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), sink_T)
               << create_node(1, MAX_ID-1, sink_T, create_node_ptr(2, MAX_ID))
               << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID-1), create_node_ptr(1, MAX_ID));

            node_file out_nodes = bdd_not(in_nodes);

            node_test_stream ns(out_nodes);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                           sink_T,
                                                           sink_F)));

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                           create_node_ptr(2, MAX_ID),
                                                           sink_F)));

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                           sink_F,
                                                           create_node_ptr(2, MAX_ID))));


            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                           create_node_ptr(1, MAX_ID-1),
                                                           create_node_ptr(1, MAX_ID))));

            AssertThat(ns.can_pull(), Is().False());
                                                      });
      });
  });
