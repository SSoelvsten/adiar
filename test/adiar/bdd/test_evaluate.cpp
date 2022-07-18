go_bandit([]() {
  describe("adiar/bdd/evaluate.cpp", []() {
    ptr_t terminal_T = create_terminal_ptr(true);
    ptr_t terminal_F = create_terminal_ptr(false);

    node_file bdd;
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

    node_t n5 = create_node(3,0, terminal_F, terminal_T);
    node_t n4 = create_node(2,1, terminal_T, n5.uid);
    node_t n3 = create_node(2,0, terminal_F, terminal_T);
    node_t n2 = create_node(1,0, n3.uid, n4.uid);
    node_t n1 = create_node(0,0, n3.uid, n2.uid);

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd);
      nw << n5 << n4 << n3 << n2 << n1;
    }

    node_file skip_bdd;
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

    node_t skip_n4 = create_node(4,0, terminal_F, terminal_T);
    node_t skip_n3 = create_node(2,1, terminal_T, skip_n4.uid);
    node_t skip_n2 = create_node(2,0, terminal_F, terminal_T);
    node_t skip_n1 = create_node(0,0, skip_n2.uid, skip_n3.uid);

    { // Garbage collect writer to free write-lock
      node_writer skip_nw(skip_bdd);
      skip_nw << skip_n4 << skip_n3 << skip_n2 << skip_n1;
    }

    node_file non_zero_bdd;
    /*
                   ---- x0

           1       ---- x1
          / \
          F T
    */

    { // Garbage collect writer to free write-lock
      node_writer nw(non_zero_bdd);
      nw << create_node(1,0, terminal_F, terminal_T);
    }

    node_file bdd_F;
    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_F);
      nw << create_terminal(false);
    }

    node_file bdd_T;
    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_T);
      nw << create_terminal(true);
    }

    describe("bdd_eval(bdd, assignment_file)", [&]() {
      it("returns F on test BDD with assignment (F,F,F,T)", [&]() {
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

      it("returns F on test BDD with assignment (F,_,F,T)", [&]() {
        assignment_file assignment;

        { // Garbage collect writer to free write-lock
          assignment_writer aw(assignment);

          aw << create_assignment(0, false)
             << create_assignment(2, false)
             << create_assignment(3, true);
        }

        AssertThat(bdd_eval(bdd, assignment), Is().False());
      });

      it("returns T on test BDD with assignment (F,T,T,T)", [&]() {
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

      it("returns F on test BDD with assignment (T,F,F,T)", [&]() {
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

      it("returns T on test BDD with assignment (T,F,T,F)", [&]() {
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

      it("returns T on test BDD with assignment (T,T,F,T)", [&]() {
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

      it("returns T on test BDD with assignment (T,T,T,F)", [&]() {
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

      it("returns T on test BDD with assignment (T,T,T,T)", [&]() {
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

      it("returns T on BDD with non-zero root with assignment (F,T)", [&]() {
        assignment_file assignment;

        { // Garbage collect writer to free write-lock
          assignment_writer aw(assignment);

          aw << create_assignment(0, false)
             << create_assignment(1, true);
        }

        AssertThat(bdd_eval(non_zero_bdd, assignment), Is().True());
      });

      it("returns F on F terminal-only BDD", [&]() {
        assignment_file assignment;

        { // Garbage collect writer to free write-lock
          assignment_writer aw(assignment);

          aw << create_assignment(0, true)
             << create_assignment(1, false)
             << create_assignment(2, false)
             << create_assignment(3, true);
        }

        AssertThat(bdd_eval(bdd_F, assignment), Is().False());
      });

      it("returns F on F terminal-only BDD with empty assignment", [&]() {
        assignment_file assignment;

        AssertThat(bdd_eval(bdd_F, assignment), Is().False());
      });

      it("returns T on T terminal-only BDD", [&]() {
        assignment_file assignment;

        { // Garbage collect writer to free write-lock
          assignment_writer aw(assignment);

          aw << create_assignment(0, true)
             << create_assignment(1, true)
             << create_assignment(2, false)
             << create_assignment(3, true);
        }

        AssertThat(bdd_eval(bdd_T, assignment), Is().True());
      });

      it("returns T on T terminal-only BDD with empty assignment", [&]() {
        assignment_file assignment;

        AssertThat(bdd_eval(bdd_T, assignment), Is().True());
      });
    });

    describe("bdd_eval(bdd, assignment_func)", [&]() {
      it("returns F on test BDD with assignment 'l -> l = 3'", [&]() {
        assignment_func af = [](const label_t l) { return l == 3; };
        AssertThat(bdd_eval(bdd, af), Is().False());
      });

      it("returns T on test BDD with assignment 'l -> l % 2 == 0'", [&]() {
        assignment_func af = [](const label_t l) { return (l & 1u) == 0; };
        AssertThat(bdd_eval(bdd, af), Is().True());
      });

      it("returns T on test BDD with assignment 'l -> l > 0'", [&]() {
        assignment_func af = [](const label_t l) { return l > 0; };
        AssertThat(bdd_eval(bdd, af), Is().True());
      });

      it("returns F on test BDD with assignment 'l -> l == 0 || l == 3'", [&]() {
        assignment_func af = [](const label_t l) { return l == 0 || l == 3; };
        AssertThat(bdd_eval(bdd, af), Is().False());
      });

      it("returns F on test BDD with assignment 'l -> l % 2 == 1'", [&]() {
        assignment_func af = [](const label_t l) { return (l & 1) == 1; };
        AssertThat(bdd_eval(bdd, af), Is().False());
      });

      it("returns T on test BDD with assignment 'l -> l != 2'", [&]() {
        assignment_func af = [](const label_t l) { return l != 2; };
        AssertThat(bdd_eval(bdd, af), Is().True());
      });

      it("returns F on test BDD with assignment 'l -> l < 3'", [&]() {
        assignment_func af = [](const label_t l) { return l < 3; };
        AssertThat(bdd_eval(bdd, af), Is().False());
      });

      it("returns T on test BDD with assignment '_ -> true'", [&]() {
        assignment_func af = [](const label_t) { return true; };
        AssertThat(bdd_eval(bdd, af), Is().True());
      });

      it("returns F on BDD that skips with assignment 'l -> l == 1 || l > 2'", [&]() {
        assignment_func af = [](const label_t l) { return l == 1 || l > 2; };
        AssertThat(bdd_eval(skip_bdd, af), Is().False());
      });

      it("returns F on BDD that skips with assignment 'l -> l != 1 && l < 4'", [&]() {
        assignment_func af = [](const label_t l) { return l != 1 && l < 4; };
        AssertThat(bdd_eval(skip_bdd, af), Is().False());
      });

      it("returns T on BDD with non-zero root with assignment 'l -> l == 1'", [&]() {
        assignment_func af = [](const label_t l) { return l == 1; };
        AssertThat(bdd_eval(non_zero_bdd, af), Is().True());
      });

      it("returns F on F terminal-only BDD with assignment '_ -> true'", [&]() {
        assignment_func af = [](const label_t) { return true; };
        AssertThat(bdd_eval(bdd_F, af), Is().False());
      });

      it("returns T on T terminal-only BDD with assignment '_ -> false'", [&]() {
        assignment_func af = [](const label_t) { return false; };
        AssertThat(bdd_eval(bdd_T, af), Is().True());
      });
    });
  });
 });
