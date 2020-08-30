#include <coom/assignment.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Assignment", []() {
        /* We can definitely improve the output at the cost of performance, but
         * we only need 'some' assignment, not one with the fewest variables set
         * to true etc. */

        it("is sorted first by label, then by value", [&]() {
            auto a1 = create_assignment(2, false);
            auto a2 = create_assignment(2, true);
            auto a3 = create_assignment(3, false);

            // Less than
            AssertThat(a1 < a2, Is().True());
            AssertThat(a1 < a3, Is().True());
            AssertThat(a2 < a3, Is().True());
            AssertThat(a2 < a1, Is().False());
            AssertThat(a3 < a1, Is().False());
            AssertThat(a3 < a2, Is().False());

            // Greater than
            AssertThat(a2 > a1, Is().True());
            AssertThat(a3 > a1, Is().True());
            AssertThat(a3 > a2, Is().True());
            AssertThat(a1 > a2, Is().False());
            AssertThat(a1 > a3, Is().False());
            AssertThat(a2 > a3, Is().False());


            // Equality
            AssertThat(a1 == a1, Is().True());
            AssertThat(a2 == a2, Is().True());
            AssertThat(a3 == a3, Is().True());
            AssertThat(a2 == a1, Is().False());
            AssertThat(a3 == a1, Is().False());
            AssertThat(a3 == a2, Is().False());

            // Inequality
            AssertThat(a2 != a1, Is().True());
            AssertThat(a3 != a1, Is().True());
            AssertThat(a3 != a2, Is().True());
            AssertThat(a1 != a1, Is().False());
            AssertThat(a2 != a2, Is().False());
            AssertThat(a3 != a3, Is().False());
          });

        it("should retrieve an assignment", [&]() {
            /*
                  1
                 / \
                 2 |
                / \|
               3   4
              / \ / \
              F 5 T F
               / \
               F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            tpie::file_stream<node> obdd;
            obdd.open();

            node n5 = create_node(3,0, sink_F, sink_T);
            obdd.write(n5);

            node n4 = create_node(2,1, sink_T, sink_F);
            obdd.write(n4);

            node n3 = create_node(2,0, sink_F, n5.uid);
            obdd.write(n3);

            node n2 = create_node(1,0, n3.uid, n4.uid);
            obdd.write(n2);

            node n1 = create_node(0,0, n2.uid, n4.uid);
            obdd.write(n1);

            tpie::file_stream<assignment> out_assignment;
            out_assignment.open();

            get_assignment(obdd, is_true, out_assignment);

            out_assignment.seek(0);

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(0, false)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(1, false)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(3, true)));

            AssertThat(out_assignment.can_read(), Is().False());
          });

        it("should retrieve an assignment with requested ordering", [&]() {
            /*
                  1
                 / \
                 2 |
                / \|
               3   4
              / \ / \
              F 5 T F
               / \
               F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            tpie::file_stream<node> obdd;
            obdd.open();

            node n5 = create_node(3,0, sink_F, sink_T);
            obdd.write(n5);

            node n4 = create_node(2,1, sink_T, sink_F);
            obdd.write(n4);

            node n3 = create_node(2,0, sink_F, n5.uid);
            obdd.write(n3);

            node n2 = create_node(1,0, n3.uid, n4.uid);
            obdd.write(n2);

            node n1 = create_node(0,0, n2.uid, n4.uid);
            obdd.write(n1);

            tpie::file_stream<assignment> out_assignment;
            out_assignment.open();

            get_assignment(obdd, is_true, out_assignment, std::greater<assignment>());

            out_assignment.seek(0);

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(3, true)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(1, false)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(0, false)));

            AssertThat(out_assignment.can_read(), Is().False());
          });

        it("should retrieve an assignment with only relevant labels", [&]() {
            /*
                   1
                  / \
                 2   \
                / \   \
               3   4   5
              / \ / \ / \
              F | T F T |
                \___ ___/
                    6
                   / \
                   F T
            */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            tpie::file_stream<node> obdd;
            obdd.open();

            node n6 = create_node(3,0, sink_F, sink_T);
            obdd.write(n6);

            node n5 = create_node(2,2, sink_T, n6.uid);
            obdd.write(n5);

            node n4 = create_node(2,1, sink_T, sink_F);
            obdd.write(n4);

            node n3 = create_node(2,0, sink_F, n6.uid);
            obdd.write(n3);

            node n2 = create_node(1,0, n3.uid, n4.uid);
            obdd.write(n2);

            node n1 = create_node(0,0, n2.uid, n5.uid);
            obdd.write(n1);

            tpie::file_stream<assignment> out_assignment;
            out_assignment.open();

            get_assignment(obdd, is_false, out_assignment);

            out_assignment.seek(0);

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(0, true)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(2, true)));

            AssertThat(out_assignment.can_read(), Is().True());
            AssertThat(out_assignment.read(), Is().EqualTo(create_assignment(3, false)));

            AssertThat(out_assignment.can_read(), Is().False());
          });

        it("should retrieve an empty assignment for sink-only OBDDs", [&]() {
            tpie::file_stream<node> obdd;
            obdd.open();
            obdd.write(create_sink(true));

            tpie::file_stream<assignment> out_assignment;
            out_assignment.open();

            get_assignment(obdd, is_true, out_assignment);

            AssertThat(out_assignment.size(), Is().EqualTo(0));
          });
      });
  });
