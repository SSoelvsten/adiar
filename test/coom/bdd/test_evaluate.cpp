go_bandit([]() {
    describe("BDD: Evaluate", [&]() {
        // == CREATE OBDD FOR UNIT TESTS ==
        //               START

        /*
               1           ---- x0
              / \
              | 2          ---- x1
              |/ \
              3   4        ---- x2
             / \ / \
             F T T 5       ---- x3
                  / \
                  F T
        */

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        node_file obdd;

        node_t n5 = create_node(3,0, sink_F, sink_T);
        node_t n4 = create_node(2,1, sink_T, n5.uid);
        node_t n3 = create_node(2,0, sink_F, sink_T);
        node_t n2 = create_node(1,0, n3.uid, n4.uid);
        node_t n1 = create_node(0,0, n3.uid, n2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw(obdd);
          nw << n5 << n4 << n3 << n2 << n1;
        }

        //                END
        // == CREATE OBDD FOR UNIT TESTS ==

        it("should return F on test OBDD with assignment (F,F,F,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().False());
          });

        it("should return F on test OBDD with assignment (F,_,F,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().False());
           });

        it("should return T on test OBDD with assignment (F,T,T,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().True());
          });

        it("should return F on test OBDD with assignment (T,F,F,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().False());
          });

        it("should return T on test OBDD with assignment (T,F,T,F)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, true)
                 << create_assignment(3, false);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().True());
          });

        it("should return T on test OBDD with assignment (T,T,F,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().True());
          });

        it("should return T on test OBDD with assignment (T,T,T,F)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, false);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().False());
          });

        it("should return T on test OBDD with assignment (T,T,T,T)", [&obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd, assignment), Is().True());
          });

        // == CREATE 'SKIPPING' OBDD ==
        //             START
        /*
             1      ---- x0
            / \
           /   \    ---- x1
           |   |
           2   3    ---- x2
          / \ / \
          F T T |   ---- x3
                |
                4   ---- x4
               / \
               F T
        */

        node_file skip_obdd;

        node_t skip_n4 = create_node(4,0, sink_F, sink_T);
        node_t skip_n3 = create_node(2,1, sink_T, skip_n4.uid);
        node_t skip_n2 = create_node(2,0, sink_F, sink_T);
        node_t skip_n1 = create_node(0,0, skip_n2.uid, skip_n3.uid);

        { // Garbage collect writer to free write-lock
          node_writer skip_nw(skip_obdd);
          skip_nw << skip_n4 << skip_n3 << skip_n2 << skip_n1;
        }

        //              END
        // == CREATE 'SKIPPING' OBDD ==

        it("should be able to evaluate OBDD that skips layers [1]", [&skip_obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true)
                 << create_assignment(2, false)
                 << create_assignment(3, true)
                 << create_assignment(4, true);
            }

            AssertThat(coom::bdd_eval(skip_obdd, assignment), Is().False());
          });

        it("should be able to evaluate OBDD that skips layers [2]", [&skip_obdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, true)
                 << create_assignment(3, true)
                 << create_assignment(4, false);
            }

            AssertThat(coom::bdd_eval(skip_obdd, assignment), Is().False());
          });

        it("should return T on OBDD with non-zero root with assignment (F,T)", [&]() {
            /*
                                 ---- x0

                     1           ---- x1
                    / \
                    F T
             */

            node_file non_zero_obdd;

            { // Garbage collect writer to free write-lock
              node_writer nw(non_zero_obdd);
              nw << create_node(1,0, sink_F, sink_T);
            }

            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true);
            }

            AssertThat(coom::bdd_eval(non_zero_obdd, assignment), Is().True());
          });

        it("should return F on F sink-only OBDD", [&]() {
            node_file obdd2;

            { // Garbage collect writer to free write-lock
              node_writer nw2(obdd2);
              nw2 << create_sink(false);
            }

            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd2, assignment), Is().False());
          });

        it("should return T on T sink-only OBDD", [&]() {
            node_file obdd2;

            { // Garbage collect writer to free write-lock
              node_writer nw2(obdd2);
              nw2 << create_sink(true);
            }

            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(coom::bdd_eval(obdd2, assignment), Is().True());
          });
      });
  });
