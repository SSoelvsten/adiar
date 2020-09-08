#include <tpie/tpie.h>

#include <coom/reduce.cpp>
#include <coom/pred.cpp>
#include <coom/restrict.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Restrict", [&]() {
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

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        tpie::file_stream<node_t> obdd;
        obdd.open();

        node_t n5 = create_node(3,0, sink_F, sink_T);
        obdd.write(n5);

        node_t n4 = create_node(2,1, sink_T, n5.uid);
        obdd.write(n4);

        node_t n3 = create_node(2,0, sink_F, sink_T);
        obdd.write(n3);

        node_t n2 = create_node(1,0, n3.uid, n4.uid);
        obdd.write(n2);

        node_t n1 = create_node(0,0, n3.uid, n2.uid);
        obdd.write(n1);

        //                END
        // == CREATE OBDD FOR UNIT TESTS ==

        it("should bridge layers [1]. Assignment: (_,_,T,_)", [&]() {
            /*
                 1
                / \
                T 2
                 / \
                 T |
                   |
                   5
                  / \
                  F T
            */

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(2, true));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n2.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n2.uid), n5.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n1.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n2.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n5.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n5.uid), sink_T }));
          });

        it("should bridge layers. [2]. Assignment: (_,F,_,_)", [&]() {
            /*
                 1
                / \
                | |
                \ /
                 3
                / \
                F T
            */

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(1, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n1.uid, n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_T }));
          });

        it("should bridge layers [3]. Assignment: (_,T,_,_)", [&]() {
            /*
                  1
                 / \
                /   \
                |   |
                3   4
               / \ / \
               F T T 5
                    / \
                    F T
            */


            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(1, true));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n1.uid, n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n4.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n4.uid), n5.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n4.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n5.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n5.uid), sink_T }));
          });

        it("should remove root. Assignment: (T,_,_,F)", [&]() {
            /*
                  2
                 / \
                /   \
                3   4
               / \ / \
               F T T F
            */

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(0, true));
            assignment.write(create_assignment(3, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n2.uid, n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n2.uid), n4.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n4.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n4.uid), sink_F }));
          });

        it("should ignore skipped variables. Assignment: (F,T,_,F)", [&]() {
            /*
                 3
                / \
                F T
            */

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(0, false));
            assignment.write(create_assignment(1, true));
            assignment.write(create_assignment(3, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_T }));
          });

        it("should return F sink. Assignment: (F,_,F,_)", [&obdd]() {
            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(0, false));
            assignment.write(create_assignment(2, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(reduce_node_arcs.size(), Is().EqualTo(0u));
            AssertThat(reduce_sink_arcs.size(), Is().EqualTo(0u));

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return T sink. Assignment: (T,T,F,_)", [&obdd]() {
            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(0, true));
            assignment.write(create_assignment(1, true));
            assignment.write(create_assignment(2, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(reduce_node_arcs.size(), Is().EqualTo(0u));
            AssertThat(reduce_sink_arcs.size(), Is().EqualTo(0u));

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return T sink given a T sink", [&]() {
            tpie::file_stream<node_t> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink(true));

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(0, true));
            assignment.write(create_assignment(2, true));
            assignment.write(create_assignment(42, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            restrict(in_nodes, assignment, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return F sink given a F sink", [&]() {
            tpie::file_stream<node_t> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink(false));

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(2, true));
            assignment.write(create_assignment(21, true));
            assignment.write(create_assignment(28, false));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            restrict(in_nodes, assignment, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return input unchanged when given an empty assignment", [&]() {
            tpie::file_stream<assignment> assignment;
            assignment.open();

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            restrict(obdd, assignment, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(n5));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(n4));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(n3));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(n2));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(n1));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should have sink arcs restricted to a sink sorted [1]", []() {
            /*
                    1                 1
                   / \              /   \
                  2   3     =>     2     3
                 / \ / \         /   \  / \
                 4 F T F         F*  F  T F
                / \
                T F              * This arc will be resolved as the last one
             */
            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            tpie::file_stream<node_t> in_nodes;
            in_nodes.open();

            node_t n4 = create_node(2,0, sink_T, sink_F);
            in_nodes.write(n4);

            node_t n3 = create_node(1,1, sink_T, sink_F);
            in_nodes.write(n3);

            node_t n2 = create_node(1,0, n4.uid, sink_F);
            in_nodes.write(n2);

            node_t n1 = create_node(0,0, n2.uid, n3.uid);
            in_nodes.write(n1);

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(2, true));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(in_nodes, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n1.uid, n2.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n2.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
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

            tpie::file_stream<node_t> in_nodes;
            in_nodes.open();

            node_t n5 = create_node(2,1, sink_F, sink_T);
            in_nodes.write(n5);

            node_t n4 = create_node(2,0, sink_T, sink_F);
            in_nodes.write(n4);

            node_t n3 = create_node(1,1, n5.uid, sink_F);
            in_nodes.write(n3);

            node_t n2 = create_node(1,0, n4.uid, sink_F);
            in_nodes.write(n2);

            node_t n1 = create_node(0,0, n2.uid, n3.uid);
            in_nodes.write(n1);

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(2, true));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(in_nodes, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n1.uid, n2.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n3.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n2.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n2.uid), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n3.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n3.uid), sink_F} ));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
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

            tpie::file_stream<node_t> dead_obdd;
            dead_obdd.open();

            node_t n9 = create_node(3,1, sink_T, sink_F);
            dead_obdd.write(n9);

            node_t n8 = create_node(3,0, sink_F, sink_T);
            dead_obdd.write(n8);

            node_t n7 = create_node(2,3, n9.uid, sink_T);
            dead_obdd.write(n7);

            node_t n6 = create_node(2,2, sink_T, n9.uid);
            dead_obdd.write(n6);

            node_t n5 = create_node(2,1, sink_F, n8.uid);
            dead_obdd.write(n5);

            node_t n4 = create_node(2,0, sink_T, sink_F);
            dead_obdd.write(n4);

            node_t n3 = create_node(1,1, n6.uid, n7.uid);
            dead_obdd.write(n3);

            node_t n2 = create_node(1,0, n4.uid, n5.uid);
            dead_obdd.write(n2);

            node_t n1 = create_node(0,0, n2.uid, n3.uid);
            dead_obdd.write(n1);

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write(create_assignment(1, true));

            tpie::file_stream<node_t> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc_t> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc_t> reduce_sink_arcs;
            reduce_sink_arcs.open();

            restrict(dead_obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0u));

            reduce_node_arcs.seek(0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n1.uid, n5.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n1.uid), n7.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { flag(n5.uid), n8.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(arc { n7.uid, n9.uid }));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            reduce_sink_arcs.seek(0);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n5.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n7.uid), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n8.uid, sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n8.uid), sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { n9.uid, sink_T }));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(arc { flag(n9.uid), sink_F }));

            AssertThat(reduce_sink_arcs.can_read(), Is().False());
          });
      });
  });
