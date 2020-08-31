#include <coom/pred.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Predicates", []() {
        describe("is_sink", []() {
            it("should accept the correct sink-only OBDD [1]", [&]() {
                tpie::file_stream<node_t> sink_T;
                sink_T.open();
                sink_T.write(create_sink(true));

                AssertThat(is_sink(sink_T, is_true), Is().True());
              });

            it("should accept the correct sink-only OBDD [2]", [&]() {
                tpie::file_stream<node_t> sink_F;
                sink_F.open();
                sink_F.write(create_sink(false));

                AssertThat(is_sink(sink_F, is_false), Is().True());
              });


            it("should reject the incorrect sink-only OBDD [1]", [&]() {
                tpie::file_stream<node_t> sink_T;
                sink_T.open();
                sink_T.write(create_sink(true));

                AssertThat(is_sink(sink_T, is_false), Is().False());
              });

            it("should reject the incorrect sink-only OBDD [2]", [&]() {
                tpie::file_stream<node_t> sink_F;
                sink_F.open();
                sink_F.write(create_sink(false));

                AssertThat(is_sink(sink_F, is_true), Is().False());
              });

            it("should have any sink as default", [&]() {
                tpie::file_stream<node_t> sink_T;
                sink_T.open();
                sink_T.write(create_sink(true));

                AssertThat(is_sink(sink_T), Is().True());

                tpie::file_stream<node_t> sink_F;
                sink_F.open();
                sink_F.write(create_sink(false));

                AssertThat(is_sink(sink_F), Is().True());
              });

            it("should reject the non-sink OBDD [1]", [&]() {
                tpie::file_stream<node_t> x0;
                x0.open();
                x0.write(create_node(0,MAX_ID,
                                     create_sink_ptr(false),
                                     create_sink_ptr(true)));

                AssertThat(is_sink(x0, is_true), Is().False());
                AssertThat(is_sink(x0, is_false), Is().False());
                AssertThat(is_sink(x0, is_any), Is().False());
              });

            it("should reject the non-sink OBDD [2]", [&]() {
                tpie::file_stream<node_t> x0_and_x1;
                x0_and_x1.open();

                x0_and_x1.write(create_node(1, MAX_ID,
                                     create_sink_ptr(false),
                                     create_sink_ptr(true)));

                x0_and_x1.write(create_node(0, MAX_ID,
                                     create_sink_ptr(false),
                                     create_node_ptr(1, MAX_ID)));

                AssertThat(is_sink(x0_and_x1, is_true), Is().False());
                AssertThat(is_sink(x0_and_x1, is_false), Is().False());
                AssertThat(is_sink(x0_and_x1, is_any), Is().False());
              });
          });
      });
  });
