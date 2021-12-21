go_bandit([]() {
  describe("BDD: Negate", [&]() {
    node_file sink_T_nf;

    { // Garbage collect writer to free write-lock
      node_writer nw(sink_T_nf);
      nw << create_sink(true);
    }

    bdd sink_T(sink_T_nf);

    it("should negate a T sink-only BDD into an F sink-only BDD [const&]", [&]() {
      bdd out = bdd_not(sink_T);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should negate a T sink-only BDD into an F sink-only BDD [&&]", [&]() {
      bdd out = bdd_not(sink_T_nf);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(false)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should doube-negate a T sink-only BDD back into a T sink-only BDD", [&]() {
      bdd out = bdd_not(bdd_not(sink_T));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    node_file sink_F_nf;

    { // Garbage collect writer to free write-lock
      node_writer nw(sink_F_nf);
      nw << create_sink(false);
    }

    bdd sink_F(sink_F_nf);

    it("should negate a F sink-only BDD into an T sink-only BDD [const&]", [&]() {
      bdd out = bdd_not(sink_F);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should negate a F sink-only BDD into an T sink-only BDD [&&]", [&]() {
      bdd out = bdd_not(sink_F);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should doube-negate an F sink-only BDD back into an F sink-only BDD", [&]() {
      bdd out = bdd_not(bdd_not(sink_T));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    /*
             1
            / \
            | 2
            |/ \
            3  |
            |\ /
            F T
    */

    node_file bdd_1_nf;

    ptr_t sink_T_ptr = create_sink_ptr(true);
    ptr_t sink_F_ptr = create_sink_ptr(false);

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_1_nf);

      nw << create_node(2, MAX_ID, sink_F_ptr, sink_T_ptr)
         << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), sink_T_ptr)
         << create_node(0, MAX_ID, create_node_ptr(2, MAX_ID), create_node_ptr(1, MAX_ID));
    }

    bdd bdd_1(bdd_1_nf);

    it("should negate sink-children in BDD 1 [const&]", [&]() {
      bdd out = bdd_not(bdd_1);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_T_ptr,
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });

    it("should negate sink-children in BDD 1 [&&]", [&]() {
      bdd out = bdd_not(bdd_1_nf);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_T_ptr,
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });

    it("should double-negate sink-children in BDD 1 back to the original [const&, &&]", [&]() {
      // Checkmate, constructivists...

      bdd out = bdd_not(bdd_not(bdd_1));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_F_ptr,
                                                     sink_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });

    it("should double-negate sink-children in BDD 1 back to the original [const&, const&]", [&]() {
      bdd temp = bdd_not(bdd_1);
      bdd out = bdd_not(temp);

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_F_ptr,
                                                     sink_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });

    /*
              1
             / \
            2   3
            |\ /|
             \T/
              6
             / \
             F T
    */

    node_file bdd_2_nf;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_2_nf);

      nw << create_node(2, MAX_ID, sink_F_ptr, sink_T_ptr)
         << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), sink_T_ptr)
         << create_node(1, MAX_ID-1, sink_T_ptr, create_node_ptr(2, MAX_ID))
         << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID-1), create_node_ptr(1, MAX_ID));
    }

    bdd bdd_2(bdd_2_nf);

    it("should negate sink-children in BDD 2 [const&]", [&]() {
      bdd out = bdd_not(bdd_2_nf);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_T_ptr,
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                     sink_F_ptr,
                                                     create_node_ptr(2, MAX_ID))));


      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(1, MAX_ID-1),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });

    it("should negate sink-children in BDD 2 [&&]", [&]() {
      bdd out = bdd_not(bdd_2_nf);

      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                     sink_T_ptr,
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                     create_node_ptr(2, MAX_ID),
                                                     sink_F_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                     sink_F_ptr,
                                                     create_node_ptr(2, MAX_ID))));


      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                     create_node_ptr(1, MAX_ID-1),
                                                     create_node_ptr(1, MAX_ID))));

      AssertThat(ns.can_pull(), Is().False());
    });
  });
 });
