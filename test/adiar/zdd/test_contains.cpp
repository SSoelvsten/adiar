go_bandit([]() {
  describe("ZDD: Contains", [&]() {
    // == CREATE SINK-ONLY BDD FOR UNIT TESTS ==
    //                  START
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_sink(false);

      node_writer nw_T(zdd_T);
      nw_T << create_sink(true);
    }

    it("returns false for Ø on Ø", [&]() {
      label_file labels;

      AssertThat(zdd_contains(zdd_F, labels), Is().False());
    });

    it("returns true for { Ø } on Ø", [&]() {
      label_file labels;

      AssertThat(zdd_contains(zdd_T, labels), Is().True());
    });

    it("returns false for { Ø } on { 1, 42 }", [&]() {
      label_file labels;

      {
        label_writer lw(labels);
        lw << 1 << 42;
      }

      AssertThat(zdd_contains(zdd_T, labels), Is().False());
    });

    const ptr_t sink_F = create_sink_ptr(false);
    const ptr_t sink_T = create_sink_ptr(true);

    node_file zdd_x0;
    // { { 0 } }
    /*
            1        ---- x0
           / \
           F T
     */
    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_x0);
      nw << create_node(0, MAX_ID, sink_F, sink_T);
    }

    // TODO: tests...

    node_file zdd_x1_null;
    // { Ø, { 1 } }
    /*
            1         ---- x1
           / \
           T T
     */
    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_x1_null);
      nw << create_node(1, MAX_ID, sink_T, sink_T);
    }

    // TODO: tests...

    node_file zdd_1;
    // { Ø, { 0,2 }, { 0,3 } { 1,2 }, { 1,3 }, { 1,2,3 }, { 0,2,3 } }
    /*
                1      ---- x0
               / \
               2  \    ---- x1
              / \ /
              T  3     ---- x2
                / \
               4  5    ---- x3
              / \/ \
              F T  T
     */
    const node_t n1_5 = create_node(3, MAX_ID,   sink_T,   sink_T);
    const node_t n1_4 = create_node(3, MAX_ID-1, sink_F,   sink_T);
    const node_t n1_3 = create_node(2, MAX_ID,   n1_4.uid, n1_5.uid);
    const node_t n1_2 = create_node(1, MAX_ID,   sink_T,   n1_3.uid);
    const node_t n1_1 = create_node(0, MAX_ID,   n1_2.uid, n1_3.uid);

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_1);
      nw << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    it("returns visited root for [1] on Ø", [&]() {
      label_file labels;

      AssertThat(zdd_contains(zdd_1, labels), Is().True());
    });

    it("returns visited sink for [1] on { 0 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    it("returns visited sink for [1] on { 1 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 1;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    it("returns visited sink for [1] on { 0, 2 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0 << 2;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().True());
    });

    it("returns visited sink for [1] on { 1, 3 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 1 << 3;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().True());
    });

    it("returns visited sink for [1] on { 0, 2, 3 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0 << 2 << 3;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().True());
    });

    it("fails on missed label for [1] on { 0, 1, 2 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0 << 1 << 2;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    it("fails on sink with unread labels for [1] on { 2 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    it("fails on sink with unread labels for [1] on { 0, 2, 4 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0 << 2 << 4;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    it("fails on sink with unread labels for [1] on { 0, 2, 3, 4 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 0 << 2 << 3 << 4;
      }

      AssertThat(zdd_contains(zdd_1, labels), Is().False());
    });

    node_file zdd_2;
    // { { 6 }, { 2,4 }, { 2,6 }, { 2,4,6 } }
    /*
               1       ---- x2
              / \
              | 2      ---- x4
              |/ \
              3  4     ---- x6
             / \/ \
             F T  T
     */
    const node_t n2_4 = create_node(6, MAX_ID,   sink_T,   sink_T);
    const node_t n2_3 = create_node(6, MAX_ID-1, sink_F,   sink_T);
    const node_t n2_2 = create_node(4, MAX_ID,   n2_3.uid, n2_4.uid);
    const node_t n2_1 = create_node(2, MAX_ID,   n2_3.uid, n2_2.uid);

    { // Garbage collect writers to free write-lock
      node_writer nw(zdd_2);
      nw << n2_4 << n2_3 << n2_2 << n2_1;
    }

    it("returns visited root for [2] on Ø", [&]() {
      label_file labels;

      AssertThat(zdd_contains(zdd_2, labels), Is().False());
    });

    it("returns visited sink for [2] on { 2 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().False());
    });

    it("returns visited sink for [2] on { 6 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 6;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().True());
    });

    it("returns visited sink for [2] on { 2, 4 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2 << 4;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().True());
    });

    it("returns visited sink for [2] on { 2, 4, 6 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2 << 4 << 6;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().True());
    });

    it("fails on missed label for [2] on { 4, 6 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 4 << 6;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().False());
    });

    it("fails on missed label for [2] on { 2, 3, 4 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2 << 3 << 4;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().False());
    });

    it("fails on missed label for [2] on { 2, 4, 6, 8 }", [&]() {
      label_file labels;

      {
        label_writer w(labels);
        w << 2 << 4 << 6 << 8;
      }

      AssertThat(zdd_contains(zdd_2, labels), Is().False());
    });

    // TODO: reaching sink with yet unseen labels
  });
 });
