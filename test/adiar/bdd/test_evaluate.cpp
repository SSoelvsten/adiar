#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/evaluate.cpp", []() {
    { // bdd_eval
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_t> bdd;
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

      shared_levelized_file<bdd::node_t> skip_bdd;
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

      shared_levelized_file<bdd::node_t> non_zero_bdd;
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

      shared_levelized_file<bdd::node_t> bdd_F;
      { // Garbage collect writer to free write-lock
        node_writer nw(bdd_F);
        nw << node(false);
      }

      shared_levelized_file<bdd::node_t> bdd_T;
      { // Garbage collect writer to free write-lock
        node_writer nw(bdd_T);
        nw << node(true);
      }

      describe("bdd_eval(bdd, adiar::shared_file<assignment>)", [&]() {
        it("returns F on test BDD with assignment (F,F,F,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, false)
               << assignment(1, false)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().False());
        });

        it("returns F on test BDD with assignment (F,_,F,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, false)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().False());
        });

        it("returns T on test BDD with assignment (F,T,T,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, false)
               << assignment(1, true)
               << assignment(2, true)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().True());
        });

        it("returns F on test BDD with assignment (T,F,F,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, false)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().False());
        });

        it("returns T on test BDD with assignment (T,F,T,F)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, false)
               << assignment(2, true)
               << assignment(3, false);
          }

          AssertThat(bdd_eval(bdd, ass), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,F,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, true)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,T,F)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, true)
               << assignment(2, true)
               << assignment(3, false);
          }

          AssertThat(bdd_eval(bdd, ass), Is().False());
        });

        it("returns T on test BDD with assignment (T,T,T,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, true)
               << assignment(2, true)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd, ass), Is().True());
        });

        it("should be able to evaluate BDD that skips level [1]", [&skip_bdd]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, false)
               << assignment(1, true)
               << assignment(2, false)
               << assignment(3, true)
               << assignment(4, true);
          }

          AssertThat(bdd_eval(skip_bdd, ass), Is().False());
        });

        it("should be able to evaluate BDD that skips level [2]", [&skip_bdd]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, false)
               << assignment(2, true)
               << assignment(3, true)
               << assignment(4, false);
          }

          AssertThat(bdd_eval(skip_bdd, ass), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment (F,T)", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, false)
               << assignment(1, true);
          }

          AssertThat(bdd_eval(non_zero_bdd, ass), Is().True());
        });

        it("returns F on F terminal-only BDD", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, false)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd_F, ass), Is().False());
        });

        it("returns F on F terminal-only BDD with empty assignment", [&]() {
          adiar::shared_file<assignment> ass;

          AssertThat(bdd_eval(bdd_F, ass), Is().False());
        });

        it("returns T on T terminal-only BDD", [&]() {
          adiar::shared_file<assignment> ass;

          { // Garbage collect writer to free write-lock
            adiar::file_writer<assignment> aw(ass);

            aw << assignment(0, true)
               << assignment(1, true)
               << assignment(2, false)
               << assignment(3, true);
          }

          AssertThat(bdd_eval(bdd_T, ass), Is().True());
        });

        it("returns T on T terminal-only BDD with empty assignment", [&]() {
          adiar::shared_file<assignment> ass;

          AssertThat(bdd_eval(bdd_T, ass), Is().True());
        });
      });

      describe("bdd_eval(bdd, assignment_func)", [&]() {
        it("returns F on test BDD with assignment 'l -> l = 3'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l == 3);
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l % 2 == 0'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>((l & 1u) == 0);
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns T on test BDD with assignment 'l -> l > 0'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l > 0);
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l == 0 || l == 3'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l == 0 || l == 3);
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns F on test BDD with assignment 'l -> l % 2 == 1'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>((l & 1) == 1);
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l != 2'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l != 2);
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l < 3'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l < 3);
          };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment '_ -> true'", [&]() {
          assignment_func af = [](const bdd::label_t) {
            return static_cast<assignment_value>(true);
          };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on BDD that skips with assignment 'l -> l == 1 || l > 2'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l == 1 || l > 2);
          };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns F on BDD that skips with assignment 'l -> l != 1 && l < 4'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l != 1 && l < 4);
          };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment 'l -> l == 1'", [&]() {
          assignment_func af = [](const bdd::label_t l) {
            return static_cast<assignment_value>(l == 1);
          };
          AssertThat(bdd_eval(non_zero_bdd, af), Is().True());
        });

        it("returns F on F terminal-only BDD with assignment '_ -> true'", [&]() {
          assignment_func af = [](const bdd::label_t) {
            return static_cast<assignment_value>(true);
          };
          AssertThat(bdd_eval(bdd_F, af), Is().False());
        });

        it("returns T on T terminal-only BDD with assignment '_ -> false'", [&]() {
          assignment_func af = [](const bdd::label_t) {
            return static_cast<assignment_value>(false);
          };
          AssertThat(bdd_eval(bdd_T, af), Is().True());
        });
      });
    } // bdd_eval

    { // bdd_satmin, bdd_satmax
      ptr_uint64 terminal_T = ptr_uint64(true);
      ptr_uint64 terminal_F = ptr_uint64(false);

      shared_levelized_file<bdd::node_t> bdd_1;
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

      shared_levelized_file<bdd::node_t> bdd_2;
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

      shared_levelized_file<bdd::node_t> bdd_3;
      /*
      //                      1     ---- x1
      //                     / \
      //                     2  \   ---- x3
      //                    / \ /
      //                   3   4    ---- x5
      //                  / \ / \
      //                  T F F T
      */

      { // Garbage collect writer to free write-lock
        node n4 = node(5,1, terminal_F, terminal_T);
        node n3 = node(5,0, terminal_T, terminal_F);
        node n2 = node(3,0, n3.uid(), n4.uid());
        node n1 = node(1,0, n2.uid(), n4.uid());

        node_writer nw(bdd_3);
        nw << n4 << n3 << n2 << n1;
      }

      describe("bdd_satmin(f)", [&]() {
        it("should retrieve evaluation from true terminal", [&]() {
          shared_levelized_file<bdd::node_t> T;
          {
            node_writer nw(T);
            nw << node(true);
          }

          adiar::shared_file<evaluation> result = bdd_satmin(T);

          adiar::file_stream<evaluation> out_evaluation(result);
          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation from false terminal", [&]() {
          shared_levelized_file<bdd::node_t> F;
          {
            node_writer nw(F);
            nw << node(false);
          }

          adiar::shared_file<evaluation> result = bdd_satmin(false);

          adiar::file_stream<evaluation> out_evaluation(result);
          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [1]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_1);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [1]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_1);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [~1]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_not(bdd_1));
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, false)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [2]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_2);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, false)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [3]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_3);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(5, false)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve evaluation [3]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmin(bdd_not(bdd_3));
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(5, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });
      });

      describe("bdd_satmax(f)", [&]() {
        it("should retrieve maximal evaluation [1]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_1);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve maximal evaluation [~1]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_not(bdd_1));
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve maximal evaluation [2]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_2);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve maximal evaluation [~2]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_not(bdd_2));
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(0, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(2, false)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve maximal evaluation [3]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_3);
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(5, true)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });

        it("should retrieve maximal evaluation [3]", [&]() {
          adiar::shared_file<evaluation> result = bdd_satmax(bdd_not(bdd_3));
          adiar::file_stream<evaluation> out_evaluation(result);

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(1, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(3, true)));

          AssertThat(out_evaluation.can_pull(), Is().True());
          AssertThat(out_evaluation.pull(), Is().EqualTo(evaluation(5, false)));

          AssertThat(out_evaluation.can_pull(), Is().False());
        });
      });
    } // bdd_satmin, bdd_satmax
  });
 });
