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

        it("should bridge layers [1]. Assignment: (_,_,T,_)", [&obdd]() {
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

            assignment.write({ 2, true });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            auto np1 = create_node_ptr(0,0);
            auto np2 = create_node_ptr(1,0);
            auto np5 = create_node_ptr(3,0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np1, true, np2)));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np2, true, np5)));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            auto sink_F = create_sink(false);
            auto sink_T = create_sink(true);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np1, false, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np2, false, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np5, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np5, true, sink_T)));
          });

        it("should bridge layers. [2]. Assignment: (_,F,_,_)", [&obdd]() {
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

            assignment.write({ 1, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            auto np1 = create_node_ptr(0,0);
            auto np3 = create_node_ptr(2,0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np1, false, np3)));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np1, true, np3)));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            auto sink_F = create_sink(false);
            auto sink_T = create_sink(true);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, true, sink_T)));
          });

        it("should bridge layers [3]. Assignment: (_,T,_,_)", [&obdd]() {
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

            assignment.write({ 1, true });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            auto np1 = create_node_ptr(0,0);
            auto np3 = create_node_ptr(2,0);
            auto np4 = create_node_ptr(2,1);
            auto np5 = create_node_ptr(3,0);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np1, false, np3)));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np1, true, np4)));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np4, true, np5)));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            auto sink_F = create_sink(false);
            auto sink_T = create_sink(true);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, true, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np4, false, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np5, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np5, true, sink_T)));
          });

        it("should remove root. Assignment: (T,_,_,F)", [&obdd]() {
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

            assignment.write({ 0, true });
            assignment.write({ 3, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            auto np2 = create_node_ptr(1,0);
            auto np3 = create_node_ptr(2,0);
            auto np4 = create_node_ptr(2,1);

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np2, false, np3)));

            AssertThat(reduce_node_arcs.can_read(), Is().True());
            AssertThat(reduce_node_arcs.read(), Is().EqualTo(create_arc(np2, true, np4)));

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            auto sink_F = create_sink(false);
            auto sink_T = create_sink(true);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, true, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np4, false, sink_T)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np4, true, sink_F)));
          });

        it("should ignore skipped variables. Assignment: (F,T,_,F)", [&obdd]() {
            /*
                 3
                / \
                F T
            */

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write({ 0, false });
            assignment.write({ 1, true });
            assignment.write({ 3, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(out_nodes.size(), Is().EqualTo(0));

            reduce_node_arcs.seek(0);
            reduce_sink_arcs.seek(0);

            auto np3 = create_node_ptr(2,0);

            AssertThat(reduce_node_arcs.can_read(), Is().False());

            auto sink_F = create_sink(false);
            auto sink_T = create_sink(true);

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, false, sink_F)));

            AssertThat(reduce_sink_arcs.can_read(), Is().True());
            AssertThat(reduce_sink_arcs.read(), Is().EqualTo(create_arc(np3, true, sink_T)));
          });

        it("should return F sink. Assignment: (F,_,F,_)", [&obdd]() {
            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write({ 0, false });
            assignment.write({ 2, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(reduce_node_arcs.size(), Is().EqualTo(0));
            AssertThat(reduce_sink_arcs.size(), Is().EqualTo(0));

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return T sink. Assignment: (T,T,F,_)", [&obdd]() {
            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write({ 0, true });
            assignment.write({ 1, true });
            assignment.write({ 2, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            tpie::file_stream<arc> reduce_node_arcs;
            reduce_node_arcs.open();

            tpie::file_stream<arc> reduce_sink_arcs;
            reduce_sink_arcs.open();

            coom::restrict(obdd, assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

            AssertThat(reduce_node_arcs.size(), Is().EqualTo(0));
            AssertThat(reduce_sink_arcs.size(), Is().EqualTo(0));

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should return T sink given a T sink", [&]() {
            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink_node(true));

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write({ 0, true });
            assignment.write({ 2, true });
            assignment.write({ 42, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::restrict(in_nodes, assignment, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });


        it("should return F sink given a F sink", [&]() {
            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink_node(false));

            tpie::file_stream<assignment> assignment;
            assignment.open();

            assignment.write({ 2, true });
            assignment.write({ 21, true });
            assignment.write({ 28, false });

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::restrict(in_nodes, assignment, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));
            AssertThat(out_nodes.can_read(), Is().False());
          });
      });
  });
