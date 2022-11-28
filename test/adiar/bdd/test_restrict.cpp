#include "../../test.h"

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

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    node n5 = node(3,0, terminal_F, terminal_T);
    node n4 = node(2,1, terminal_T, n5.uid());
    node n3 = node(2,0, terminal_F, terminal_T);
    node n2 = node(1,0, n3.uid(), n4.uid());
    node n1 = node(0,0, n3.uid(), n2.uid());

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

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(2, true);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n2.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid()), n5.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n1.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n2.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n5.uid()), terminal_T }));

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
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

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(1, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid(), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
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

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(1, true);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid(), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n4.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n4.uid()), n5.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n4.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n5.uid()), terminal_T }));

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
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

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n2.uid(), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n2.uid()), n4.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n4.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n4.uid()), terminal_F }));

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(2u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(2u));
    });

    it("should ignore skipped variables. Assignment: (F,T,_,F)", [&]() {
      /*
                 3      ---- x2
                / \
                F T
      */

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, false)
           << create_assignment(1, true)
           << create_assignment(3, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("should return F terminal. Assignment: (F,_,F,_)", [&]() {
      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, false)
           << create_assignment(2, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      nodeest_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
    });

    it("should return T terminal. Assignment: (T,T,F,_)", [&]() {
      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, true)
           << create_assignment(1, true)
           << create_assignment(2, false);
      }

      __bdd out = bdd_restrict(bdd, assignment);

      nodeest_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("should return input unchanged when given a T terminal", [&]() {
      node_file T_file;

      { // Garbage collect writer to free write-lock
        node_writer Tw(T_file);
        Tw << node(true);
      }

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, true)
           << create_assignment(2, true)
           << create_assignment(42, false);
      }

      __bdd out = bdd_restrict(T_file, assignment);

      AssertThat(out.get<node_file>(), Is().EqualTo(T_file));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given a F terminal", [&]() {
      node_file F_file;

      { // Garbage collect writer to free write-lock
        node_writer Fw(F_file);
        Fw << node(false);
      }

      assignment_file assignment;

      { // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(2, true)
           << create_assignment(21, true)
           << create_assignment(28, false);
      }

      __bdd out = bdd_restrict(F_file, assignment);

      AssertThat(out.get<node_file>(), Is().EqualTo(F_file));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given an empty assignment", [&]() {
      assignment_file assignment;

      __bdd out = bdd_restrict(bdd, assignment);

      AssertThat(out.get<node_file>(), Is().EqualTo(bdd));
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

      AssertThat(out.get<node_file>(), Is().EqualTo(bdd));
      AssertThat(out.negate, Is().False());
    });

    it("should have terminal arcs restricted to a terminal sorted [1]", []() {
      /*
                    1                 1         ---- x0
                   / \              /   \
                  2   3     =>     2     3      ---- x1
                 / \ / \         /   \  / \
                 4 F T F         F*  F  T F     ---- x2
                / \
                T F              * This arc will be resolved as the last one
      */
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      node_file node_input;

      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, terminal_T, terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n4 << n3 << n2 << n1;
      }

      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(2, true);
      }

      __bdd out = bdd_restrict(node_input, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid(), n2.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n2.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n2.uid()), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });

    it("should have terminal arcs restricted to a terminal sorted [2]", []() {
      /*
                    1                _ 1 _
                   / \              /     \
                  2   3     =>     2       3
                 / \ / \         /   \   /   \
                 4 F 5 F         F*  F   T*  F
                / \ / \
                T F F T          * Both these will be resolved out-of-order!
      */
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      node_file node_input;

      node n5 = node(2,1, terminal_F, terminal_T);
      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, n5.uid(), terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n5 << n4 << n3 << n2 << n1;
      }

      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(2, true);
      }

      __bdd out = bdd_restrict(node_input, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid(), n2.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n2.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n2.uid()), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n3.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n3.uid()), terminal_F} ));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(1u));
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

      node n9 = node(3,1, terminal_T, terminal_F);
      node n8 = node(3,0, terminal_F, terminal_T);
      node n7 = node(2,3, n9.uid(), terminal_T);
      node n6 = node(2,2, terminal_T, n9.uid());
      node n5 = node(2,1, terminal_F, n8.uid());
      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, n6.uid(), n7.uid());
      node n2 = node(1,0, n4.uid(), n5.uid());
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer dead_w(dead_bdd);
        dead_w << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
      }

      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(1, true);
      }

      __bdd out = bdd_restrict(dead_bdd, assignment);

      node_arcest_stream node_arcs(out);

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n1.uid(), n5.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n1.uid()), n7.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { flag(n5.uid()), n8.uid() }));

      AssertThat(node_arcs.can_pull(), Is().True());
      AssertThat(node_arcs.pull(), Is().EqualTo(arc { n7.uid(), n9.uid() }));

      AssertThat(node_arcs.can_pull(), Is().False());

      terminal_arcest_stream terminal_arcs(out);

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n7.uid()), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n8.uid(), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n8.uid()), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { n9.uid(), terminal_T }));

      AssertThat(terminal_arcs.can_pull(), Is().True());
      AssertThat(terminal_arcs.pull(), Is().EqualTo(arc { flag(n9.uid()), terminal_F }));

      AssertThat(terminal_arcs.can_pull(), Is().False());

      level_info_test_stream<arc> meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(create_level_info(3,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<arc_file>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<arc_file>()->number_of_terminals[0], Is().EqualTo(3u));
      AssertThat(out.get<arc_file>()->number_of_terminals[1], Is().EqualTo(3u));
    });

    it("should return terminal-child of restricted root [assignment = T]", [&]() {
      node_file terminal_child_of_root_bdd;

      node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
      node n1 = node(1, node::MAX_ID, n2.uid(), terminal_F);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(terminal_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(1, true);
      }

      __bdd out = bdd_restrict(terminal_child_of_root_bdd, assignment);

      nodeest_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(0u));
    });

    it("should return terminal-child of restricted root [assignment = F]", [&]() {
      node_file terminal_child_of_root_bdd;

      node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
      node n1 = node(0, node::MAX_ID, terminal_T, n2.uid());

      { // Garbage collect writer to free write-lock
        node_writer dead_w(terminal_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      assignment_file assignment;

      {  // Garbage collect writer to free write-lock
        assignment_writer aw(assignment);
        aw << create_assignment(0, false);
      }

      __bdd out = bdd_restrict(terminal_child_of_root_bdd, assignment);

      nodeest_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node> meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<node_file>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<node_file>()->number_of_terminals[0], Is().EqualTo(0u));
      AssertThat(out.get<node_file>()->number_of_terminals[1], Is().EqualTo(1u));
    });
  });
 });
