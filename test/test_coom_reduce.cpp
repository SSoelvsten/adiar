#include <tpie/tpie.h>

#include <coom/data.cpp>
#include <coom/reduce.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Reduce", [&]() {
        it("can apply reduction rule 1 on sink arcs", [&]() {
            /*
                 1                  1
                / \                / \
                | 2                | 2
                |/ \      =>       |/ \
                3  4               3  |
               / \//               |\ /
               F  T                F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n3));
            in_node_arcs.write(create_arc(n2,true,n4));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n3,false,sink_F));
            in_sink_arcs.write(create_arc(n3,true,sink_T));
            in_sink_arcs.write(create_arc(n4,false,sink_T));
            in_sink_arcs.write(create_arc(n4,true,sink_T));


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 1 on node arcs", [&]() {
            /*
                 1                  1
                / \                / \
                | 2                | 2
                |/ \      =>       |/ \
                3  4               3   \
               / \ \\             / \  |
               F T  5             F T  5
                   / \                / \
                   F T                F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);
            auto n5 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n3));
            in_node_arcs.write(create_arc(n2,true,n4));
            in_node_arcs.write(create_arc(n4,false,n5));
            in_node_arcs.write(create_arc(n4,true,n5));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n3,false,sink_F));
            in_sink_arcs.write(create_arc(n3,true,sink_T));

            in_sink_arcs.write(create_arc(n5,false,sink_F));
            in_sink_arcs.write(create_arc(n5,true,sink_T));


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
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on sink arcs", [&]() {
            /*
               1                  1
              / \                / \
              | 2_               | 2
              | | \      =>      |/ \
              3 4 T              4  T
              |X|               / \
              F T               F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n4));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n2,true,sink_T));
            in_sink_arcs.write(create_arc(n3,false,sink_F));
            in_sink_arcs.write(create_arc(n3,true,sink_T));
            in_sink_arcs.write(create_arc(n4,false,sink_F));
            in_sink_arcs.write(create_arc(n4,true,sink_T));


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

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on node arcs", [&]() {
            /*
                 1                  1
                / \                / \
                | 2_               | 2
                | | \      =>      |/ \
                3 4 T              4  T
                |X|               / \
                5 T               5 T
               / \               / \
               F T               F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);
            auto n5 = create_node_ptr(3,0);
            auto n6 = create_node_ptr(3,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n4));
            in_node_arcs.write(create_arc(n3,false,n5));
            in_node_arcs.write(create_arc(n4,false,n5));
            in_node_arcs.write(create_arc(n3,true,n6));
            in_node_arcs.write(create_arc(n4,true,n6));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n2,true,sink_T));
            in_sink_arcs.write(create_arc(n5,false,sink_F));
            in_sink_arcs.write(create_arc(n5,true,sink_T));
            in_sink_arcs.write(create_arc(n6,false,sink_T));
            in_sink_arcs.write(create_arc(n6,true,sink_F));


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID, sink_T, sink_F)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n5
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, MAX_ID-1, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n4
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply reduction rule 2 on node and sink arcs", [&]() {
            /*
                 1                  1
                / \                / \
                | 2_               | 2
                | | \      =>      |/ \
                3 4 T              4  T
                |X|               / \
                5 T               5 T
               / \               / \
               F T               F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);
            auto n5 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n4));
            in_node_arcs.write(create_arc(n3,false,n5));
            in_node_arcs.write(create_arc(n4,false,n5));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n2,true,sink_T));
            in_sink_arcs.write(create_arc(n3,true,sink_T));
            in_sink_arcs.write(create_arc(n4,true,sink_T));
            in_sink_arcs.write(create_arc(n5,false,sink_F));
            in_sink_arcs.write(create_arc(n5,true,sink_T));


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
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, create_node_ptr(3,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can apply both reduction rule 1 and 2", [&]() {
            /*
                 1                  1
                / \                / \
                2 T                | T
               / \        =>       |
               3 4                 4
               |X|                / \
               F T                F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,false,n2));
            in_node_arcs.write(create_arc(n2,false,n3));
            in_node_arcs.write(create_arc(n2,true,n4));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n1,true,sink_T));
            in_sink_arcs.write(create_arc(n2,true,sink_T));
            in_sink_arcs.write(create_arc(n3,false,sink_F));
            in_sink_arcs.write(create_arc(n3,true,sink_T));
            in_sink_arcs.write(create_arc(n4,false,sink_F));
            in_sink_arcs.write(create_arc(n4,true,sink_T));


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

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("can reduce nodes in 'disjoint' branches", [&]() {
            /*
                     1                         1
                    / \                       / \
                   2   3                     2   3
                  / \ / \                    |\ /|
                 4   5   6        =>          \T/
                / \ / \ / \                    6
                F T 7 T F T                   / \
                   / \                        F T
                   T T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(1,1);
            auto n4 = create_node_ptr(2,0);
            auto n5 = create_node_ptr(2,1);
            auto n6 = create_node_ptr(2,2);
            auto n7 = create_node_ptr(3,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,false,n2));
            in_node_arcs.write(create_arc(n1,true,n3));
            in_node_arcs.write(create_arc(n2,false,n4));
            in_node_arcs.write(create_arc(n2,true,n5));
            in_node_arcs.write(create_arc(n3,false,n5));
            in_node_arcs.write(create_arc(n3,true,n6));
            in_node_arcs.write(create_arc(n5,false,n7));


            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n4,false,sink_F));
            in_sink_arcs.write(create_arc(n4,true,sink_T));
            in_sink_arcs.write(create_arc(n5,true,sink_F));
            in_sink_arcs.write(create_arc(n6,false,sink_F));
            in_sink_arcs.write(create_arc(n6,true,sink_T));
            in_sink_arcs.write(create_arc(n7,false,sink_T));
            in_sink_arcs.write(create_arc(n7,true,sink_T));


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // n6
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n3
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, sink_T, create_node_ptr(2, MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().True());

            // n2
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID-1, create_node_ptr(2, MAX_ID), sink_T)));
            AssertThat(out_nodes.can_read(), Is().True());

            // n1
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(1,MAX_ID-1), create_node_ptr(1, MAX_ID))));
            AssertThat(out_nodes.can_read(), Is().False());
          });


        it("can reduce the root", [&]() {
            /*
                 1
                / \
                | 2
                |/ \        =>
                3  4                4
                |X/                / \
                F T                F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,true,n3));
            in_node_arcs.write(create_arc(n2,true,n4));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n3,false,sink_F));
            in_sink_arcs.write(create_arc(n3,true,sink_T));
            in_sink_arcs.write(create_arc(n4,false,sink_F));
            in_sink_arcs.write(create_arc(n4,true,sink_T));


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
                 1                 F
                / \      =>
                F F
            */

            auto n1 = create_node_ptr(0,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n1,false,sink_F));
            in_sink_arcs.write(create_arc(n1,true,sink_F));


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // F
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));
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

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);

            in_sink_arcs.write(create_arc(n1,false,sink_T));
            in_sink_arcs.write(create_arc(n2,false,sink_T));
            in_sink_arcs.write(create_arc(n2,true,sink_T));


            // Reduce it
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            coom::reduce(in_node_arcs, in_sink_arcs, out_nodes);


            // Check it looks all right
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());

            // T
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));
            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("is stable", [&]() {
            /*
                 1                    1
                / \                  / \
                | 2                  | 2
                |/ \       =>        |/ \
                3   4                3   4
               / \ / \              / \ / \
               T F F T              T F F T
            */

            auto n1 = create_node_ptr(0,0);
            auto n2 = create_node_ptr(1,0);
            auto n3 = create_node_ptr(2,0);
            auto n4 = create_node_ptr(2,1);

            tpie::file_stream<arc> in_node_arcs;
            in_node_arcs.open();

            in_node_arcs.write(create_arc(n1,true,n2));
            in_node_arcs.write(create_arc(n1,false,n3));
            in_node_arcs.write(create_arc(n2,false,n3));
            in_node_arcs.write(create_arc(n2,true,n4));

            tpie::file_stream<arc> in_sink_arcs;
            in_sink_arcs.open();

            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            in_sink_arcs.write(create_arc(n3,false,sink_T));
            in_sink_arcs.write(create_arc(n3,true,sink_F));
            in_sink_arcs.write(create_arc(n4,false,sink_F));
            in_sink_arcs.write(create_arc(n4,true,sink_T));


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
          });
      });
  });
