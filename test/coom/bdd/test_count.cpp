go_bandit([]() {
    describe("COOM: Count", [&]() {
        /*
             1       ---- x0
            / \
            | 2      ---- x1
            |/ \
            3  |     ---- x2
           / \ /
           F  4      ---- x3
             / \
             F T
        */

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        node_file obdd_1;

        node_t n4 = create_node(3,0, sink_F, sink_T);
        node_t n3 = create_node(2,0, sink_F, n4.uid);
        node_t n2 = create_node(1,0, n3.uid, n4.uid);
        node_t n1 = create_node(0,0, n3.uid, n2.uid);

        node_writer nw_1(obdd_1);
        nw_1 << n4 << n3 << n2 << n1;

        /*
                     ---- x0

              1      ---- x1
             / \
            2  |     ---- x2
           / \ /
           F  T
        */

        node_file obdd_2;

        node_t n2_2 = create_node(2,0, sink_F, sink_T);
        node_t n2_1 = create_node(1,0, n2_2.uid, sink_T);

        node_writer nw_2(obdd_2);
        nw_2 << n2_2 << n2_1;

        /*
              T
         */
        node_file obdd_T;

        node_writer nw_T(obdd_T);
        nw_T << create_sink(true);

        /*
              F
        */
        node_file obdd_F;

        node_writer nw_F(obdd_F);
        nw_F << create_sink(true);

        /*
                 1    ---- x1
                / \
                F T
         */
        node_file obdd_root_1;

        node_writer nw_root_1(obdd_root_1);
        nw_root_1 << create_node(1,0, sink_F, sink_T);

        /*
                 1    ---- x1
                / \
                F T

          Technically not correct input, but...
        */
        node_file obdd_root_2;

        node_writer nw_root_2(obdd_root_2);
        nw_root_2 << create_node(1,0, sink_T, sink_T);

        describe("Paths", [&]() {
            it("can count number of non-disjunct paths", [&]() {
                AssertThat(bdd_pathcount(obdd_1, is_any), Is().EqualTo(8u));
              });

            it("can count paths leading to T sinks [1]", [&]() {
                AssertThat(bdd_pathcount(obdd_1, is_true), Is().EqualTo(3u));
              });

            it("can count paths leading to T sinks [2]", [&]() {
                AssertThat(bdd_pathcount(obdd_2, is_true), Is().EqualTo(2u));
              });

            it("can count paths leading to F sinks [1]", [&]() {
                AssertThat(bdd_pathcount(obdd_1, is_false), Is().EqualTo(5u));
              });

            it("can count paths leading to F sinks [2]", [&]() {
                AssertThat(bdd_pathcount(obdd_2, is_false), Is().EqualTo(1u));
              });

            it("can count paths leading to any sinks [1]", [&]() {
                AssertThat(bdd_pathcount(obdd_1, is_any), Is().EqualTo(8u));
              });

            it("can count paths leading to any sinks [2]", [&]() {
                AssertThat(bdd_pathcount(obdd_2, is_any), Is().EqualTo(3u));
              });

            it("can count paths on a never happy predicate", [&]() {
                auto all_paths_rejected = bdd_pathcount(obdd_1,
                                                        [](uint64_t /* sink */) -> bool {
                                                          return false;
                                                        });

                AssertThat(all_paths_rejected, Is().EqualTo(0u));
              });

            it("should count no paths in a true sink-only OBDD", [&]() {
                AssertThat(bdd_pathcount(obdd_T), Is().EqualTo(0u));
                AssertThat(bdd_pathcount(obdd_T, is_true), Is().EqualTo(0u));
              });

            it("should count no paths in a false sink-only OBDD", [&]() {
                AssertThat(bdd_pathcount(obdd_F), Is().EqualTo(0u));
                AssertThat(bdd_pathcount(obdd_F, is_true), Is().EqualTo(0u));
              });

            it("should count paths of a root-only OBDD [1]", [&]() {
                AssertThat(bdd_satcount(obdd_root_1, is_false), Is().EqualTo(1u));
                AssertThat(bdd_satcount(obdd_root_1, is_true), Is().EqualTo(1u));
              });

            it("should count paths of a root-only OBDD [2]", [&]() {
                AssertThat(bdd_satcount(obdd_root_2, is_false), Is().EqualTo(0u));
                AssertThat(bdd_satcount(obdd_root_2, is_true), Is().EqualTo(2u));
              });
          });

        describe("Assignment", [&]() {
            it("can count assignments leading to T sinks [1]", [&]() {
                AssertThat(bdd_satcount(obdd_1, is_true), Is().EqualTo(5u));
              });

            it("can count assignments leading to T sinks [2]", [&]() {
                AssertThat(bdd_satcount(obdd_2, is_true), Is().EqualTo(3u));
              });

            it("can count assignments leading to F sinks [1]", [&]() {
                AssertThat(bdd_satcount(obdd_1, is_false), Is().EqualTo(11u));
              });

            it("can count assignments leading to F sinks [2]", [&]() {
                AssertThat(bdd_satcount(obdd_2, is_false), Is().EqualTo(1u));
              });

            it("can count assignments leading to any sinks [1]", [&]() {
                AssertThat(bdd_satcount(obdd_1, is_any), Is().EqualTo(16u));
              });

            it("can count assignments leading to any sinks [2]", [&]() {
                AssertThat(bdd_satcount(obdd_2, is_any), Is().EqualTo(4u));
              });

            it("should count no assignments in a true sink-only OBDD", [&]() {
                AssertThat(bdd_satcount(obdd_T, is_false), Is().EqualTo(0u));
                AssertThat(bdd_satcount(obdd_T, is_true), Is().EqualTo(0u));
              });

            it("should count no assignments in a false sink-only OBDD", [&]() {
                AssertThat(bdd_satcount(obdd_F, is_false), Is().EqualTo(0u));
                AssertThat(bdd_satcount(obdd_F, is_true), Is().EqualTo(0u));
              });

            it("should count assignments of a root-only OBDD [1]", [&]() {
                AssertThat(bdd_satcount(obdd_root_1, is_false), Is().EqualTo(1u));
                AssertThat(bdd_satcount(obdd_root_1, is_true), Is().EqualTo(1u));
              });

            it("should count assignments of a root-only OBDD [2]", [&]() {
                AssertThat(bdd_satcount(obdd_root_2, is_false), Is().EqualTo(0u));
                AssertThat(bdd_satcount(obdd_root_2, is_true), Is().EqualTo(2u));
              });
          });
      });
  });
