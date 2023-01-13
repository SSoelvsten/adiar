#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/restrict.cpp", []() {
    /*
    //             1         ---- x0
    //            / \
    //            | 2        ---- x1
    //            |/ \
    //            3   4      ---- x2
    //           / \ / \
    //           F T T 5     ---- x3
    //                / \
    //                F T
    */

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    node n5 = node(3,0, terminal_F, terminal_T);
    node n4 = node(2,1, terminal_T, n5.uid());
    node n3 = node(2,0, terminal_F, terminal_T);
    node n2 = node(1,0, n3.uid(), n4.uid());
    node n1 = node(0,0, n3.uid(), n2.uid());

    shared_levelized_file<bdd::node_t> bdd;

    { // Garbage collect writer to free write-lock
      node_writer bdd_w(bdd);
      bdd_w << n5 << n4 << n3 << n2 << n1;
    }

    it("should bridge level [1] Assignment: (_,_,T,_)", [&]() {
      /*
      //                 1      ---- x0
      //                / \
      //                T 2     ---- x1
      //                 / \
      //                 T |
      //                   |
      //                   5    ---- x3
      //                  / \
      //                  F T
      */

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(2, true);
      }

      __bdd out = bdd_restrict(bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n2.uid()), n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n5.uid()), terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
    });

    it("should bridge levels [2]. Assignment: (_,F,_,_)", [&]() {
      /*
      //                 1      ---- x0
      //                / \
      //                | |
      //                \ /
      //                 3      ---- x2
      //                / \
      //                F T
      */

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(1, false);
      }

      __bdd out = bdd_restrict(bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should bridge levels [3]. Assignment: (_,T,_,_)", [&]() {
      /*
      //                  1         ---- x0
      //                 / \
      //                /   \
      //                |   |
      //                3   4       ---- x2
      //               / \ / \
      //               F T T 5      ---- x3
      //                    / \
      //                    F T
      */

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(1, true);
      }

      __bdd out = bdd_restrict(bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n4.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n4.uid()), n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n5.uid()), terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
    });

    it("should remove root. Assignment: (T,_,_,F)", [&]() {
      /*
      //                  2     ---- x1
      //                 / \
      //                /   \
      //                3   4   ---- x2
      //               / \ / \
      //               F T T F
      */

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, true)
           << map_pair<bdd::label_t, assignment>(3, false);
      }

      __bdd out = bdd_restrict(bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2.uid(), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n2.uid()), n4.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n4.uid()), terminal_F }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(2u));
    });

    it("should ignore skipped variables. Assignment: (F,T,_,F)", [&]() {
      /*
      //                 3      ---- x2
      //                / \
      //                F T
      */

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, false)
           << map_pair<bdd::label_t, assignment>(1, true)
           << map_pair<bdd::label_t, assignment>(3, false);
      }

      __bdd out = bdd_restrict(bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should return F terminal. Assignment: (F,_,F,_)", [&]() {
      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, false)
           << map_pair<bdd::label_t, assignment>(2, false);
      }

      __bdd out = bdd_restrict(bdd, ass);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
    });

    it("should return T terminal. Assignment: (T,T,F,_)", [&]() {
      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, true)
           << map_pair<bdd::label_t, assignment>(1, true)
           << map_pair<bdd::label_t, assignment>(2, false);
      }

      __bdd out = bdd_restrict(bdd, ass);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should return input unchanged when given a T terminal", [&]() {
      shared_levelized_file<bdd::node_t> T_file;

      { // Garbage collect writer to free write-lock
        node_writer Tw(T_file);
        Tw << node(true);
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, true)
           << map_pair<bdd::label_t, assignment>(2, true)
           << map_pair<bdd::label_t, assignment>(42, false);
      }

      __bdd out = bdd_restrict(T_file, ass);

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(T_file));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given a F terminal", [&]() {
      shared_levelized_file<bdd::node_t> F_file;

      { // Garbage collect writer to free write-lock
        node_writer Fw(F_file);
        Fw << node(false);
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(2, true)
           << map_pair<bdd::label_t, assignment>(21, true)
           << map_pair<bdd::label_t, assignment>(28, false);
      }

      __bdd out = bdd_restrict(F_file, ass);

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(F_file));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given an empty assignment", [&]() {
      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      __bdd out = bdd_restrict(bdd, ass);

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when assignment that is disjoint of its live variables", [&]() {
      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;
      { // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(5, false)
           << map_pair<bdd::label_t, assignment>(6, true)
           << map_pair<bdd::label_t, assignment>(7, true)
          ;
      }

      __bdd out = bdd_restrict(bdd, ass);

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(bdd));
      AssertThat(out.negate, Is().False());
    });

    it("should have terminal arcs restricted to a terminal sorted [1]", []() {
      /*
      //                    1                 1         ---- x0
      //                   / \              /   \
      //                  2   3     =>     2     3      ---- x1
      //                 / \ / \         /   \  / \
      //                 4 F T F         F*  F  T F     ---- x2
      //                / \
      //                T F              * This arc will be resolved as the last one
      */
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_t> node_input;

      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, terminal_T, terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n4 << n3 << n2 << n1;
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(2, true);
      }

      __bdd out = bdd_restrict(node_input, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n2.uid()), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should have terminal arcs restricted to a terminal sorted [2]", []() {
      /*
      //                    1                _ 1 _
      //                   / \              /     \
      //                  2   3     =>     2       3
      //                 / \ / \         /   \   /   \
      //                 4 F 5 F         F*  F   T*  F
      //                / \ / \
      //                T F F T          * Both these will be resolved out-of-order!
      */
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_t> node_input;

      node n5 = node(2,1, terminal_F, terminal_T);
      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, n5.uid(), terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(node_input);
        inw << n5 << n4 << n3 << n2 << n1;
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(2, true);
      }

      __bdd out = bdd_restrict(node_input, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n2.uid()), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n3.uid()), terminal_F} ));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should skip 'dead' nodes", [&]() {
      /*
      //                        1           ---- x0
      //                      /   \
      //                     2     3        ---- x1
      //                    / \   / \
      //                   4  5   6  7      ---- x2
      //                  / \/ \ / \/ \
      //                  T F  8 F  9 T     ---- x3
      //                      / \  / \
      //                      F T  T F
      //
      //                 Here, node 4 and 6 are going to be dead, when x1 -> T.
      */

      shared_levelized_file<bdd::node_t> dead_bdd;

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

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(1, true);
      }

      __bdd out = bdd_restrict(dead_bdd, ass);

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n1.uid()), n7.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(n5.uid()), n8.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n7.uid(), n9.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n7.uid()), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n8.uid(), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n8.uid()), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n9.uid(), terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { flag(n9.uid()), terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arcs_t>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arcs_t>()->number_of_terminals[true],  Is().EqualTo(3u));
    });

    it("should return terminal-child of restricted root [assignment = T]", [&]() {
      shared_levelized_file<bdd::node_t> terminal_child_of_root_bdd;

      node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
      node n1 = node(1, node::MAX_ID, n2.uid(), terminal_F);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(terminal_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(1, true);
      }

      __bdd out = bdd_restrict(terminal_child_of_root_bdd, ass);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(0u));
    });

    it("should return terminal-child of restricted root [assignment = F]", [&]() {
      shared_levelized_file<bdd::node_t> terminal_child_of_root_bdd;

      node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
      node n1 = node(0, node::MAX_ID, terminal_T, n2.uid());

      { // Garbage collect writer to free write-lock
        node_writer dead_w(terminal_child_of_root_bdd);
        dead_w << n2 << n1;
      }

      adiar::shared_file<map_pair<bdd::label_t, assignment>> ass;

      {  // Garbage collect writer to free write-lock
        adiar::file_writer<map_pair<bdd::label_t, assignment>> aw(ass);
        aw << map_pair<bdd::label_t, assignment>(0, false);
      }

      __bdd out = bdd_restrict(terminal_child_of_root_bdd, ass);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[false], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_t>>()->number_of_terminals[true],  Is().EqualTo(1u));
    });
  });
 });
