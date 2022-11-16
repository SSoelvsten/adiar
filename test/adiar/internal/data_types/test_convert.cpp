#include <adiar/internal/data_types/convert.h>

go_bandit([]() {
    describe("adiar/internal/data_types/convert.h", []() {
        describe("low_arc_of", []() {
            it("should extract low arc from node", [&]() {
                const node n = node(7,42,
                                      ptr_uint64(8,21),
                                      ptr_uint64(9,8));

                const arc_t arc = low_arc_of(n);

                AssertThat(arc.source().label(), Is().EqualTo(7u));
                AssertThat(arc.source().id(), Is().EqualTo(42u));
                AssertThat(arc.target().label(), Is().EqualTo(8u));
                AssertThat(arc.target().id(), Is().EqualTo(21u));
              });
          });

        describe("high_arc_of", []() {
            it("should extract high arc from node", [&]() {
                const node n = node(6,13,
                                      ptr_uint64(8,21),
                                      ptr_uint64(9,8));

                const arc_t arc = high_arc_of(n);

                AssertThat(arc.source().label(), Is().EqualTo(6u));
                AssertThat(arc.source().id(), Is().EqualTo(13u));
                AssertThat(arc.target().label(), Is().EqualTo(9u));
                AssertThat(arc.target().id(), Is().EqualTo(8u));
              });
          });

        describe("node_of", []() {
            it("should combine low and high arcs into single node", [&]() {
                const arc_t low_arc = arc(ptr_uint64(17,42), ptr_uint64(9,8));
                const arc_t high_arc = arc(flag(ptr_uint64(17,42)), ptr_uint64(8,21));

                const node n = node_of(low_arc, high_arc);

                AssertThat(n.label(), Is().EqualTo(17u));
                AssertThat(n.id(), Is().EqualTo(42u));

                AssertThat(n.low(), Is().EqualTo(low_arc.target()));
                AssertThat(n.high(), Is().EqualTo(high_arc.target()));
              });
          });
      });
  });
