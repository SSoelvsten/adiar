go_bandit([]() {
  describe("adiar/bdd/restrict.cpp", []() {
    /*
             1         ---- x0
            / \
            | 2        ---- x1
            |/ \
            3   4      ---- x2
           / \ / \
           F T T 5     ---- x3
                / \
                F T
    */

    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    node_t n5 = create_node(3,0, sink_F, sink_T);
    node_t n4 = create_node(2,1, sink_T, n5.uid);
    node_t n3 = create_node(2,0, sink_F, sink_T);
    node_t n2 = create_node(1,0, n3.uid, n4.uid);
    node_t n1 = create_node(0,0, n3.uid, n2.uid);

    node_file bdd;

    { // Garbage collect writer to free write-lock
      node_writer bdd_w(bdd);
      bdd_w << n5 << n4 << n3 << n2 << n1;
    }

    it("should bridge level [1] Assignment: (_,_,T,_)", [&]() {
      /*
                 1      ---- x0
                / \
                T 2     ---- x1
                 / \
                 T |
                   |
                   5    ---- x3
                  / \
                  F T
      */

      __bdd out = bdd_restrict(bdd, 2, true);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n2.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n5.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n1.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
    });

    it("should bridge levels [2]. Assignment: (_,F,_,_)", [&]() {
      /*
                 1      ---- x0
                / \
                | |
                \ /
                 3      ---- x2
                / \
                F T
      */

      __bdd out = bdd_restrict(bdd, 1, false);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("should bridge levels [3]. Assignment: (_,T,_,_)", [&]() {
      /*
                  1         ---- x0
                 / \
                /   \
                |   |
                3   4       ---- x2
               / \ / \
               F T T 5      ---- x3
                    / \
                    F T
      */

      __bdd out = bdd_restrict(bdd, 1, true);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n4.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n5.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
    });

    it("should remove root. Assignment: (T,_,_,F)", [&]() {
      /*
                  2     ---- x1
                 / \
                /   \
                3   4   ---- x2
               / \ / \
               F T T F
      */

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, true)
           << create_assignment(3, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid, n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n4.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), sink_F }));

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(2u));
    });

    it("should return F sink. Assignment: (F,_,F,_)", [&]() {
      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, false)
           << create_assignment(2, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(0u));
    });

    it("should return T sink. Assignment: (T,T,F,_)", [&]() {
      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, true)
           << create_assignment(1, true)
           << create_assignment(2, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("should return input unchanged when given a T sink", [&]() {
      node_file T_file;

      { // Garbage collect writer to free write-lock
        node_writer Tw(T_file);
        Tw << create_sink(true);
      }

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, true)
           << create_assignment(2, true)
           << create_assignment(42, false);
      }

      __bdd out = bdd_restrict(T_file, assignment);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(T_file._file_ptr));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given a F sink", [&]() {
      node_file F_file;

      { // Garbage collect writer to free write-lock
        node_writer Fw(F_file);
        Fw << create_sink(false);
      }

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(2, true)
           << create_assignment(21, true)
           << create_assignment(28, false);
      }

      __bdd out = bdd_restrict(F_file, assignment);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(F_file._file_ptr));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given an empty assignment", [&]() {
      assignment_file assignment;

      __bdd out = bdd_restrict(bdd, assignment);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd._file_ptr));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when assignment that is disjoint of its live variables", [&]() {
      assignment_file assignment;
      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(5, false)
           << create_assignment(6, true)
           << create_assignment(7, true)
          ;
      }

      __bdd out = bdd_restrict(bdd, assignment);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(bdd._file_ptr));
      AssertThat(out.negate, Is().False());
    });

    it("should have sink arcs restricted to a sink sorted [1]", []() {
      /*
                    1                 1         ---- x0
                   / \              /   \
                  2   3     =>     2     3      ---- x1
                 / \ / \         /   \  / \
                 4 F T F         F*  F  T F     ---- x2
                / \
                T F              * This arc will be resolved as the last one
      */
      ptr_t sink_T = create_sink_ptr(true);
      ptr_t sink_F = create_sink_ptr(false);

      node_file node_input;

      node_t n4 = create_node(2,0, sink_T, sink_F);
      node_t n3 = create_node(1,1, sink_T, sink_F);
      node_t n2 = create_node(1,0, n4.uid, sink_F);
      node_t n1 = create_node(0,0, n2.uid, n3.uid);

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n4 << n3 << n2 << n1;
      }

      __bdd out = bdd_restrict(node_input, 2, true);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n2.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("should have sink arcs restricted to a sink sorted [2]", []() {
      /*
                    1                _ 1 _
                   / \              /     \
                  2   3     =>     2       3
                 / \ / \         /   \   /   \
                 4 F 5 F         F*  F   T*  F
                / \ / \
                T F F T          * Both these will be resolved out-of-order!
      */
      ptr_t sink_T = create_sink_ptr(true);
      ptr_t sink_F = create_sink_ptr(false);

      node_file node_input;

      node_t n5 = create_node(2,1, sink_F, sink_T);
      node_t n4 = create_node(2,0, sink_T, sink_F);
      node_t n3 = create_node(1,1, n5.uid, sink_F);
      node_t n2 = create_node(1,0, n4.uid, sink_F);
      node_t n1 = create_node(0,0, n2.uid, n3.uid);

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n5 << n4 << n3 << n2 << n1;
      }

      __bdd out = bdd_restrict(node_input, 2, true);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n2.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_F} ));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
    });

    it("should skip 'dead' nodes", [&]() {
      /*
                        1           ---- x0
                      /   \
                     2     3        ---- x1
                    / \   / \
                   4  5   6  7      ---- x2
                  / \/ \ / \/ \
                  T F  8 F  9 T     ---- x3
                      / \  / \
                      F T  T F

                 Here, node 4 and 6 are going to be dead, when x1 -> T.
      */

      node_file dead_bdd;

      node_t n9 = create_node(3,1, sink_T, sink_F);
      node_t n8 = create_node(3,0, sink_F, sink_T);
      node_t n7 = create_node(2,3, n9.uid, sink_T);
      node_t n6 = create_node(2,2, sink_T, n9.uid);
      node_t n5 = create_node(2,1, sink_F, n8.uid);
      node_t n4 = create_node(2,0, sink_T, sink_F);
      node_t n3 = create_node(1,1, n6.uid, n7.uid);
      node_t n2 = create_node(1,0, n4.uid, n5.uid);
      node_t n1 = create_node(0,0, n2.uid, n3.uid);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(dead_bdd);
        dead_w << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
      }

      __bdd out = bdd_restrict(dead_bdd, 1, true);

      node_arc_test_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n5.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n7.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), n8.uid }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n7.uid, n9.uid }));

      AssertThat(node_arcs.can_pull(), Is().False());

      sink_arc_test_stream sink_arcs(out);

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n7.uid), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n8.uid, sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n8.uid), sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n9.uid, sink_T }));

      AssertThat(sink_arcs.can_pull(), Is().True());
      AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n9.uid), sink_F }));

      AssertThat(sink_arcs.can_pull(), Is().False());

      level_info_test_stream<arc_t> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
    });

    it("should return sink-child of restricted root [assignment = T]", [&]() {
      node_file sink_child_of_root_bdd;

      node_t n2 = create_node(2,MAX_ID, sink_T, sink_T);
      node_t n1 = create_node(1,MAX_ID, n2.uid, sink_F);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(sink_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      __bdd out = bdd_restrict(sink_child_of_root_bdd, 1, true);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(0u));
    });

    it("should return sink-child of restricted root [assignment = F]", [&]() {
      node_file sink_child_of_root_bdd;

      node_t n2 = create_node(2,MAX_ID, sink_T, sink_T);
      node_t n1 = create_node(0,MAX_ID, sink_T, n2.uid);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(sink_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      __bdd out = bdd_restrict(sink_child_of_root_bdd, 0, false);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(1u));
    });

    describe("bdd_restrict with predicates", [&]() {
      it("should bridge level [1] Assignment: (_,_,T,_) using predicate", [&]() {
        /*
                  1      ---- x0
                  / \
                  T 2     ---- x1
                  / \
                  T |
                    |
                    5    ---- x3
                    / \
                    F T
        */

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 2) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n2.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n1.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("should bridge levels [2]. Assignment: (_,F,_,_) using predicate", [&]() {
        /*
                  1      ---- x0
                  / \
                  | |
                  \ /
                  3      ---- x2
                  / \
                  F T
        */

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 1) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("should bridge levels [3]. Assignment: (_,T,_,_) using predicate", [&]() {
        /*
                    1         ---- x0
                  / \
                  /   \
                  |   |
                  3   4       ---- x2
                / \ / \
                F T T 5      ---- x3
                      / \
                      F T
        */

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 1) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("should remove root. Assignment: (T,_,_,F) using predicate", [&]() {
        /*
                    2     ---- x1
                  / \
                  /   \
                  3   4   ---- x2
                / \ / \
                F T T F
        */

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 0) {
            return substitute_act::FIX_TRUE;
          }
          if(label == 3) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), sink_F }));

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("should return F sink. Assignment: (F,_,F,_) using predicate", [&]() {
        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 0 || label == 2) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("should return T sink. Assignment: (T,T,F,_) using predicate", [&]() {
        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label <= 1) {
            return substitute_act::FIX_TRUE;
          }
          if(label == 2) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("should return input unchanged when given a T sink using predicate", [&]() {
        node_file T_file;

        { // Garbage collect writer to free write-lock
          node_writer Tw(T_file);
          Tw << create_sink(true);
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 0 || label == 2) {
            return substitute_act::FIX_TRUE;
          }
          if(label == 42) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(T_file, pred);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(T_file._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should return input unchanged when given a F sink using predicate", [&]() {
        node_file F_file;

        { // Garbage collect writer to free write-lock
          node_writer Fw(F_file);
          Fw << create_sink(false);
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 2 || label == 21) {
            return substitute_act::FIX_TRUE;
          }
          if(label == 28) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(F_file, pred);

        AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(F_file._file_ptr));
        AssertThat(out.negate, Is().False());
      });

      it("should return input unchanged when given an empty assignment using predicate", [&]() {
        const act_predicate pred = [](label_t /*label*/) -> substitute_act
        {
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n2.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("should return input unchanged when assignment that is disjoint of its live variables using predicate", [&]() {
        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 6 || label == 7) {
            return substitute_act::FIX_TRUE;
          }
          if(label == 5) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n2.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid, n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), n4.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid), n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n4.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("should have sink arcs restricted to a sink sorted [1] using predicate", []() {
        /*
                      1                 1         ---- x0
                    / \              /   \
                    2   3     =>     2     3      ---- x1
                  / \ / \         /   \  / \
                  4 F T F         F*  F  T F     ---- x2
                  / \
                  T F              * This arc will be resolved as the last one
        */
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        node_file node_input;

        node_t n4 = create_node(2,0, sink_T, sink_F);
        node_t n3 = create_node(1,1, sink_T, sink_F);
        node_t n2 = create_node(1,0, n4.uid, sink_F);
        node_t n1 = create_node(0,0, n2.uid, n3.uid);

        { // Garbage collect writer to free write-lock
          node_writer inw(node_input);
          inw << n4 << n3 << n2 << n1;
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 2) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(node_input, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n2.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("should have sink arcs restricted to a sink sorted [2] using predicate", []() {
        /*
                      1                _ 1 _
                    / \              /     \
                    2   3     =>     2       3
                  / \ / \         /   \   /   \
                  4 F 5 F         F*  F   T*  F
                  / \ / \
                  T F F T          * Both these will be resolved out-of-order!
        */
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        node_file node_input;

        node_t n5 = create_node(2,1, sink_F, sink_T);
        node_t n4 = create_node(2,0, sink_T, sink_F);
        node_t n3 = create_node(1,1, n5.uid, sink_F);
        node_t n2 = create_node(1,0, n4.uid, sink_F);
        node_t n1 = create_node(0,0, n2.uid, n3.uid);

        { // Garbage collect writer to free write-lock
          node_writer inw(node_input);
          inw << n5 << n4 << n3 << n2 << n1;
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 2) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(node_input, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n2.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n2.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n3.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n3.uid), sink_F} ));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("should skip 'dead' nodes using predicate", [&]() {
        /*
                          1           ---- x0
                        /   \
                      2     3        ---- x1
                      / \   / \
                    4  5   6  7      ---- x2
                    / \/ \ / \/ \
                    T F  8 F  9 T     ---- x3
                        / \  / \
                        F T  T F

                  Here, node 4 and 6 are going to be dead, when x1 -> T.
        */

        node_file dead_bdd;

        node_t n9 = create_node(3,1, sink_T, sink_F);
        node_t n8 = create_node(3,0, sink_F, sink_T);
        node_t n7 = create_node(2,3, n9.uid, sink_T);
        node_t n6 = create_node(2,2, sink_T, n9.uid);
        node_t n5 = create_node(2,1, sink_F, n8.uid);
        node_t n4 = create_node(2,0, sink_T, sink_F);
        node_t n3 = create_node(1,1, n6.uid, n7.uid);
        node_t n2 = create_node(1,0, n4.uid, n5.uid);
        node_t n1 = create_node(0,0, n2.uid, n3.uid);

        { // Garbage collect writer to free write-lock
          node_writer dead_w(dead_bdd);
          dead_w << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 1) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(dead_bdd, pred);

        node_arc_test_stream node_arcs(out);

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid, n5.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid), n7.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n5.uid), n8.uid }));

        AssertThat(node_arcs.can_pull(), Is().True());
        AssertThat(node_arcs.pull(), Is().EqualTo(arc { n7.uid, n9.uid }));

        AssertThat(node_arcs.can_pull(), Is().False());

        sink_arc_test_stream sink_arcs(out);

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n5.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n7.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n8.uid, sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n8.uid), sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { n9.uid, sink_T }));

        AssertThat(sink_arcs.can_pull(), Is().True());
        AssertThat(sink_arcs.pull(), Is().EqualTo(arc { flag(n9.uid), sink_F }));

        AssertThat(sink_arcs.can_pull(), Is().False());

        level_info_test_stream<arc_t> meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<arc_file>()->number_of_sinks[0], Is().EqualTo(3u));
        AssertThat(out.get<arc_file>()->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("should return sink-child of restricted root [assignment = T] using predicate", [&]() {
        node_file sink_child_of_root_bdd;

        node_t n2 = create_node(2,MAX_ID, sink_T, sink_T);
        node_t n1 = create_node(1,MAX_ID, n2.uid, sink_F);

        { // Garbage collect writer to free write-lock
          node_writer dead_w(sink_child_of_root_bdd);
          dead_w << n2 << n1;
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 1) {
            return substitute_act::FIX_TRUE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(sink_child_of_root_bdd, pred);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("should return sink-child of restricted root [assignment = F] using predicate", [&]() {
        node_file sink_child_of_root_bdd;

        node_t n2 = create_node(2,MAX_ID, sink_T, sink_T);
        node_t n1 = create_node(0,MAX_ID, sink_T, n2.uid);

        { // Garbage collect writer to free write-lock
          node_writer dead_w(sink_child_of_root_bdd);
          dead_w << n2 << n1;
        }

        const act_predicate pred = [](label_t label) -> substitute_act
        {
          if(label == 0) {
            return substitute_act::FIX_FALSE;
          }
          return substitute_act::KEEP;
        };

        __bdd out = bdd_restrict(sink_child_of_root_bdd, pred);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out.get<node_file>()->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out.get<node_file>()->number_of_sinks[1], Is().EqualTo(1u));
      });
    });
  });
});
