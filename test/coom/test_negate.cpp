#include <tpie/tpie.h>

#include <coom/negate.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Negate", [&]() {
        it("should negate a T sink-only OBDD into an F sink-only OBDD", [&]() {
            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink(true));

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            // Negate it
            negate(in_nodes, out_nodes);

            // Check if it is correct
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should negate a F sink-only OBDD into an T sink-only OBDD", [&]() {
            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            in_nodes.write(create_sink(false));

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            // Negate it
            negate(in_nodes, out_nodes);

            // Check if it is correct
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should negate sink-children in OBDD [1]", [&]() {
            /*
                   1
                  / \
                  | 2
                  |/ \
                  3  |
                  |\ /
                  F T
            */

            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            // n3
            in_nodes.write(create_node(2, MAX_ID,
                                       sink_F,
                                       sink_T));

            // n2
            in_nodes.write(create_node(1, MAX_ID,
                                       create_node_ptr(2, MAX_ID),
                                       sink_T));

            // n1
            in_nodes.write(create_node(0, MAX_ID,
                                       create_node_ptr(2, MAX_ID),
                                       create_node_ptr(1, MAX_ID)));

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            // Negate it
            negate(in_nodes, out_nodes);

            // Check if it is correct
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_T,
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  create_node_ptr(1, MAX_ID))));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should negate sink-children in OBDD [2]", [&]() {
            /*
                    1
                   / \
                  2   3
                  |\ /|
                   \T/
                    6
                   / \
                   F T
            */

            tpie::file_stream<node> in_nodes;
            in_nodes.open();

            ptr_t sink_T = create_sink_ptr(true);
            ptr_t sink_F = create_sink_ptr(false);

            // n1
            in_nodes.write(create_node(2, MAX_ID,
                                       sink_F,
                                       sink_T));

            // n3
            in_nodes.write(create_node(1, MAX_ID,
                                       create_node_ptr(2, MAX_ID),
                                       sink_T));

            // n2
            in_nodes.write(create_node(1, MAX_ID-1,
                                       sink_T,
                                       create_node_ptr(2, MAX_ID)));

            // n1
            in_nodes.write(create_node(0, MAX_ID,
                                       create_node_ptr(1, MAX_ID-1),
                                       create_node_ptr(1, MAX_ID)));

            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            // Negate it
            negate(in_nodes, out_nodes);

            // Check if it is correct
            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, MAX_ID,
                                                                  sink_T,
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID,
                                                                  create_node_ptr(2, MAX_ID),
                                                                  sink_F)));

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                                  sink_F,
                                                                  create_node_ptr(2, MAX_ID))));


            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, MAX_ID,
                                                                  create_node_ptr(1, MAX_ID-1),
                                                                  create_node_ptr(1, MAX_ID))));

            AssertThat(out_nodes.can_read(), Is().False());
          });
      });
  });
