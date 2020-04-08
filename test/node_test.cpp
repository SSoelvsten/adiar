#include <../src/node.cpp>

go_bandit([]() {
    describe("Node and arcs", []() {

        it("should store and retrieve value in Sink arcs", [&]() {
            auto arc = create_sink_arc(true);
            AssertThat(value_of(arc), Is().True());

            arc = create_sink_arc(false);
            AssertThat(value_of(arc), Is().False());
          });

        it("should store and retrieve label in Node arcs", [&]() {
            auto arc = create_node_arc(42,2);
            AssertThat(label_of(arc), Is().EqualTo(42));

            arc = create_node_arc(21,2);
            AssertThat(label_of(arc), Is().EqualTo(21));
          });

        it("should store and retrieve index in Node arcs", [&]() {
            auto arc = create_node_arc(2,42);
            AssertThat(index_of(arc), Is().EqualTo(42));

            arc = create_node_arc(2,21);
            AssertThat(index_of(arc), Is().EqualTo(21));
          });

        it("should sort Sink arcs after Node arcs", [&]() {
            // Create a node arc with the highest possible raw value
            auto arc_node = create_node_arc(NODE_MAX_LABEL,NODE_MAX_INDEX);

            // Create a sink arc with the lowest raw value
            auto arc_sink = create_sink_arc(false);

            AssertThat(arc_node < arc_sink, Is().True());
          });

        it("should recognise Sink arcs as sinks", [&]() {
            auto sink_f = create_sink_arc (false);
            auto sink_t = create_sink_arc (true);

            AssertThat(is_sink_arc(sink_f), Is().True());
            AssertThat(is_sink_arc(sink_t), Is().True());
          });

        it("should not recognise Node arcs as sinks", [&]() {
            auto arc_node_max = create_node_arc(NODE_MAX_LABEL,NODE_MAX_INDEX);
            AssertThat(is_sink_arc(arc_node_max), Is().False());

            auto arc_node_min = create_node_arc(0,0);
            AssertThat(is_sink_arc(arc_node_min), Is().False());

            auto arc_node = create_node_arc(42,18);
            AssertThat(is_sink_arc(arc_node), Is().False());
          });

        it("should recognise Node arcs as nodes", [&]() {
            auto arc_node_max = create_node_arc(NODE_MAX_LABEL,NODE_MAX_INDEX);
            AssertThat(is_node_arc(arc_node_max), Is().True());

            auto arc_node_min = create_node_arc(0,0);
            AssertThat(is_node_arc(arc_node_min), Is().True());

            auto arc_node = create_node_arc(42,18);
            AssertThat(is_node_arc(arc_node), Is().True());
          });

        it("should not recognise Sink arcs as nodes", [&]() {
            auto sink_f = create_sink_arc (false);
            auto sink_t = create_sink_arc (true);

            AssertThat(is_node_arc(sink_f), Is().False());
            AssertThat(is_node_arc(sink_t), Is().False());
          });

        it("should sort Node arcs by label, then by index", [&]() {
            auto node_1_2 = create_node_arc(1,2);
            auto node_2_1 = create_node_arc(2,1);
            auto node_2_2 = create_node_arc(2,2);

            AssertThat(node_1_2 < node_2_1, Is().True());
            AssertThat(node_2_1 < node_2_2, Is().True());
          });

        it("should sort Node by label, then by index", [&]() {
            auto sink_f = create_sink_arc (false);
            auto sink_t = create_sink_arc (true);

            auto node_1_2 = create_node(1,2,sink_f,sink_t);
            auto node_2_1 = create_node(2,1,sink_t,sink_f);
            auto node_2_2 = create_node(2,2,sink_f,sink_f);

            AssertThat(node_1_2 < node_2_1, Is().True());
            AssertThat(node_2_1 < node_2_2, Is().True());
          });

        it("should treat NodeArcs and Node as PODs", [&]() {
            AssertThat(std::is_pod<node>::value, Is().True());
          });

        it("should use minimal space on NodeArcs and Node", [&]() {
            auto test_arc_node = create_node_arc(42,2);
            auto test_arc_sink = create_sink_arc (false);
            auto test_node = create_node(1,
                                         8,
                                         test_arc_node,
                                         test_arc_sink);

            AssertThat(sizeof(test_arc_sink), Is().EqualTo(8));
            AssertThat(sizeof(test_arc_node), Is().EqualTo(8));
            AssertThat(sizeof(test_node), Is().EqualTo(3 * 8));
          });
      });
  });
