#include <adiar/internal/data_types/convert.h>

go_bandit([]() {
    describe("adiar/internal/data_types/convert.h", []() {
        describe("low_arc_of", []() {
            it("should extract low arc from node", [&]() {
                const node_t node = create_node(7,42,
                                                create_node_ptr(8,21),
                                                create_node_ptr(9,8));

                const arc_t arc = low_arc_of(node);

                AssertThat(label_of(arc.source), Is().EqualTo(7u));
                AssertThat(id_of(arc.source), Is().EqualTo(42u));
                AssertThat(label_of(arc.target), Is().EqualTo(8u));
                AssertThat(id_of(arc.target), Is().EqualTo(21u));
              });
          });

        describe("high_arc_of", []() {
            it("should extract high arc from node", [&]() {
                const node_t node = create_node(6,13,
                                                create_node_ptr(8,21),
                                                create_node_ptr(9,8));

                const arc_t arc = high_arc_of(node);

                AssertThat(label_of(arc.source), Is().EqualTo(6u));
                AssertThat(id_of(arc.source), Is().EqualTo(13u));
                AssertThat(label_of(arc.target), Is().EqualTo(9u));
                AssertThat(id_of(arc.target), Is().EqualTo(8u));
              });
          });

        describe("node_of", []() {
            it("should combine low and high arcs into single node", [&]() {
                const arc_t low_arc = { create_node_ptr(17,42), create_node_ptr(9,8) };
                const arc_t high_arc = { flag(create_node_ptr(17,42)), create_node_ptr(8,21) };

                const node_t node = node_of(low_arc, high_arc);

                AssertThat(label_of(node), Is().EqualTo(17u));
                AssertThat(id_of(node), Is().EqualTo(42u));

                AssertThat(label_of(node.low), Is().EqualTo(label_of(low_arc.target)));
                AssertThat(id_of(node.low), Is().EqualTo(id_of(low_arc.target)));

                AssertThat(label_of(node.high), Is().EqualTo(label_of(high_arc.target)));
                AssertThat(id_of(node.high), Is().EqualTo(id_of(high_arc.target)));
              });
          });
      });
  });
