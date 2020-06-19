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
