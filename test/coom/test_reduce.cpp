#include <tpie/tpie.h>

#include <coom/data.cpp>
#include <coom/reduce.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Reduce", [&]() {
        it("can apply reduction rule 1 on sink arcs", [&]() {
            /*
                 1                  1     ---- x0
                / \                / \
                | 2                | 2    ---- x1
                |/ \      =>       |/ \
                3  4               3  |   ---- x2
               / \//               |\ /
               F  T                F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n3 });
            in_node_arcs.write({ flag(n2),n4 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ n3,sink_F });
            in_sink_arcs.write({ flag(n3),sink_T });
            in_sink_arcs.write({ n4,sink_T });
            in_sink_arcs.write({ flag(n4),sink_T });

            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);

            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 1 on node arcs", [&]() {
            /*
                 1                  1        ---- x0
                / \                / \
                | 2                | 2       ---- x1
                |/ \      =>       |/ \
                3  4               3   \     ---- x2
               / \ \\             / \  |
               F T  5             F T  5     ---- x3
                   / \                / \
                   F T                F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);
            ptr_t n5 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n3 });
            in_node_arcs.write({ flag(n2),n4 });
            in_node_arcs.write({ n4,n5 });
            in_node_arcs.write({ flag(n4),n5 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ n3,sink_F });
            in_sink_arcs.write({ flag(n3),sink_T });

            in_sink_arcs.write({ n5,sink_F });
            in_sink_arcs.write({ flag(n5),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);

            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(3,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on sink arcs", [&]() {
            /*
               1                  1      ---- x0
              / \                / \
              | 2_               | 2     ---- x1
              | | \      =>      |/ \
              3 4 T              4  T    ---- x2
              |X|               / \
              F T               F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n4 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ flag(n2),sink_T });
            in_sink_arcs.write({ n3,sink_F });
            in_sink_arcs.write({ flag(n3),sink_T });
            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);

            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on node arcs", [&]() {
            /*
                 1                  1       ---- x0
                / \                / \
                | 2_               | 2      ---- x1
                | | \      =>      |/ \
                3 4 T              4  T     ---- x2
                |X|               / \
                5 6               5  6      ---- x3
               / \ \\            / \ \\
               F T T F           F T T F
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);
            ptr_t n5 = create_node_ptr(3,0);
            ptr_t n6 = create_node_ptr(3,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n4 });
            in_node_arcs.write({ n3,n5 });
            in_node_arcs.write({ n4,n5 });
            in_node_arcs.write({ flag(n3),n6 });
            in_node_arcs.write({ flag(n4),n6 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ flag(n2),sink_T });
            in_sink_arcs.write({ n5,sink_F });
            in_sink_arcs.write({ flag(n5),sink_T });
            in_sink_arcs.write({ n6,sink_T });
            in_sink_arcs.write({ flag(n6),sink_F });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID-1, sink_T, sink_F)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  create_node_ptr(3, MAX_ID),
                                                                  create_node_ptr(3, MAX_ID-1))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on node and sink arcs", [&]() {
            /*
                 1                  1     ---- x0
                / \                / \
                | 2_               | 2    ---- x1
                | | \      =>      |/ \
                3 4 T              4  T   ---- x2
                |X|               / \
                5 T               5 T     ---- x3
               / \               / \
               F T               F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);
            ptr_t n5 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n4 });
            in_node_arcs.write({ n3,n5 });
            in_node_arcs.write({ n4,n5 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink_ptr(true);
            auto sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ flag(n2),sink_T });
            in_sink_arcs.write({ flag(n3),sink_T });
            in_sink_arcs.write({ flag(n4),sink_T });
            in_sink_arcs.write({ n5,sink_F });
            in_sink_arcs.write({ flag(n5),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  create_node_ptr(3,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply both reduction rule 1 and 2", [&]() {
            /*
                 1                  1     ---- x0
                / \                / \
                2 T                | T    ---- x1
               / \        =>       |
               3 4                 4      ---- x2
               |X|                / \
               F T                F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ n1,n2 });
            in_node_arcs.write({ n2,n3 });
            in_node_arcs.write({ flag(n2),n4 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ flag(n1),sink_T });
            in_sink_arcs.write({ n3,sink_F });
            in_sink_arcs.write({ flag(n3),sink_T });
            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2,MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can reduce nodes in 'disjoint' branches", [&]() {
            /*
                     1                         1      ---- x0
                    / \                       / \
                   2   3                     2_ _3    ---- x1
                  / \ / \                    \_T_/
                 4   5   6        =>           6     ---- x2
                / \ / \ / \                   / \
                F T 7 T F T                   F T     ---- x3
                   / \
                   T T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(1,1);
            ptr_t n4 = create_node_ptr(2,0);
            ptr_t n5 = create_node_ptr(2,1);
            ptr_t n6 = create_node_ptr(2,2);
            ptr_t n7 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ n1,n2 });
            in_node_arcs.write({ flag(n1),n3 });
            in_node_arcs.write({ n2,n4 });
            in_node_arcs.write({ flag(n2),n5 });
            in_node_arcs.write({ n3,n5 });
            in_node_arcs.write({ flag(n3),n6 });
            in_node_arcs.write({ n5,n7 });


            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });
            in_sink_arcs.write({ flag(n5),sink_T });
            in_sink_arcs.write({ n6,sink_F });
            in_sink_arcs.write({ flag(n6),sink_T });
            in_sink_arcs.write({ n7,sink_T });
            in_sink_arcs.write({ flag(n7),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);

            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                                  sink_T,
                                                                  create_node_ptr(2, MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(1,MAX_ID),
                                                                  create_node_ptr(1, MAX_ID-1))));
            AssertThat(out_nodes.can_read(), Is().False());
          });


        it("can reduce the root", [&]() {
            /*
                 1                         ---- x0
                / \
                | 2                        ---- x1
                |/ \        =>
                3  4                4      ---- x2
                |X/                / \
                F T                F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(2,0);
            ptr_t n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,n3 });
            in_node_arcs.write({ n2,n3 });
            in_node_arcs.write({ flag(n2),n4 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ n3,sink_F });
            in_sink_arcs.write({ flag(n3),sink_T });
            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can reduce down to a sink (1)", [&]() {
            /*
                 1                 F       ---- x0
                / \      =>
                F F
            */

            ptr_t n1 = create_node_ptr(0,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_F = create_sink_ptr(false);

            in_sink_arcs.write({ n1,sink_F });
            in_sink_arcs.write({ flag(n1),sink_F });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // F
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can reduce down to a sink (2)", [&]() {
            /*
               1                  T
              / \
              | 2         =>
              |/ \
              T  T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_T = create_sink_ptr(true);

            in_sink_arcs.write({ n1,sink_T });
            in_sink_arcs.write({ n2,sink_T });
            in_sink_arcs.write({ flag(n2),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // T
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("does forward the correct children [1]", [&]() {
            /*
                   1                1        ---- x0
                  / \              / \
                 2   3             | 3       ---- x1
                / \ / \      =>    |/ \
               4   5   6           5   6     ---- x2
              / \ / \ / \         / \ / \
              F T F T T F         F T T F
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(1,1);
            ptr_t n4 = create_node_ptr(2,0);
            ptr_t n5 = create_node_ptr(2,1);
            ptr_t n6 = create_node_ptr(2,2);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ n1,n2 });
            in_node_arcs.write({ flag(n1),n3 });
            in_node_arcs.write({ n2,n4 });
            in_node_arcs.write({ flag(n2),n5 });
            in_node_arcs.write({ n3,n5 });
            in_node_arcs.write({ flag(n3),n6 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_F = create_sink_ptr(false);
            ptr_t sink_T = create_sink_ptr(true);

            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });
            in_sink_arcs.write({ n5,sink_F });
            in_sink_arcs.write({ flag(n5),sink_T });
            in_sink_arcs.write({ n6,sink_T });
            in_sink_arcs.write({ flag(n6),sink_F });

            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True()); // 5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));

            AssertThat(out_nodes.can_read(), Is().True()); // 6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                                  sink_T,
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True()); // 3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  create_node_ptr(2, MAX_ID-1))));

            AssertThat(out_nodes.can_read(), Is().True()); // 1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  create_node_ptr(1, MAX_ID))));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("does forward the correct children [2]", [&]() {
            /*
                   1                1        ---- x0
                  / \              / \
                 2   3             | 3       ---- x1
                / \ / \      =>    |/ \
               4   5   6           5   6     ---- x2
              / \ / \ / \         / \ / \
              T F T F F T         T F F T
            */

            ptr_t n1 = create_node_ptr(0,0);
            ptr_t n2 = create_node_ptr(1,0);
            ptr_t n3 = create_node_ptr(1,1);
            ptr_t n4 = create_node_ptr(2,0);
            ptr_t n5 = create_node_ptr(2,1);
            ptr_t n6 = create_node_ptr(2,2);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ n1,n2 });
            in_node_arcs.write({ flag(n1),n3 });
            in_node_arcs.write({ n2,n4 });
            in_node_arcs.write({ flag(n2),n5 });
            in_node_arcs.write({ n3,n5 });
            in_node_arcs.write({ flag(n3),n6 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            ptr_t sink_F = create_sink_ptr(false);
            ptr_t sink_T = create_sink_ptr(true);

            in_sink_arcs.write({ n4,sink_T });
            in_sink_arcs.write({ flag(n4),sink_F });
            in_sink_arcs.write({ n5,sink_T });
            in_sink_arcs.write({ flag(n5),sink_F });
            in_sink_arcs.write({ n6,sink_F });
            in_sink_arcs.write({ flag(n6),sink_T });

            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True()); // 6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_F,
                                                                  sink_T)));

            AssertThat(out_nodes.can_read(), Is().True()); // 5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                                  sink_T,
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True()); // 3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID-1),
                                                                  create_node_ptr(2, MAX_ID))));

            AssertThat(out_nodes.can_read(), Is().True()); // 1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID-1),
                                                                  create_node_ptr(1, MAX_ID))));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        /*it("is stable", [&]() {
            /*
                 1                    1
                / \                  / \
                | 2                  | 2
                |/ \       =>        |/ \
                3   4                3   4
               / \ / \              / \ / \
               T F F T              T F F T
            */

            /*auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write({ flag(n1),n2 });
            in_node_arcs.write({ n1,false,n3 });
            in_node_arcs.write({ n2,false,n3 });
            in_node_arcs.write({ flag(n2),n4 });

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write({ n3,sink_T });
            in_sink_arcs.write({ flag(n3),sink_F });
            in_sink_arcs.write({ n4,sink_F });
            in_sink_arcs.write({ flag(n4),sink_T });


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID-1, sink_T, sink_F)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID-1), create_node_ptr(2,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID-1), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });*/
      });
  });
