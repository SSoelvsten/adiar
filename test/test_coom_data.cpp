#include <coom/data.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Node and arcs", []() {
        it("should store and retrieve value in Sink arcs", [&]() {
            auto arc = create_sink(true);
            AssertThat(value_of(arc), Is().True());

            arc = create_sink(false);
            AssertThat(value_of(arc), Is().False());
          });

        it("should store and retrieve label in Node Ptr", [&]() {
            auto arc = create_node_ptr(42,2);
            AssertThat(label_of(arc), Is().EqualTo(42));

            arc = create_node_ptr(21,2);
            AssertThat(label_of(arc), Is().EqualTo(21));
          });

        it("should store and retrieve id in Node Ptr", [&]() {
            auto arc = create_node_ptr(2,42);
            AssertThat(id_of(arc), Is().EqualTo(42));

            arc = create_node_ptr(2,21);
            AssertThat(id_of(arc), Is().EqualTo(21));
          });

        it("should sort Sink arcs after Node Ptr", [&]() {
            // Create a node arc with the highest possible raw value
            auto arc_node = create_node_ptr(MAX_LABEL,MAX_ID);

            // Create a sink arc with the lowest raw value
            auto arc_sink = create_sink(false);

            AssertThat(arc_node < arc_sink, Is().True());
          });

        it("should recognise Sinks as such", [&]() {
            auto sink_f = create_sink (false);
            auto sink_t = create_sink (true);

            AssertThat(is_sink(sink_f), Is().True());
            AssertThat(is_sink(sink_t), Is().True());
          });

        it("should not recognise Node Ptr as Sinks", [&]() {
            auto arc_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
            AssertThat(is_sink(arc_node_max), Is().False());

            auto arc_node_min = create_node_ptr(0,0);
            AssertThat(is_sink(arc_node_min), Is().False());

            auto arc_node = create_node_ptr(42,18);
            AssertThat(is_sink(arc_node), Is().False());
          });

        it("should not recognise Nil as Sinks", [&]() {
            AssertThat(is_sink(NIL), Is().False());
          });

        it("should recognise Node Ptr as such", [&]() {
            auto arc_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
            AssertThat(is_node_ptr(arc_node_max), Is().True());

            auto arc_node_min = create_node_ptr(0,0);
            AssertThat(is_node_ptr(arc_node_min), Is().True());

            auto arc_node = create_node_ptr(42,18);
            AssertThat(is_node_ptr(arc_node), Is().True());
          });

        it("should distinguish Nil from Node Ptr", [&]() {
            AssertThat(is_node_ptr(NIL), Is().False());
          });

        it("should not recognise Sinks as Node Ptr", [&]() {
            auto sink_f = create_sink (false);
            auto sink_t = create_sink (true);

            AssertThat(is_node_ptr(sink_f), Is().False());
            AssertThat(is_node_ptr(sink_t), Is().False());
          });

        it("should sort Node Ptr by label, then by id", [&]() {
            auto node_1_2 = create_node_ptr(1,2);
            auto node_2_1 = create_node_ptr(2,1);
            auto node_2_2 = create_node_ptr(2,2);

            AssertThat(node_1_2 < node_2_1, Is().True());
            AssertThat(node_2_1 < node_2_2, Is().True());
          });

        it("should recognise equal Nodes by their content", [&]() {
            auto sink_f = create_sink(false);
            auto sink_t = create_sink(true);

            auto node_1_v1 = create_node(42,2,sink_f,sink_t);
            auto node_1_v2 = create_node(42,2,sink_f,sink_t);

            AssertThat(node_1_v1 == node_1_v2, Is().True());
            AssertThat(node_1_v1 != node_1_v2, Is().False());
          });

        it("should recognise unequal Nodes by their content", [&]() {
            auto sink_f = create_sink(false);
            auto sink_t = create_sink(true);

            auto node_1 = create_node(42,2,sink_f,sink_t);
            auto node_2 = create_node(42,2,sink_f,sink_f);
            auto node_3 = create_node(42,3,sink_f,sink_t);
            auto node_4 = create_node(21,2,sink_f,sink_t);

            AssertThat(node_1 == node_2, Is().False());
            AssertThat(node_1 != node_2, Is().True());

            AssertThat(node_1 == node_3, Is().False());
            AssertThat(node_1 != node_3, Is().True());

            AssertThat(node_1 == node_4, Is().False());
            AssertThat(node_1 != node_4, Is().True());
          });

        it("should sort Node by label, then by id", [&]() {
            auto sink_f = create_sink (false);
            auto sink_t = create_sink (true);

            auto node_1_2 = create_node(1,2,sink_f,sink_t);
            auto node_2_1 = create_node(2,1,sink_t,sink_f);

            AssertThat(node_1_2 < node_2_1, Is().True());
            AssertThat(node_2_1 > node_1_2, Is().True());

            auto node_2_2 = create_node(2,2,sink_f,sink_f);

            AssertThat(node_2_1 < node_2_2, Is().True());
            AssertThat(node_2_2 > node_2_1, Is().True());
          });

        it("should recognise equal Arcs by their content", [&]() {
            auto source = create_node_ptr(4,2);
            auto target = create_node_ptr(42,3);

            auto arc_1 = create_arc(source, true, target);
            auto arc_2 = create_arc(source, true, target);

            AssertThat(arc_1 == arc_2, Is().True());
            AssertThat(arc_1 != arc_2, Is().False());
          });

        it("should recognise unequal Arcs by their content", [&]() {
            auto node_ptr_1 = create_node_ptr(4,2);
            auto node_ptr_2 = create_node_ptr(4,3);
            auto node_ptr_3 = create_node_ptr(3,2);

            auto arc_1 = create_arc(node_ptr_1, true, node_ptr_2);
            auto arc_2 = create_arc(node_ptr_1, true, node_ptr_3);

            AssertThat(arc_1 == arc_2, Is().False());
            AssertThat(arc_1 != arc_2, Is().True());

            auto arc_3 = create_arc(node_ptr_1, true, node_ptr_2);
            auto arc_4 = create_arc(node_ptr_1, false, node_ptr_2);

            AssertThat(arc_3 == arc_4, Is().False());
            AssertThat(arc_3 != arc_4, Is().True());

            auto arc_5 = create_arc(node_ptr_1, true, node_ptr_2);
            auto arc_6 = create_arc(node_ptr_3, true, node_ptr_2);

            AssertThat(arc_5 == arc_6, Is().False());
            AssertThat(arc_5 != arc_6, Is().True());
          });

        it("should sort Arc by source, then by is_high, then by target", [&]() {
            auto arc_1 = create_arc(create_node_ptr(0,42),
                                    false,
                                    create_node_ptr(2,2));

            auto arc_2 = create_arc(create_node_ptr(0,42),
                                    true,
                                    create_node_ptr(0,0));

            AssertThat(arc_1 < arc_2, Is().True());
            AssertThat(arc_2 > arc_1, Is().True());

            auto arc_3 = create_arc(create_node_ptr(2,2),
                                    true,
                                    create_node_ptr(2,2));

            auto arc_4 = create_arc(create_node_ptr(2,2),
                                    true,
                                    create_node_ptr(2,3));

            AssertThat(arc_3 < arc_4, Is().True());
            AssertThat(arc_4 > arc_3, Is().True());
          });

        it("should treat Nodes as PODs", [&]() {
            AssertThat(std::is_pod<node>::value, Is().True());
          });

        it("should treat Arcs as PODs", [&]() {
            AssertThat(std::is_pod<arc>::value, Is().True());
          });

        it("should use 8 bytes of memory for a Node Ptr", [&]() {
            auto node_ptr = create_node_ptr(42,2);
            AssertThat(sizeof(node_ptr), Is().EqualTo(8));
          });

        it("should use 8 bytes of memory for a Sink", [&]() {
            auto sink = create_sink (false);
            AssertThat(sizeof(sink), Is().EqualTo(8));
          });

        it("should use 24 bytes of memory for a Node", [&]() {
            auto node_ptr = create_node_ptr(42,2);
            auto sink = create_sink (false);
            auto node = create_node(1,
                                    8,
                                    node_ptr,
                                    sink);

            AssertThat(sizeof(node), Is().EqualTo(3 * 8));
          });

        it("should use 24 bytes of memory for an Arc", [&]() {
            auto node_ptr = create_node_ptr(42,2);
            auto sink = create_sink (false);
            auto arc = create_arc(node_ptr,
                                  false,
                                  sink);

            // For C++ a struct has to be word-aligned, which means the 1 bit
            // boolean field will have to take up a whole word (8b) instead.
            AssertThat(sizeof(arc), Is().EqualTo(3*8));
          });
      });
  });
