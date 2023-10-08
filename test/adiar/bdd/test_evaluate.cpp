#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/evaluate.cpp", []() {
    { // bdd_eval
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_type> bdd;
      /*
      //               1           ---- x0
      //              / \
      //              | 2          ---- x1
      //              |/ \
      //              3   4        ---- x2
      //             / \ / \
      //             F T T 5       ---- x3
      //                  / \
      //                  F T
      */

      node n5 = node(3,0, terminal_F, terminal_T);
      node n4 = node(2,1, terminal_T, n5.uid());
      node n3 = node(2,0, terminal_F, terminal_T);
      node n2 = node(1,0, n3.uid(), n4.uid());
      node n1 = node(0,0, n3.uid(), n2.uid());

      { // Garbage collect writer to free write-lock
        node_writer nw(bdd);
        nw << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> skip_bdd;
      /*
      //             1      ---- x0
      //            / \
      //           /   \    ---- x1
      //           |   |
      //           2   3    ---- x2
      //          / \ / \
      //          F T T |   ---- x3
      //                |
      //                4   ---- x4
      //               / \
      //               F T
      */

      node skip_n4 = node(4,0, terminal_F, terminal_T);
      node skip_n3 = node(2,1, terminal_T, skip_n4.uid());
      node skip_n2 = node(2,0, terminal_F, terminal_T);
      node skip_n1 = node(0,0, skip_n2.uid(), skip_n3.uid());

      { // Garbage collect writer to free write-lock
        node_writer skip_nw(skip_bdd);
        skip_nw << skip_n4 << skip_n3 << skip_n2 << skip_n1;
      }

      shared_levelized_file<bdd::node_type> non_zero_bdd;
      /*
      //                   ---- x0
      //
      //           1       ---- x1
      //          / \
      //          F T
      */

      { // Garbage collect writer to free write-lock
        node_writer nw(non_zero_bdd);
        nw << node(1,0, terminal_F, terminal_T);
      }

      shared_levelized_file<bdd::node_type> bdd_F;
      { // Garbage collect writer to free write-lock
        node_writer nw(bdd_F);
        nw << node(false);
      }

      shared_levelized_file<bdd::node_type> bdd_T;
      { // Garbage collect writer to free write-lock
        node_writer nw(bdd_T);
        nw << node(true);
      }

      describe("bdd_eval(bdd, adiar::generator<...>)", [&]() {
        it("returns F on test BDD with assignment (F,F,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, false},
            {1, false},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns F on test BDD with assignment (F,_,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, false},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (F,T,T,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, false},
            {1, true},
            {2, true},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns F on test BDD with assignment (T,F,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, false},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (T,F,T,F)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, false},
            {2, true},
            {3, false}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,T,F)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true},
            {2, true},
            {3, false}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (T,T,T,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true},
            {2, true},
            {3, true}
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("should be able to evaluate BDD that skips level [1]", [&skip_bdd]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, false},
            {1, true},
            {2, false},
            {3, true},
            {4, true}
          };

          AssertThat(bdd_eval(skip_bdd, ass.begin(), ass.end()), Is().False());
        });

        it("should be able to evaluate BDD that skips level [2]", [&skip_bdd]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, false},
            {2, true},
            {3, true},
            {4, false}
          };

          AssertThat(bdd_eval(skip_bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment (F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, false},
            {1, true}
          };

          AssertThat(bdd_eval(non_zero_bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns F on F terminal-only BDD", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, false},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd_F, ass.begin(), ass.end()), Is().False());
        });

        it("returns F on F terminal-only BDD with empty assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = { };

          AssertThat(bdd_eval(bdd_F, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on T terminal-only BDD", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true},
            {2, false},
            {3, true}
          };

          AssertThat(bdd_eval(bdd_T, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on T terminal-only BDD with empty assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass;

          AssertThat(bdd_eval(bdd_T, ass.begin(), ass.end()), Is().True());
        });

        it("throws exception when given non-ascending list of assignments", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {2, true},
            {1, true},
            {3, true},
            {4, true}
          };

          AssertThrows(invalid_argument, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });

        it("throws exception when running out of assignments", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true}
          };

          AssertThrows(out_of_range, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });

        it("throws exception when list is missing a needed assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            {0, true},
            {1, true},
            {3, true},
            {4, true}
          };

          AssertThrows(invalid_argument, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });
      });

      describe("bdd_eval(bdd, predicate<...>)", [&]() {
        it("returns F on test BDD with assignment 'l -> l = 3'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l == 3;
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l % 2 == 0'", [&]() {
          auto af = [](const bdd::label_type l) {
            return (l & 1u) == 0;
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns T on test BDD with assignment 'l -> l > 0'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l > 0;
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l == 0 || l == 3'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l == 0 || l == 3;
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns F on test BDD with assignment 'l -> l % 2 == 1'", [&]() {
          auto af = [](const bdd::label_type l) {
            return (l & 1) == 1;
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l != 2'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l != 2;
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l < 3'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l < 3;
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment '_ -> true'", [&]() {
          auto af = [](const bdd::label_type) {
            return true;
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on BDD that skips with assignment 'l -> l == 1 || l > 2'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l == 1 || l > 2;
          };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns F on BDD that skips with assignment 'l -> l != 1 && l < 4'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l != 1 && l < 4;
          };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment 'l -> l == 1'", [&]() {
          auto af = [](const bdd::label_type l) {
            return l == 1;
          };
          AssertThat(bdd_eval(non_zero_bdd, af), Is().True());
        });

        it("returns F on F terminal-only BDD with assignment '_ -> true'", [&]() {
          auto af = [](const bdd::label_type) {
            return true;
          };
          AssertThat(bdd_eval(bdd_F, af), Is().False());
        });

        it("returns T on T terminal-only BDD with assignment '_ -> false'", [&]() {
          auto af = [](const bdd::label_type) {
            return false;
          };
          AssertThat(bdd_eval(bdd_T, af), Is().True());
        });
      });
    } // bdd_eval

    { // bdd_satmin, bdd_satmax
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_type> bdd_T;
      {
        node_writer nw(bdd_T);
        nw << node(true);
      }

      shared_levelized_file<bdd::node_type> bdd_F;
      {
        node_writer nw(bdd_F);
        nw << node(false);
      }

      shared_levelized_file<bdd::node_type> bdd_0;
      /*
      //              1      ---- x0
      //             / \
      //             F T
      */
      {
        node_writer nw(bdd_0);
        nw << node(0,0, terminal_F, terminal_T);
      }

      shared_levelized_file<bdd::node_type> bdd_1;
      /*
      //              1      ---- x0
      //             / \
      //             2 |     ---- x1
      //            / \|
      //           3   4     ---- x2
      //          / \ / \
      //          F 5 T F    ---- x3
      //           / \
      //           F T
      */

      {
        node n5 = node(3,0, terminal_F, terminal_T);
        node n4 = node(2,1, terminal_T, terminal_F);
        node n3 = node(2,0, terminal_F, n5.uid());
        node n2 = node(1,0, n3.uid(), n4.uid());
        node n1 = node(0,0, n2.uid(), n4.uid());

        node_writer nw(bdd_1);
        nw << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> bdd_2;
      /*
      //               1       ---- x0
      //              / \
      //             2   \     ---- x1
      //            / \   \
      //           3   4   5   ---- x2
      //          / \ / \ / \
      //          F | T F | T  ---- x3
      //            \__ __/
      //               6
      //              / \
      //              T F
      */

      { // Garbage collect writer to free write-lock
        node n6 = node(3,0, terminal_T, terminal_F);
        node n5 = node(2,2, n6.uid(), terminal_T);
        node n4 = node(2,1, terminal_T, terminal_F);
        node n3 = node(2,0, terminal_F, n6.uid());
        node n2 = node(1,0, n3.uid(), n4.uid());
        node n1 = node(0,0, n2.uid(), n5.uid());

        node_writer nw(bdd_2);
        nw << n6 << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> bdd_3;
      /*
      //                1     ---- x1
      //               / \
      //               2  \   ---- x3
      //              / \ /
      //             3   4    ---- x5
      //            / \ / \
      //            T F F T
      */

      { // Garbage collect writer to free write-lock
        node n4 = node(5,1, terminal_F, terminal_T);
        node n3 = node(5,0, terminal_T, terminal_F);
        node n2 = node(3,0, n3.uid(), n4.uid());
        node n1 = node(1,0, n2.uid(), n4.uid());

        node_writer nw(bdd_3);
        nw << n4 << n3 << n2 << n1;
      }

      describe("bdd_satmin(f) [bdd levels]", [&]() {
        domain_unset();

        it("returns same file for true terminal", [&]() {
          bdd out = bdd_satmin(bdd_T);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
        });

        it("returns same file for false terminal", [&]() {
          bdd out = bdd_satmin(bdd_F);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        });

        it("should retrieve evaluation [0]", [&]() {
          bdd out = bdd_satmin(bdd_0);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve evaluation [1]", [&]() {
          bdd out = bdd_satmin(bdd_1);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         bdd::pointer_type(1, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve evaluation [~1]", [&]() {
          bdd out  = bdd_satmin(bdd_not(bdd_1));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         bdd::pointer_type(1, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve evaluation [2]", [&]() {
          bdd out = bdd_satmin(bdd_2);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         bdd::pointer_type(1, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve evaluation [3]", [&]() {
          bdd out = bdd_satmin(bdd_3);


          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         bdd::pointer_type(5, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("bdd_satmin(f) [domain]", [&]() {
        it("should retrieve evaluation [0] in domain { 0,1,2 }", [&]() {
          domain_set(3);
          bdd out = bdd_satmin(bdd_0);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve evaluation [2] in domain { 0,2,4 }", [&]() {
          std::vector<domain_var> dom = { 0,2,4 };
          domain_set(dom.begin(), dom.end());

          bdd out = bdd_satmin(bdd_2);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         bdd::pointer_type(4, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         bdd::pointer_type(2, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         bdd::pointer_type(1, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        domain_unset();
      });

      describe("bdd_satmin(f, cb) [bdd levels]", [&]() {
        domain_unset();

        it("is never called for true terminal", [&]() {
          size_t calls = 0u;
          const auto cb = [&calls](bdd::label_type, bool) { calls++; };

          bdd_satmin(bdd_T, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("is never called for false terminal", [&]() {
          size_t calls = 0u;
          const auto cb = [&calls](bdd::label_type, bool) { calls++; };

          bdd_satmin(bdd_F, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("is called once for [0]", [&]() {
          size_t calls = 0u;
          const auto cb = [&calls](bdd::label_type x, bool v) {
            AssertThat(calls, Is().EqualTo(0u));
            AssertThat(x, Is().EqualTo(0u));
            AssertThat(v, Is().EqualTo(true));

            calls++;
          };

          bdd_satmin(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(1u));
        });

        it("is called with expected evaluation [1]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, false}, {1, false}, {2, true}, {3, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_1, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("is called with expected evaluation [~1]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, false}, {1, false}, {2, false}, {3, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_not(bdd_1), cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("is called with expected evaluation [2]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, false}, {1, false}, {2, true}, {3, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));

            calls++;
          };

          bdd_satmin(bdd_2, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("is called with expected evaluation [3]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {1, false}, {3, false}, {5, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_3, cb);
          AssertThat(calls, Is().EqualTo(3u));
        });
      });

      describe("bdd_satmin(f, cb) [bdd levels]", [&]() {
        it("is called with full domain for [0]", [&]() {
          domain_set(2);

          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));

            calls++;
          };

          bdd_satmin(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("is called with expected evaluation [2]", [&]() {
          std::vector<bdd::label_type> dom = { 0,2,4 };
          domain_set(dom.begin(), dom.end());

          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, false}, {1, false}, {2, true}, {3, false}, {4, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));

            calls++;
          };

          bdd_satmin(bdd_2, cb);
          AssertThat(calls, Is().EqualTo(5u));
        });
      });

      describe("bdd_satmin(f, begin, end) [bdd levels]", [&]() {
        domain_unset();

        // TODO
      });

      describe("bdd_satmin(f, begin, end) [domain]", [&]() {
        // TODO
      });

      describe("bdd_satmax(f) [bdd levels]", [&]() {
        domain_unset();

        it("should retrieve maximal evaluation [1]", [&]() {
          bdd out = bdd_satmax(bdd_1);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [~1]", [&]() {
          bdd out = bdd_satmax(bdd_not(bdd_1));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [2]", [&]() {
          bdd out = bdd_satmax(bdd_2);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [~2]", [&]() {
          bdd out = bdd_satmax(bdd_not(bdd_2));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [3]", [&]() {
          bdd out = bdd_satmax(bdd_3);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [~3]", [&]() {
          bdd out = bdd_satmax(bdd_not(bdd_3));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("bdd_satmax(f) [domain]", [&]() {
        it("should retrieve maximal evaluation [~1] in domain { 0, 1, ..., 5 }", [&]() {
          domain_set(6);
          bdd out = bdd_satmax(bdd_1);

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(6u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(7u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(7u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(6u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [~2] in domain { 0, 1, ..., 5 }", [&]() {
          domain_set(5);

          bdd out = bdd_satmax(bdd_not(bdd_2));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("should retrieve maximal evaluation [~2] in domain { 0, 2, 4 }", [&]() {
          std::vector<bdd::label_type> dom = { 0,2,4 };
          domain_set(dom.begin(), dom.end());

          bdd out = bdd_satmax(bdd_not(bdd_2));

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id,
                                                         terminal_F,
                                                         terminal_T)));

          // Notice, this variable is outside the domain but on the traversed path!
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id,
                                                         bdd::pointer_type(3, bdd::max_id),
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id,
                                                         terminal_F,
                                                         bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        domain_unset();
      });

      describe("bdd_satmax(f, cb) [bdd levels]", [&]() {
        domain_unset();

        it("should retrieve maximal evaluation [1]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, false}, {3, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_1, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("should retrieve maximal evaluation [~1]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, true}, {3, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_not(bdd_1), cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("should retrieve maximal evaluation [2]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, true}, {3, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_2, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("should retrieve maximal evaluation [~2]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, false}, {3, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_not(bdd_2), cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("should retrieve maximal evaluation [3]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {1, true}, {3, true}, {5, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_3, cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("should retrieve maximal evaluation [~3]", [&]() {
          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {1, true}, {3, true}, {5, false} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_not(bdd_3), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });
      });

      describe("bdd_satmax(f, cb) [domain]", [&]() {
        it("should retrieve maximal evaluation [~1] in domain { 0, 1, ..., 5 }", [&]() {
          domain_set(6);

          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, false}, {3, true}, {4, true}, {5, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_1, cb);
          AssertThat(calls, Is().EqualTo(6u));
        });

        it("should retrieve maximal evaluation [~2] in domain { 0, 1, ..., 5 }", [&]() {
          domain_set(5);

          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {1, true}, {2, false}, {3, true}, {4, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_not(bdd_2), cb);
          AssertThat(calls, Is().EqualTo(5u));
        });

        it("should retrieve maximal evaluation [~2] in domain { 0, 2, 4 }", [&]() {
          std::vector<bdd::label_type> dom = { 0,2,4 };
          domain_set(dom.begin(), dom.end());

          bdd out = bdd_satmax(bdd_not(bdd_2));

          size_t calls = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected =
            { {0, true}, {2, false}, {3, true}, {4, true} };

          const auto cb = [&calls, &expected](bdd::label_type x, bool v) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(x, Is().EqualTo(expected.at(calls).first));
            AssertThat(v, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_not(bdd_2), cb);
          AssertThat(calls, Is().EqualTo(3u + 1u));
        });

        domain_unset();
      });

      describe("bdd_satmax(f, begin, end) [bdd levels]", [&]() {
        domain_unset();

        // TODO
      });

      describe("bdd_satmax(f, begin, end) [domain]", [&]() {
        // TODO
      });

    } // bdd_satmin, bdd_satmax
  });
 });
