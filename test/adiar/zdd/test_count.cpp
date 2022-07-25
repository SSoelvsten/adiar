go_bandit([]() {
  describe("adiar/zdd/count.cpp", [&]() {
    ptr_t terminal_T = create_terminal_ptr(true);
    ptr_t terminal_F = create_terminal_ptr(false);

    node_file zdd_1;
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
      node_t n4 = create_node(3,0, terminal_F, terminal_T);
      node_t n3 = create_node(2,0, terminal_F, n4.uid);
      node_t n2 = create_node(1,0, n3.uid, n4.uid);
      node_t n1 = create_node(0,0, n3.uid, n2.uid);

      node_writer nw_1(zdd_1);
      nw_1 << n4 << n3 << n2 << n1;
    }

    node_file zdd_2;
    /*
                     ---- x0

              1      ---- x1
             / \
            2  |     ---- x2
           / \ /
           F  T
    */
    { // Garbage collect writer to free write-lock
      node_t n2 = create_node(2,0, terminal_F, terminal_T);
      node_t n1 = create_node(1,0, n2.uid, terminal_T);

      node_writer nw_2(zdd_2);
      nw_2 << n2 << n1;
    }

    node_file zdd_T;
    /*
          T
    */
    // This describes the set {}

    { // Garbage collect writer to free write-lock
      node_writer nw_T(zdd_T);
      nw_T << create_terminal(true);
    }

    node_file zdd_F;
    /*
          F
    */
    // This describes no set

    { // Garbage collect writer to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_terminal(false);
    }

    node_file zdd_root_1;
    /*
           1    ---- x1
          / \
          F T
    */
    // This describes the set { 1 }

    { // Garbage collect writer to free write-lock
      node_writer nw_root_1(zdd_root_1);
      nw_root_1 << create_node(1,0, terminal_F, terminal_T);
    }

    describe("zdd_nodecount", [&]() {
      it("can count number of nodes", [&]() {
        AssertThat(zdd_nodecount(zdd_1), Is().EqualTo(4u));
        AssertThat(zdd_nodecount(zdd_2), Is().EqualTo(2u));
        AssertThat(zdd_nodecount(zdd_T), Is().EqualTo(0u));
        AssertThat(zdd_nodecount(zdd_F), Is().EqualTo(0u));
        AssertThat(zdd_nodecount(zdd_root_1), Is().EqualTo(1u));
      });
    });

    describe("zdd_varcount", [&]() {
      it("can count number of variables", [&]() {
        AssertThat(zdd_varcount(zdd_1), Is().EqualTo(4u));
        AssertThat(zdd_varcount(zdd_2), Is().EqualTo(2u));
        AssertThat(zdd_varcount(zdd_T), Is().EqualTo(0u));
        AssertThat(zdd_varcount(zdd_F), Is().EqualTo(0u));
        AssertThat(zdd_varcount(zdd_root_1), Is().EqualTo(1u));
      });
    });

    describe("zdd_size", [&]() {
      it("can count family { {x2, x3}, {x0, x2, x3}, {x0, x1, x3} } [1]", [&]() {
        AssertThat(zdd_size(zdd_1), Is().EqualTo(3u));
      });

      it("can count family { {x1}, {x2} } [2]", [&]() {
        AssertThat(zdd_size(zdd_2), Is().EqualTo(2u));
      });

      it("can count family { Ø } [T]", [&]() {
        AssertThat(zdd_size(zdd_T), Is().EqualTo(1u));
      });

      it("can count family Ø [F]", [&]() {
        AssertThat(zdd_size(zdd_F), Is().EqualTo(0u));
      });

      it("can count family { 1 } [root_1]", [&]() {
        AssertThat(zdd_size(zdd_root_1), Is().EqualTo(1u));
      });
    });
  });
 });
