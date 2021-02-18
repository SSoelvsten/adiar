go_bandit([]() {
    describe("BDD: Count", [&]() {
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        node_file bdd_1;
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

        { // Garbage collect writer to free write-lock
          node_t n4 = create_node(3,0, sink_F, sink_T);
          node_t n3 = create_node(2,0, sink_F, n4.uid);
          node_t n2 = create_node(1,0, n3.uid, n4.uid);
          node_t n1 = create_node(0,0, n3.uid, n2.uid);

          node_writer nw_1(bdd_1);
          nw_1 << n4 << n3 << n2 << n1;
        }

        node_file bdd_2;
        /*
                     ---- x0

              1      ---- x1
             / \
            2  |     ---- x2
           / \ /
           F  T
        */

        { // Garbage collect writer to free write-lock
          node_t n2 = create_node(2,0, sink_F, sink_T);
          node_t n1 = create_node(1,0, n2.uid, sink_T);

          node_writer nw_2(bdd_2);
          nw_2 << n2 << n1;
        }

        node_file bdd_3;
        /*
                    ---- x0

             1      ---- x1
            / \
           2  3     ---- x2
          / \/ \
          F T  F
        */

        { // Garbage collect writer to free write-lock
          node_t n3 = create_node(2,1, sink_T, sink_F);
          node_t n2 = create_node(2,0, sink_F, sink_T);
          node_t n1 = create_node(1,0, n2.uid, n3.uid);

          node_writer nw_3(bdd_3);
          nw_3 << n3 << n2 << n1;
        }

        node_file bdd_4;
        /*
                    __1__      ---- x0
                   /     \
                  _2_   _3_    ---- x2
                 /   \ /   \
                 \    4    /   ---- x4
                  \  / \  /
                   \/   \/
                   5    6      ---- x6
                  / \  / \
                  T F  F T
         */

        { // Garbage collect writer to free write-lock
          node_t n6 = create_node(6,1, sink_F, sink_T);
          node_t n5 = create_node(6,0, sink_T, sink_F);
          node_t n4 = create_node(4,0, n5.uid, n6.uid);
          node_t n3 = create_node(2,1, n5.uid, n4.uid);
          node_t n2 = create_node(2,0, n4.uid, n6.uid);
          node_t n1 = create_node(0,0, n2.uid, n3.uid);

          node_writer nw_4(bdd_4);
          nw_4 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        node_file bdd_T;
        /*
              T
         */

        { // Garbage collect writer to free write-lock
          node_writer nw_T(bdd_T);
          nw_T << create_sink(true);
        }

        node_file bdd_F;
        /*
              F
        */

        { // Garbage collect writer to free write-lock
          node_writer nw_F(bdd_F);
          nw_F << create_sink(false);
        }

        node_file bdd_root_1;
        /*
                 1    ---- x1
                / \
                F T
         */

        { // Garbage collect writer to free write-lock
          node_writer nw_root_1(bdd_root_1);
          nw_root_1 << create_node(1,0, sink_F, sink_T);
        }

        describe("nodecount", [&]() {
            it("can count number of nodes", [&]() {
                AssertThat(bdd_nodecount(bdd_1), Is().EqualTo(4u));
                AssertThat(bdd_nodecount(bdd_2), Is().EqualTo(2u));
                AssertThat(bdd_nodecount(bdd_3), Is().EqualTo(3u));
                AssertThat(bdd_nodecount(bdd_4), Is().EqualTo(6u));
                AssertThat(bdd_nodecount(bdd_T), Is().EqualTo(0u));
                AssertThat(bdd_nodecount(bdd_F), Is().EqualTo(0u));
                AssertThat(bdd_nodecount(bdd_root_1), Is().EqualTo(1u));
              });
          });

        describe("varcount", [&]() {
            it("can count number of variables", [&]() {
                AssertThat(bdd_varcount(bdd_1), Is().EqualTo(4u));
                AssertThat(bdd_varcount(bdd_2), Is().EqualTo(2u));
                AssertThat(bdd_varcount(bdd_3), Is().EqualTo(2u));
                AssertThat(bdd_varcount(bdd_4), Is().EqualTo(4u));
                AssertThat(bdd_varcount(bdd_T), Is().EqualTo(0u));
                AssertThat(bdd_varcount(bdd_F), Is().EqualTo(0u));
                AssertThat(bdd_varcount(bdd_root_1), Is().EqualTo(1u));
              });
          });

        describe("pathcount", [&]() {
            it("can count paths leading to T sinks [1]", [&]() {
                AssertThat(bdd_pathcount(bdd_1), Is().EqualTo(3u));
              });

            it("can count paths leading to T sinks [2]", [&]() {
                AssertThat(bdd_pathcount(bdd_2), Is().EqualTo(2u));
              });

            it("can count paths leading to T sinks [3]", [&]() {
                AssertThat(bdd_pathcount(bdd_3), Is().EqualTo(2u));
              });

            it("can count paths leading to T sinks [4]", [&]() {
                AssertThat(bdd_pathcount(bdd_4), Is().EqualTo(6u));
              });

            it("can count paths leading to F sinks [1]", [&]() {
                AssertThat(bdd_pathcount(bdd_not(bdd_1)), Is().EqualTo(5u));
              });

            it("can count paths leading to F sinks [2]", [&]() {
                AssertThat(bdd_pathcount(bdd_not(bdd_2)), Is().EqualTo(1u));
              });

            it("should count no paths in a true sink-only BDD", [&]() {
                AssertThat(bdd_pathcount(bdd_T), Is().EqualTo(0u));
              });

            it("should count no paths in a false sink-only BDD", [&]() {
                AssertThat(bdd_pathcount(bdd_F), Is().EqualTo(0u));
              });

            it("should count paths of a root-only BDD [1]", [&]() {
                AssertThat(bdd_satcount(bdd_root_1), Is().EqualTo(1u));
              });
          });

        describe("satcount", [&]() {
            describe("with varcount", [&]() {
                it("can count assignments leading to T sinks [1]", [&]() {
                    AssertThat(bdd_satcount(bdd_1, 4), Is().EqualTo(5u));
                    AssertThat(bdd_satcount(bdd_1, 5), Is().EqualTo(2 * 5u));
                    AssertThat(bdd_satcount(bdd_1, 6), Is().EqualTo(2 * 2 * 5u));
                  });

                it("can count assignments leading to T sinks [2]", [&]() {
                    AssertThat(bdd_satcount(bdd_2, 2), Is().EqualTo(3u));
                    AssertThat(bdd_satcount(bdd_2, 3), Is().EqualTo(2 * 3u));
                    AssertThat(bdd_satcount(bdd_2, 5), Is().EqualTo(2 * 2 * 2 * 3u));
                  });

                it("can count assignments leading to T sinks [3]", [&]() {
                    AssertThat(bdd_satcount(bdd_3, 2), Is().EqualTo(2u));
                    AssertThat(bdd_satcount(bdd_3, 3), Is().EqualTo(2 * 2u));
                    AssertThat(bdd_satcount(bdd_3, 5), Is().EqualTo(2 * 2 * 2 * 2u));
                  });

                it("can count assignments leading to T sinks [4]", [&]() {
                    AssertThat(bdd_satcount(bdd_4, 4), Is().EqualTo(8u));
                    AssertThat(bdd_satcount(bdd_4, 5), Is().EqualTo(2 * 8u));
                    AssertThat(bdd_satcount(bdd_4, 8), Is().EqualTo(2 * 2 * 2 * 2 * 8u));
                  });

                it("can count assignments leading to F sinks [1]", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_1), 4), Is().EqualTo(11u));
                    AssertThat(bdd_satcount(bdd_not(bdd_1), 5), Is().EqualTo(2 * 11u));
                    AssertThat(bdd_satcount(bdd_not(bdd_1), 6), Is().EqualTo(2 * 2 * 11u));
                  });

                it("can count assignments leading to F sinks [2]", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_2), 2), Is().EqualTo(1u));
                    AssertThat(bdd_satcount(bdd_not(bdd_2), 3), Is().EqualTo(2 * 1u));
                    AssertThat(bdd_satcount(bdd_not(bdd_2), 5), Is().EqualTo(2 * 2 * 2 * 1u));
                  });

                it("should count no assignments to the wrong sink-only BDD", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_T), 5), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(bdd_not(bdd_T), 4), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(bdd_F, 3), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(bdd_F, 2), Is().EqualTo(0u));
                  });

                it("should count all assignments to the desired sink-only BDD", [&]() {
                    AssertThat(bdd_satcount(bdd_T, 5), Is().EqualTo(32u));
                    AssertThat(bdd_satcount(bdd_T, 4), Is().EqualTo(16u));
                    AssertThat(bdd_satcount(bdd_not(bdd_F), 3), Is().EqualTo(8u));
                    AssertThat(bdd_satcount(bdd_not(bdd_F), 2), Is().EqualTo(4u));
                  });
              });

            describe("without varcount", [&]() {
                it("can count assignments leading to T sinks [1]", [&]() {
                    AssertThat(bdd_satcount(bdd_1), Is().EqualTo(5u));
                  });

                it("can count assignments leading to T sinks [2]", [&]() {
                    AssertThat(bdd_satcount(bdd_2), Is().EqualTo(3u));
                  });

                it("can count assignments leading to F sinks [1]", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_1)), Is().EqualTo(11u));
                  });

                it("can count assignments leading to F sinks [2]", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_2)), Is().EqualTo(1u));
                  });

                it("should count no assignments to the true sink-only BDD", [&]() {
                    AssertThat(bdd_satcount(bdd_T), Is().EqualTo(0u));
                  });

                it("should count no assignments in a false sink-only BDD", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_F)), Is().EqualTo(0u));
                    AssertThat(bdd_satcount(bdd_F), Is().EqualTo(0u));
                  });

                it("should count assignments of a root-only BDD [1]", [&]() {
                    AssertThat(bdd_satcount(bdd_not(bdd_root_1)), Is().EqualTo(1u));
                    AssertThat(bdd_satcount(bdd_root_1), Is().EqualTo(1u));
                  });
              });
          });
      });
  });
