go_bandit([]() {
    describe("BDD: Evaluate", [&]() {
        // == CREATE BDD FOR UNIT TESTS ==
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

        node_file bdd;

        node_t n5 = create_node(3,0, sink_F, sink_T);
        node_t n4 = create_node(2,1, sink_T, n5.uid);
        node_t n3 = create_node(2,0, sink_F, sink_T);
        node_t n2 = create_node(1,0, n3.uid, n4.uid);
        node_t n1 = create_node(0,0, n3.uid, n2.uid);

        { // Garbage collect writer to free write-lock
          node_writer nw(bdd);
          nw << n5 << n4 << n3 << n2 << n1;
        }

        //                END
        // == CREATE BDD FOR UNIT TESTS ==

        it("should return F on test BDD with assignment (F,F,F,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().False());
          });

        it("should return F on test BDD with assignment (F,_,F,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().False());
           });

        it("should return T on test BDD with assignment (F,T,T,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().True());
          });

        it("should return F on test BDD with assignment (T,F,F,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().False());
          });

        it("should return T on test BDD with assignment (T,F,T,F)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, true)
                 << create_assignment(3, false);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().True());
          });

        it("should return T on test BDD with assignment (T,T,F,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, false)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().True());
          });

        it("should return T on test BDD with assignment (T,T,T,F)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, false);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().False());
          });

        it("should return T on test BDD with assignment (T,T,T,T)", [&bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, true)
                 << create_assignment(2, true)
                 << create_assignment(3, true);
            }

            AssertThat(bdd_eval(bdd, assignment), Is().True());
          });

        // == CREATE 'SKIPPING' BDD ==
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

        node_file skip_bdd;

        node_t skip_n4 = create_node(4,0, sink_F, sink_T);
        node_t skip_n3 = create_node(2,1, sink_T, skip_n4.uid);
        node_t skip_n2 = create_node(2,0, sink_F, sink_T);
        node_t skip_n1 = create_node(0,0, skip_n2.uid, skip_n3.uid);

        { // Garbage collect writer to free write-lock
          node_writer skip_nw(skip_bdd);
          skip_nw << skip_n4 << skip_n3 << skip_n2 << skip_n1;
        }

        //              END
        // == CREATE 'SKIPPING' BDD ==

        it("should be able to evaluate BDD that skips level [1]", [&skip_bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true)
                 << create_assignment(2, false)
                 << create_assignment(3, true)
                 << create_assignment(4, true);
            }

            AssertThat(bdd_eval(skip_bdd, assignment), Is().False());
          });

        it("should be able to evaluate BDD that skips level [2]", [&skip_bdd]() {
            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, true)
                 << create_assignment(1, false)
                 << create_assignment(2, true)
                 << create_assignment(3, true)
                 << create_assignment(4, false);
            }

            AssertThat(bdd_eval(skip_bdd, assignment), Is().False());
          });

        it("should return T on BDD with non-zero root with assignment (F,T)", [&]() {
            /*
                                 ---- x0

                     1           ---- x1
                    / \
                    F T
             */

            node_file non_zero_bdd;

            { // Garbage collect writer to free write-lock
              node_writer nw(non_zero_bdd);
              nw << create_node(1,0, sink_F, sink_T);
            }

            assignment_file assignment;

            { // Garbage collect writer to free write-lock
              assignment_writer aw(assignment);

              aw << create_assignment(0, false)
                 << create_assignment(1, true);
            }

            AssertThat(bdd_eval(non_zero_bdd, assignment), Is().True());
          });

        it("should return F on F sink-only BDD", [&]() {
            node_file bdd2;

            { // Garbage collect writer to free write-lock
              node_writer nw2(bdd2);
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

            AssertThat(bdd_eval(bdd2, assignment), Is().False());
          });

        it("should return T on T sink-only BDD", [&]() {
            node_file bdd2;

            { // Garbage collect writer to free write-lock
              node_writer nw2(bdd2);
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

            AssertThat(bdd_eval(bdd2, assignment), Is().True());
          });
      });
  });
