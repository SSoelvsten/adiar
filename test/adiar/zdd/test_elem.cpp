go_bandit([]() {
  describe("adiar/zdd/elem.h", [&]() {
    node_file zdd_F;
    node_file zdd_T;

    { // Garbage collect writers to free write-lock
      node_writer nw_F(zdd_F);
      nw_F << create_sink(false);

      node_writer nw_T(zdd_T);
      nw_T << create_sink(true);
    }

    const ptr_t sink_T = create_sink_ptr(true);
    const ptr_t sink_F = create_sink_ptr(false);

    node_file zdd_1;
    // { { 0 }, { 1 }, { 0,2 }, { 1,2 } }
    /*
            1     ---- x0
           / \
           2 |    ---- x1
          / \|
          F  3    ---- x2
            / \
            T T
     */

    {
      const node_t n3 = create_node(2, MAX_ID, sink_T, sink_T);
      const node_t n2 = create_node(1, MAX_ID, sink_F, n3.uid);
      const node_t n1 = create_node(0, MAX_ID, n2.uid, n3.uid);

      node_writer nw(zdd_1);
      nw << n3 << n2 << n1;
    }

    node_file zdd_2;
    // { Ø, { 1 }, { 2 }, { 2,3 } }
    /*
             1      ---- x1
            / \
            2 T     ---- x2
           / \
           T 3      ---- x3
            / \
            T T

     */

    {
      const node_t n3 = create_node(3, MAX_ID, sink_T, sink_T);
      const node_t n2 = create_node(2, MAX_ID, sink_T, n3.uid);
      const node_t n1 = create_node(1, MAX_ID, n2.uid, sink_T);

      node_writer nw(zdd_2);
      nw << n3 << n2 << n1;
    }

    node_file zdd_3;
    // { { 2,4 }, { 0,2 }, { 0,4 } }
    /*
            _1_      ---- x0
           /   \
           2   3     ---- x2
          / \ / \
          F  4  T    ---- x4
            / \
            F T
     */
    {
      const node_t n4 = create_node(4, MAX_ID,   sink_F, sink_T);
      const node_t n3 = create_node(2, MAX_ID,   n4.uid, sink_T);
      const node_t n2 = create_node(2, MAX_ID-1, sink_F, n4.uid);
      const node_t n1 = create_node(1, MAX_ID,   n2.uid, n3.uid);

      node_writer nw(zdd_3);
      nw << n4 << n3 << n2 << n1;
    }

    node_file zdd_4;
    // { {1}, {0,1} }
    /*
            1      ---- x0
           | |
            2      ---- x1
           / \
           F T

     */
    {
      const node_t n2 = create_node(1, MAX_ID, sink_F, sink_T);
      const node_t n1 = create_node(0, MAX_ID, n2.uid, n2.uid);

      node_writer nw(zdd_4);
      nw << n2 << n1;
    }

    describe("zdd_minelem", [&]() {
      it("finds no element on Ø", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_F);
        AssertThat(result.has_value(), Is().False());
      });

      it("finds empty set on { Ø }", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_T);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());
        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {1} on [1]", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_1);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds empty set on [2]", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_2);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());
        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {2,4} on [3]", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_3);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(2u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(4u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {1} on [4]", [&]() {
        std::optional<label_file> result = zdd_minelem(zdd_4);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().False());
      });
    });

    describe("zdd_maxelem", [&]() {
      it("finds no element on Ø", [&]() {
        std::optional<label_file> result = zdd_maxelem(zdd_F);
        AssertThat(result.has_value(), Is().False());
      });

      it("finds empty set on { Ø }", [&]() {
        std::optional<label_file> result = zdd_maxelem(zdd_T);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());
        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {0,2} on [1]", [&]() {
        std::optional<label_file> result = zdd_maxelem(zdd_1);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(2u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {1} on [2]", [&]() {
        std::optional<label_file> result = zdd_maxelem(zdd_2);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("finds {0,1} on [4]", [&]() {
        std::optional<label_file> result = zdd_maxelem(zdd_4);
        AssertThat(result.has_value(), Is().True());

        label_stream<> ls(result.value());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().False());
      });
    });
  });
 });
