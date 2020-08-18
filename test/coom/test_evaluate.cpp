#include <tpie/tpie.h>
#include <coom/evaluate.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Evaluate", [&]() {
        // == CREATE OBDD FOR UNIT TESTS ==
        //               START

        /*
               1
              / \
              | 2
              |/ \
              3   4
             / \ / \
             F T T 5
                  / \
                  F T
        */

        auto sink_T = create_sink(true);
        auto sink_F = create_sink(false);

        tpie::file_stream<node> obdd;
        obdd.open();

        auto n5 = create_node(3,0, sink_F, sink_T);
        obdd.write(n5);

        auto n4 = create_node(2,1, sink_T, n5.node_ptr);
        obdd.write(n4);

        auto n3 = create_node(2,0, sink_F, sink_T);
        obdd.write(n3);

        auto n2 = create_node(1,0, n3.node_ptr, n4.node_ptr);
        obdd.write(n2);

        auto n1 = create_node(0,0, n3.node_ptr, n2.node_ptr);
        obdd.write(n1);

        //                END
        // == CREATE OBDD FOR UNIT TESTS ==

        it("should return F on test OBDD with assignment (F,F,F,T)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(false);
            assignment.write(false);
            assignment.write(false);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd, assignment), Is().False());
          });

        it("should return T on test OBDD with assignment (F,T,T,T)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(false);
            assignment.write(true);
            assignment.write(true);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd, assignment), Is().True());
          });

        it("should return F on test OBDD with assignment (T,F,F,T)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(false);
            assignment.write(false);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd, assignment), Is().False());
          });

        it("should return T on test OBDD with assignment (T,F,T,F)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(false);
            assignment.write(true);
            assignment.write(false);


            AssertThat(coom::evaluate(obdd, assignment), Is().True());
          });

        it("should return T on test OBDD with assignment (T,T,F,T)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(true);
            assignment.write(false);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd, assignment), Is().True());
          });

        it("should return T on test OBDD with assignment (T,T,T,F)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(true);
            assignment.write(true);
            assignment.write(false);


            AssertThat(coom::evaluate(obdd, assignment), Is().False());
          });

        it("should return T on test OBDD with assignment (T,T,T,T)", [&obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(true);
            assignment.write(true);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd, assignment), Is().True());
          });

        // == CREATE 'SKIPPING' OBDD ==
        //             START
        /*
             1      ---- x0
            / \
           /   \    ---- x1
           |   |
           2   3    ---- x2
          / \ / \
          F T T |   ---- x3
                |
                4   ---- x4
               / \
               F T
        */

        tpie::file_stream<node> skip_obdd;
        skip_obdd.open();

        auto skip_n4 = create_node(4,0, sink_F, sink_T);
        skip_obdd.write(skip_n4);

        auto skip_n3 = create_node(2,1, sink_T, skip_n4.node_ptr);
        skip_obdd.write(skip_n3);

        auto skip_n2 = create_node(2,0, sink_F, sink_T);
        skip_obdd.write(skip_n2);

        auto skip_n1 = create_node(0,0, skip_n2.node_ptr, skip_n3.node_ptr);
        skip_obdd.write(skip_n1);

        //              END
        // == CREATE 'SKIPPING' OBDD ==

        it("should be able to evaluate OBDD that skips layers [1]", [&skip_obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(false);
            assignment.write(true);
            assignment.write(false);
            assignment.write(true);
            assignment.write(true);

            AssertThat(coom::evaluate(skip_obdd, assignment), Is().False());
          });

        it("should be able to evaluate OBDD that skips layers [2]", [&skip_obdd]() {
            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(false);
            assignment.write(true);
            assignment.write(true);
            assignment.write(false);

            AssertThat(coom::evaluate(skip_obdd, assignment), Is().False());
          });

        it("should return T on OBDD with non-zero root with assignment (F,T)", [&]() {
            /*
                                 ---- x0

                     1           ---- x1
                    / \
                    F T
             */

            tpie::file_stream<node> non_zero_obdd;
            non_zero_obdd.open();

            auto non_zero_n1 = create_node(1,0, sink_F, sink_T);
            non_zero_obdd.write(non_zero_n1);

            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(false);
            assignment.write(true);

            AssertThat(coom::evaluate(non_zero_obdd, assignment), Is().True());
          });

        it("should return F on F sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd2;
            obdd2.open();

            obdd2.write(create_sink_node(false));

            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(false);
            assignment.write(false);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd2, assignment), Is().False());
          });

        it("should return T on T sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd2;
            obdd2.open();

            obdd2.write(create_sink_node(true));

            tpie::file_stream<bool> assignment;
            assignment.open();

            assignment.write(true);
            assignment.write(true);
            assignment.write(false);
            assignment.write(true);


            AssertThat(coom::evaluate(obdd2, assignment), Is().True());
          });
      });
  });
