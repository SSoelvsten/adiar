go_bandit([]() {
  describe("adiar/builder.h", []() {
    const ptr_t terminal_T = create_terminal_ptr(true);
    const ptr_t terminal_F = create_terminal_ptr(false);

    describe("builder_ptr", [&]() {
      it("supports copy-construction", [&]() {
         bdd_builder b;

         const bdd_ptr p1 = b.add_node(1,false,true);
         const bdd_ptr p2 = p1;

         const bdd_ptr p3 = b.add_node(0,p2,true);

         bdd out = b.build();
         node_test_stream out_nodes(out);

         AssertThat(out_nodes.can_pull(), Is().True());

         AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                               terminal_F,
                                                               terminal_T)));

         AssertThat(out_nodes.can_pull(), Is().True());

         AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                               create_node_ptr(1,MAX_ID),
                                                               terminal_T)));

         AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("supports move-semantics", [&]() {
        bdd_builder b;

        b.add_node(0,b.add_node(1,false,true),true);

        bdd out = b.build();
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(1,MAX_ID),
                                                              terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());
      });
    });

    it("can create a false terminal-only BDD", [&]() {
      bdd_builder b;

      b.add_node(false);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(0u));
    });

    it("can create a true terminal-only BDD", [&]() {
      bdd_builder b;

      b.add_node(true);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("creates the last pushed terminal if no nodes are pushed", [&]() {
      bdd_builder b;

      b.add_node(true);

      b.add_node(false);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(0u));
    });

    it("throws an exception when create is called on an empty file", [&]() {
      bdd_builder b;

      AssertThrows(std::domain_error, b.build());
    });

    it("throws an exception when calling create a second time with no new nodes in between", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);
      b.build();

      AssertThrows(std::domain_error, b.build());
    });

    it("can create a single-node BDD", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                            terminal_F,
                                                            terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("throws an exception if pointers are used from a different builder [1]", [&]() {
      bdd_builder b1;
      bdd_builder b2;

      builder_ptr p = b1.add_node(true);

      AssertThrows(std::invalid_argument, b2.add_node(0,false,p));
    });

    it("throws an exception if pointers are used from a different builder [2]", [&]() {
      bdd_builder b1;
      bdd_builder b2;

      builder_ptr p = b1.add_node(1,true,false);

      AssertThrows(std::invalid_argument, b2.add_node(0,p,false));
    });

    it("throws an exception if pointers are used after reset", [&]() {
      bdd_builder b;

      builder_ptr p = b.add_node(1,true,false);
      b.clear();

      AssertThrows(std::invalid_argument, b.add_node(0,p,false));
    });

    it("throws an exception if pointers are used after create", [&]() {
      bdd_builder b;

      builder_ptr p = b.add_node(1,true,false);
      b.build();

      AssertThrows(std::invalid_argument, b.add_node(0,p,false));
    });

    it("throws an exception when label > MAX_LABEL", [&]() {
      bdd_builder b;

      AssertThrows(std::invalid_argument, b.add_node(MAX_LABEL + 1,false,true));
    });

    it("throws an exception when label > last label", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);

      AssertThrows(std::invalid_argument, b.add_node(1,false,true));
    });

    it("throws an exception when label_of(low) >= label [1]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(0,false,true);

      AssertThrows(std::invalid_argument, b.add_node(0,p,true));
    });

    it("throws an exception when label_of(low) >= label [2]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(3,false,true);

      AssertThrows(std::invalid_argument, b.add_node(3,p,true));
    });

    it("throws an exception when label_of(high) >= label [1]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(0,false,true);

      AssertThrows(std::invalid_argument, b.add_node(0,false,p));
    });

    it("throws an exception when label_of(high) >= label [2]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(6,false,true);

      AssertThrows(std::invalid_argument, b.add_node(6,false,p));
    });

    it("can create nodes on different levels", [&]() {
      /*
                    1      ---- x0
                  / \
                  | 2     ---- x1
                  |/ \
                  3  T    ---- x2
                  / \
                  F T
      */

      bdd_builder b;

      const bdd_ptr p3 = b.add_node(2,false,true);
      const bdd_ptr p2 = b.add_node(1,p3,true);
      const bdd_ptr p1 = b.add_node(0,p3,p2);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            terminal_F,
                                                            terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n2
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                            create_node_ptr(2,MAX_ID),
                                                            terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                            create_node_ptr(2,MAX_ID),
                                                            create_node_ptr(1,MAX_ID))));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(5u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("can create several nodes on the same level", [&]() {
      /*
                        1       ---- x0
                       / \
                       2  3      ---- x1
                      / \/ \
                      4  5  T   ---- x2
                     / \/ \
                     F  T  F
      */

      bdd_builder b;

      const bdd_ptr p5 = b.add_node(2,true,false);
      const bdd_ptr p4 = b.add_node(2,false,true);
      const bdd_ptr p3 = b.add_node(1,p5,true);
      const bdd_ptr p2 = b.add_node(1,p4,p5);
      const bdd_ptr p1 = b.add_node(0,p2,p3);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n5
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n4
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1, terminal_F, terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                            create_node_ptr(2, MAX_ID),
                                                            terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n2
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                            create_node_ptr(2,MAX_ID-1),
                                                            create_node_ptr(2,MAX_ID))));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                            create_node_ptr(1,MAX_ID-1),
                                                            create_node_ptr(1,MAX_ID))));
      AssertThat(out_nodes.can_pull(), Is().False());


      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(5u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(6u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(3u));
    });

    it("can reset and create new nodes", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);

      b.clear();

      b.add_node(1,true,false);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                            terminal_T,
                                                            terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("is empty after reset", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);

      b.clear();

      AssertThrows(std::domain_error, b.build());
    });

    it("can create two different BDDs", [&]() {
      bdd_builder b;

      { // FIRST
        b.add_node(0,false,true);
        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      }

      { // SECOND
        b.add_node(1,true,false);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_T,
                                                              terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      }
    });

    it("throws an exception when there is more than one root [1]", [&]() {
      bdd_builder b;

      b.add_node(0,false,true);
      b.add_node(0,true,false);

      AssertThrows(std::domain_error, b.build());
    });

    it("throws an exception when there is more than one root [2]", [&]() {
      bdd_builder b;

      b.add_node(4,false,true);
      b.add_node(2,true,false);

      AssertThrows(std::domain_error, b.build());
    });

    it("throws an exception when there is more than one root [3]", [&]() {
      bdd_builder b;

      const bdd_ptr p5 = b.add_node(5,true,false);
      const bdd_ptr p4 = b.add_node(4,false,true);
      const bdd_ptr p3 = b.add_node(4,p5,true);
      const bdd_ptr p2 = b.add_node(2,p3,p4);
      const bdd_ptr p1 = b.add_node(1,p3,p5);

      AssertThrows(std::domain_error, b.build());
    });

    it("recognizes copies of nodes", [&]() {
      /*
                         1        ---- x2
                        / \
                        3  4      ---- x4
                       / \/ \
                      5   T  F    ---- x5
                     / \
                     T  F
      */

      bdd_builder b;

      const bdd_ptr p5 = b.add_node(5,true,false);
      const bdd_ptr p4 = b.add_node(4,true,false);
      const bdd_ptr p3 = b.add_node(4,p5,true);
      const bdd_ptr p2 = p3;
      const bdd_ptr p1 = b.add_node(2,p2,p4);

      bdd out = b.build();

      // Check it looks all right
      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n5
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(5, MAX_ID, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n4
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID-1,
                                                            create_node_ptr(5, MAX_ID),
                                                            terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            create_node_ptr(4,MAX_ID-1),
                                                            create_node_ptr(4,MAX_ID))));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(5,1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(4,2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));
      AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(5u));

      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
      AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(6u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(8u));

      AssertThat(out->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out->number_of_terminals[1], Is().EqualTo(3u));
    });

    describe("bdd_builder", [&]() {
      it("uses the BDD reduction rule", [&]() {
        bdd_builder b;

        const bdd_ptr p = b.add_node(1,false,true);

        b.add_node(0,p,p);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can collapse BDD reduction rule to a false terminal", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2,false,false);
        const bdd_ptr p1 = b.add_node(1,false,p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can collapse BDD reduction rule to a true terminal [1]", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2,true,true);
        const bdd_ptr p1 = b.add_node(1,p2,p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can collapse BDD reduction rule to a true terminal [2]", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2,true,true);
        const bdd_ptr p1 = b.add_node(1,p2,true);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("does not decrement 'id' when applying the BDD reduction rule", [&]() {
        bdd_builder b;

        /*
                 _1_      ---- x0
                /   \
                2   3     ---- x1
               / \ //
               F  4       ---- x2
                 / \
                 F T
         */
        const bdd_ptr p4 = b.add_node(2, false, true);
        const bdd_ptr p3 = b.add_node(1, p4, p4);
        const bdd_ptr p2 = b.add_node(1, false, p4);
        const bdd_ptr p1 = b.add_node(0, p3, p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_F,
                                                              create_node_uid(2, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_uid(2, MAX_ID),
                                                              create_node_uid(1, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("uses the BDD reduction rule with copies of nodes", [&]() {
        bdd_builder b;

        const bdd_ptr p1 = b.add_node(1,false,true);
        const bdd_ptr p2 = p1;

        b.add_node(0,p1,p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("does not allow multiple roots when using BDD reduction rule", [&]() {
        bdd_builder b;

        const bdd_ptr p4 = b.add_node(2,true,false);
        const bdd_ptr p3 = b.add_node(2,false,true);
        const bdd_ptr p2 = b.add_node(1,p4,true);
        const bdd_ptr p1 = b.add_node(1,p3,p4); // root
        const bdd_ptr p0 = b.add_node(0,p2,p2); // root

        AssertThrows(std::domain_error, b.build());
      });
    });

    describe("zdd_builder", [&]() {
      it("uses the ZDD reduction rule", [&]() {
        zdd_builder b;

        const zdd_ptr p = b.add_node(1,false,true);

        b.add_node(0,p,false);

        zdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("can collapse ZDD reduction rule to a false terminal", [&]() {
        zdd_builder b;

        const zdd_ptr p2 = b.add_node(2,false,false);
        const zdd_ptr p1 = b.add_node(1,p2,false);

        zdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(0u));
      });

      it("can collapse ZDD reduction rule to a true terminal", [&]() {
        zdd_builder b;

        const zdd_ptr p2 = b.add_node(2,true,false);
        const zdd_ptr p1 = b.add_node(1,p2,false);

        zdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_terminal(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("does not decrement 'id' when applying the ZDD reduction rule", [&]() {
        zdd_builder b;

        /*
                 _1_      ---- x0
                /   \
                2   3     ---- x1
                \\ / \
                  4  F    ---- x2
                 / \
                 F T
         */
        const zdd_ptr p4 = b.add_node(2, false, true);
        const zdd_ptr p3 = b.add_node(1, p4, false);
        const zdd_ptr p2 = b.add_node(1, p4, p4);
        const zdd_ptr p1 = b.add_node(0, p3, p2);

        zdd out = b.build();

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              terminal_F,
                                                              terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_uid(2, MAX_ID),
                                                              create_node_uid(2, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_uid(2, MAX_ID),
                                                              create_node_uid(1, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[0], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[1], Is().EqualTo(1u));
      });

      it("does not allow multiple roots when using ZDD reduction rule", [&]() {
        zdd_builder b;

        const zdd_ptr p4 = b.add_node(2,true,false);
        const zdd_ptr p3 = b.add_node(2,false,true);
        const zdd_ptr p2 = b.add_node(1,p4,true);
        const zdd_ptr p1 = b.add_node(1,p3,p4);    // root
        const zdd_ptr p0 = b.add_node(0,p2,false); // root

        AssertThrows(std::domain_error, b.build());
      });
    });
  });
});
