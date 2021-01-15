go_bandit([]() {
    describe("BDD: Assignment", []() {
        /* We can definitely improve the output at the cost of performance, but
         * we only need 'some' assignment, not one with the fewest variables set
         * to true etc. */

        it("should retrieve an assignment", [&]() {
            /*
                  1      ---- x0
                 / \
                 2 |     ---- x1
                / \|
               3   4     ---- x2
              / \ / \
              F 5 T F    ---- x3
               / \
               F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n5 = create_node(3,0, sink_F, sink_T);
            node_t n4 = create_node(2,1, sink_T, sink_F);
            node_t n3 = create_node(2,0, sink_F, n5.uid);
            node_t n2 = create_node(1,0, n3.uid, n4.uid);
            node_t n1 = create_node(0,0, n2.uid, n4.uid);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n5 << n4 << n3 << n2 << n1;
            }

            auto result = bdd_get_assignment(obdd, is_true);

            AssertThat(result.has_value(), Is().True());

            assignment_stream<> out_assignment(result.value());

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

            AssertThat(out_assignment.can_pull(), Is().False());
          });

        it("should retrieve an assignment with requested ordering", [&]() {
            /*
                  1
                 / \
                 2 |
                / \|
               3   4
              / \ / \
              F 5 T F
               / \
               F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n5 = create_node(3,0, sink_F, sink_T);
            node_t n4 = create_node(2,1, sink_T, sink_F);
            node_t n3 = create_node(2,0, sink_F, n5.uid);
            node_t n2 = create_node(1,0, n3.uid, n4.uid);
            node_t n1 = create_node(0,0, n2.uid, n4.uid);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n5 << n4 << n3 << n2 << n1;
            }

            auto result = bdd_get_assignment<std::greater<assignment_t>>(obdd, is_true);

            AssertThat(result.has_value(), Is().True());

            assignment_stream<> out_assignment(result.value());

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

            AssertThat(out_assignment.can_pull(), Is().False());
          });

        it("should retrieve an assignment with only relevant labels", [&]() {
            /*
                   1       ---- x0
                  / \
                 2   \     ---- x1
                / \   \
               3   4   5   ---- x2
              / \ / \ / \
              F | T F T |  ---- x3
                \___ ___/
                    6
                   / \
                   F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n6 = create_node(3,0, sink_F, sink_T);
            node_t n5 = create_node(2,2, sink_T, n6.uid);
            node_t n4 = create_node(2,1, sink_T, sink_F);
            node_t n3 = create_node(2,0, sink_F, n6.uid);
            node_t n2 = create_node(1,0, n3.uid, n4.uid);
            node_t n1 = create_node(0,0, n2.uid, n5.uid);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n6 << n5 << n4 << n3 << n2 << n1;
            }

            auto result = bdd_get_assignment(obdd, is_false);

            AssertThat(result.has_value(), Is().True());

            assignment_stream<> out_assignment(result.value());

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, false)));

            AssertThat(out_assignment.can_pull(), Is().False());
          });

        it("should skip nodes until finds one good sink [1]", [&]() {
            /*
                   1     ---- x0
                  / \
                  2 F    ---- x1
                 / \
                 T T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n2 = create_node(1,0, sink_T, sink_T);
            node_t n1 = create_node(0,0, n2.uid, sink_F);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n2 << n1;
            }

            auto result = bdd_get_assignment(obdd, is_false);

            AssertThat(result.has_value(), Is().True());

            assignment_stream<> out_assignment(result.value());

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

            AssertThat(out_assignment.can_pull(), Is().False());
          });

        it("should skip nodes until finds one good sink [2]", [&]() {
            /*
                    _1_             ---- x0
                   /   \
                  _2_  F            ---- x1
                 /   \
                 T  _3_             ---- x2
                   /   \
                   4   5            ---- x3
                  / \ / \
                  F F F F
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n5 = create_node(3,1, sink_F, sink_F);
            node_t n4 = create_node(3,0, sink_F, sink_F);
            node_t n3 = create_node(2,0, n4.uid, n5.uid);
            node_t n2 = create_node(1,0, sink_T, n3.uid);
            node_t n1 = create_node(0,0, n2.uid, sink_F);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n5 << n4 << n3 << n2 << n1;
            }

            auto result = bdd_get_assignment(obdd, is_true);

            AssertThat(result.has_value(), Is().True());

            assignment_stream<> out_assignment(result.value());

            AssertThat(out_assignment.can_pull(), Is().True());
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));
            AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

            AssertThat(out_assignment.can_pull(), Is().False());
          });

        it("should retrieve an empty assignment for sink-only OBDDs", [&]() {
            node_file obdd;

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << create_sink(true);
            }

            auto result = bdd_get_assignment(obdd, is_true);

            AssertThat(result.has_value(), Is().True());
            AssertThat(result.value().size(), Is().EqualTo(0u));
          });

        it("should retrieve failed empty assignment for never-happy predicate", [&]() {
            /*
                  1
                 / \
                 2 T
                / \
                F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            node_file obdd;

            node_t n2 = create_node(1,0,sink_F, sink_T);
            node_t n1 = create_node(0,0,n2.uid, sink_T);

            { // Garbage collect writer to free write-lock
              node_writer nw(obdd);
              nw << n2 << n1;
            }

            auto result = bdd_get_assignment(obdd, [](uint64_t) -> bool { return false; });

            AssertThat(result.has_value(), Is().False());
          });
      });
  });
