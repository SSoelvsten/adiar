go_bandit([]() {
    describe("BDD: Count", [&]() {
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

        { // Garbage collect writer to free write-lock
          node_writer nw_1(obdd_1);
          nw_1 << n4 << n3 << n2 << n1;
        }

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

        { // Garbage collect writer to free write-lock
          node_writer nw_2(obdd_2);
          nw_2 << n2_2 << n2_1;
        }

        /*
          ---- x0

             1      ---- x1
            / \
           2  3     ---- x2
          / \/ \
          F T  F
        */

        node_file obdd_3;

        node_t n3_3 = create_node(2,1, sink_T, sink_F);
        node_t n3_2 = create_node(2,0, sink_F, sink_T);
        node_t n3_1 = create_node(1,0, n3_2.uid, n3_3.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw_3(obdd_3);
          nw_3 << n3_3 << n3_2 << n3_1;
        }

        /*
              T
         */
        node_file obdd_T;

        { // Garbage collect writer to free write-lock
          node_writer nw_T(obdd_T);
          nw_T << create_sink(true);
        }

        /*
              F
        */
        node_file obdd_F;

        { // Garbage collect writer to free write-lock
          node_writer nw_F(obdd_F);
          nw_F << create_sink(false);
        }

        /*
                 1    ---- x1
                / \
                F T
         */
        node_file obdd_root_1;

        { // Garbage collect writer to free write-lock
          node_writer nw_root_1(obdd_root_1);
          nw_root_1 << create_node(1,0, sink_F, sink_T);
        }

        /*
                 1    ---- x1
                / \
                F T

          Technically not correct input, but...
        */
        node_file obdd_root_2;

        { // Garbage collect writer to free write-lock
          node_writer nw_root_2(obdd_root_2);
          nw_root_2 << create_node(1,0, sink_T, sink_T);
        }

        describe("nodecount", [&]() {
            it("can count number of nodes", [&]() {
                AssertThat(bdd_nodecount(obdd_1), Is().EqualTo(4u));
                AssertThat(bdd_nodecount(obdd_2), Is().EqualTo(2u));
                AssertThat(bdd_nodecount(obdd_3), Is().EqualTo(3u));
                AssertThat(bdd_nodecount(obdd_T), Is().EqualTo(0u));
                AssertThat(bdd_nodecount(obdd_F), Is().EqualTo(0u));
                AssertThat(bdd_nodecount(obdd_root_1), Is().EqualTo(1u));
                AssertThat(bdd_nodecount(obdd_root_2), Is().EqualTo(1u));
              });
          });

        describe("varcount", [&]() {
            it("can count number of variables", [&]() {
                AssertThat(bdd_varcount(obdd_1), Is().EqualTo(4u));
                AssertThat(bdd_varcount(obdd_2), Is().EqualTo(2u));
                AssertThat(bdd_varcount(obdd_3), Is().EqualTo(2u));
                AssertThat(bdd_varcount(obdd_T), Is().EqualTo(0u));
                AssertThat(bdd_varcount(obdd_F), Is().EqualTo(0u));
                AssertThat(bdd_varcount(obdd_root_1), Is().EqualTo(1u));
                AssertThat(bdd_varcount(obdd_root_2), Is().EqualTo(1u));
              });
          });

        describe("pathcount", [&]() {
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

        describe("satcount", [&]() {
            describe("given [min_label, max_label]", [&]() {
                it("can count assignments leading to T sinks [1]", [&]() {
                    AssertThat(bdd_satcount(obdd_1, 0, 3, is_true), Is().EqualTo(5u));
                    AssertThat(bdd_satcount(obdd_1, 0, 4, is_true), Is().EqualTo(2 * 5u));
                  });

                it("can count assignments leading to T sinks [2]", [&]() {
                    AssertThat(bdd_satcount(obdd_2, 1, 2, is_true), Is().EqualTo(3u));
                    AssertThat(bdd_satcount(obdd_2, 1, 3, is_true), Is().EqualTo(2 * 3u));
                    AssertThat(bdd_satcount(obdd_2, 0, 2, is_true), Is().EqualTo(2 * 3u));
                    AssertThat(bdd_satcount(obdd_2, 0, 3, is_true), Is().EqualTo(2 * 2 * 3u));
                  });

                it("can count assignments leading to F sinks [1]", [&]() {
                    AssertThat(bdd_satcount(obdd_1, 0, 3, is_false), Is().EqualTo(11u));
                    AssertThat(bdd_satcount(obdd_1, 0, 4, is_false), Is().EqualTo(2 * 11u));
                  });

                it("can count assignments leading to F sinks [2]", [&]() {
                    AssertThat(bdd_satcount(obdd_2, 1, 2, is_false), Is().EqualTo(1u));
                    AssertThat(bdd_satcount(obdd_2, 1, 3, is_false), Is().EqualTo(2 * 1u));
                    AssertThat(bdd_satcount(obdd_2, 0, 2, is_false), Is().EqualTo(2 * 1u));
                    AssertThat(bdd_satcount(obdd_2, 0, 3, is_false), Is().EqualTo(2 * 2 * 1u));
                  });


                it("should count no assignments to the wrong sink-only OBDD", [&]() {
                    AssertThat(bdd_satcount(obdd_T, 0, 4, is_false), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_T, 1, 3, is_false), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_F, 0, 2, is_true), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_F, 2, 4, is_true), Is().EqualTo(0u));
                  });

                it("should count all assignments to the desired sink-only OBDD", [&]() {
                    AssertThat(bdd_satcount(obdd_T, 0, 4, is_true), Is().EqualTo(16u));
                    AssertThat(bdd_satcount(obdd_T, 1, 3, is_true), Is().EqualTo(4u));
                    AssertThat(bdd_satcount(obdd_F, 0, 2, is_false), Is().EqualTo(4u));
                    AssertThat(bdd_satcount(obdd_F, 2, 4, is_false), Is().EqualTo(4u));
                  });
              });

            describe("given varcount", [&]() {
                it("can count assignments leading to T sinks [1]", [&]() {
                    AssertThat(bdd_satcount(obdd_1, 4, is_true), Is().EqualTo(5u));
                    AssertThat(bdd_satcount(obdd_1, 5, is_true), Is().EqualTo(2 * 5u));
                    AssertThat(bdd_satcount(obdd_1, 6, is_true), Is().EqualTo(2 * 2 * 5u));
                  });

                it("can count assignments leading to T sinks [2]", [&]() {
                    AssertThat(bdd_satcount(obdd_2, 2, is_true), Is().EqualTo(3u));
                    AssertThat(bdd_satcount(obdd_2, 3, is_true), Is().EqualTo(2 * 3u));
                    AssertThat(bdd_satcount(obdd_2, 5, is_true), Is().EqualTo(2 * 2 * 2 * 3u));
                  });

                it("can count assignments leading to F sinks [1]", [&]() {
                    AssertThat(bdd_satcount(obdd_1, 4, is_false), Is().EqualTo(11u));
                    AssertThat(bdd_satcount(obdd_1, 5, is_false), Is().EqualTo(2 * 11u));
                    AssertThat(bdd_satcount(obdd_1, 6, is_false), Is().EqualTo(2 * 2 * 11u));
                  });

                it("can count assignments leading to F sinks [2]", [&]() {
                    AssertThat(bdd_satcount(obdd_2, 2, is_false), Is().EqualTo(1u));
                    AssertThat(bdd_satcount(obdd_2, 3, is_false), Is().EqualTo(2 * 1u));
                    AssertThat(bdd_satcount(obdd_2, 5, is_false), Is().EqualTo(2 * 2 * 2 * 1u));
                  });

                it("should count no assignments to the wrong sink-only OBDD", [&]() {
                    AssertThat(bdd_satcount(obdd_T, 5, is_false), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_T, 4, is_false), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_F, 3, is_true), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(obdd_F, 2, is_true), Is().EqualTo(0u));
                  });

                it("should count all assignments to the desired sink-only OBDD", [&]() {
                    AssertThat(bdd_satcount(obdd_T, 5, is_true), Is().EqualTo(32u));
                    AssertThat(bdd_satcount(obdd_T, 4, is_true), Is().EqualTo(16u));
                    AssertThat(bdd_satcount(obdd_F, 3, is_false), Is().EqualTo(8u));
                    AssertThat(bdd_satcount(obdd_F, 2, is_false), Is().EqualTo(4u));
                  });
              });

            describe("given nothing", [&]() {
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

                it("should count no assignments to the true sink-only OBDD", [&]() {
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
  });
