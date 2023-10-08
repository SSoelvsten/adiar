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

    shared_levelized_file<bdd::node_type> bdd_1;

    { // Garbage collect writer to free write-lock
      node_writer w(bdd_1);
      w << n5 << n4 << n3 << n2 << n1;
    }

    /*
    //          F
    */
    shared_levelized_file<bdd::node_type> bdd_F;

    { // Garbage collect writer to free write-lock
      node_writer w(bdd_F);
      w << node(false);
    }

    /*
    //          T
    */
    shared_levelized_file<bdd::node_type> bdd_T;

    { // Garbage collect writer to free write-lock
      node_writer w(bdd_T);
      w << node(true);
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

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {2, true} };

      __bdd out = bdd_restrict(exec_policy(), bdd_1, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2.uid(), true,  n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n1.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), true,  terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
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

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {1, false} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), false, n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
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

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {1,true} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), false, n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n4.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n4.uid(), true,  n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), true,  terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
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

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,true}, {3,false} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2.uid(), false, n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n2.uid(), true,  n4.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n4.uid(), true,  terminal_F }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
    });

    it("should ignore skipped variables. Assignment: (F,T,_,F)", [&]() {
      /*
      //                 3      ---- x2
      //                / \
      //                F T
      */

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,false}, {1,true}, {3,false} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_T }));

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,1u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should return F terminal. Assignment: (F,_,F,_)", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,false}, {2,false} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::All], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(0u));
    });

    it("should return T terminal. Assignment: (T,T,F,_)", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,true}, {1,true}, {2,false} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::All], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(1u));
    });

    it("should return input unchanged when given a T terminal", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,true}, {2,true}, {42,false} };

      __bdd out = bdd_restrict(bdd_T, ass.begin(), ass.end());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_T));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given a F terminal", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass  { {2,true}, {21,true}, {28,false} };

      __bdd out = bdd_restrict(bdd_F, ass.begin(), ass.end());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_F));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged when given an empty assignment", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass;

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
      AssertThat(out.negate, Is().False());
    });

    it("should return input unchanged if assignment is disjoint of its variables", [&]() {
      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {5,false}, {6,true}, {7,true} };

      __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
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

      shared_levelized_file<bdd::node_type> in;

      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, terminal_T, terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(in);
        inw << n4 << n3 << n2 << n1;
      }

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {2,true} };

      __bdd out = bdd_restrict(in, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), false, n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
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

      shared_levelized_file<bdd::node_type> in;

      node n5 = node(2,1, terminal_F, terminal_T);
      node n4 = node(2,0, terminal_T, terminal_F);
      node n3 = node(1,1, n5.uid(), terminal_F);
      node n2 = node(1,0, n4.uid(), terminal_F);
      node n1 = node(0,0, n2.uid(), n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer inw(in);
        inw << n5 << n4 << n3 << n2 << n1;
      }

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {2,true} };

      __bdd out = bdd_restrict(in, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), false, n2.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n3.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), true,  terminal_F} ));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
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

      shared_levelized_file<bdd::node_type> in;

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
        node_writer dead_w(in);
        dead_w << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
      }

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {1,true} };

      __bdd out = bdd_restrict(in, ass.begin(), ass.end());

      arc_test_stream arcs(out);

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), false, n5.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n1.uid(), true,  n7.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n5.uid(), true,  n8.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().True());
      AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n7.uid(), false, n9.uid() }));

      AssertThat(arcs.can_pull_internal(), Is().False());

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n7.uid(), true,  terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n8.uid(), false, terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n8.uid(), true,  terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n9.uid(), false, terminal_T }));

      AssertThat(arcs.can_pull_terminal(), Is().True());
      AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n9.uid(), true,  terminal_F }));

      AssertThat(arcs.can_pull_terminal(), Is().False());

      level_info_test_stream meta_arcs(out);

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0,1u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2,2u)));

      AssertThat(meta_arcs.can_pull(), Is().True());
      AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3,2u)));

      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(3u));
      AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
    });

    it("should return terminal-child of restricted root [assignment = T]", [&]() {
      shared_levelized_file<bdd::node_type> in;

      node n2 = node(2, node::max_id, terminal_T, terminal_T);
      node n1 = node(1, node::max_id, n2.uid(), terminal_F);

      { // Garbage collect writer to free write-lock
        node_writer dead_w(in);
        dead_w << n2 << n1;
      }

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {1,true} };

      __bdd out = bdd_restrict(in, ass.begin(), ass.end());

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::All], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(0u));
    });

    it("should return terminal-child of restricted root [assignment = F]", [&]() {
      shared_levelized_file<bdd::node_type> in;

      node n2 = node(2, node::max_id, terminal_T, terminal_T);
      node n1 = node(0, node::max_id, terminal_T, n2.uid());

      { // Garbage collect writer to free write-lock
        node_writer dead_w(in);
        dead_w << n2 << n1;
      }

      std::vector<adiar::pair<bdd::label_type, bool>> ass = { {0,false} };

      __bdd out = bdd_restrict(in, ass.begin(), ass.end());

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream meta_arcs(out);
      AssertThat(meta_arcs.can_pull(), Is().False());

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut::All], Is().EqualTo(1u));

      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(0u));
      AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(1u));
    });
  });
 });
