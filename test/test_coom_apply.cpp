#include <tpie/tpie.h>
#include <coom/apply.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Apply", [&]() {
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==
        //                  START
        tpie::file_stream<node> obdd_F_1;
        obdd_F_1.open();

        obdd_F_1.write(create_sink_node(false));

        tpie::file_stream<node> obdd_F_2;
        obdd_F_2.open();

        obdd_F_2.write(create_sink_node(false));

        tpie::file_stream<node> obdd_T_1;
        obdd_T_1.open();

        obdd_T_1.write(create_sink_node(true));

        tpie::file_stream<node> obdd_T_2;
        obdd_T_2.open();

        obdd_T_2.write(create_sink_node(true));

        //                   END
        // == CREATE SINK-ONLY OBDD FOR UNIT TESTS ==

        it("should XOR F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, xor_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should XOR T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, xor_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should AND F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should AND T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, and_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_F_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should OR T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_F_2, or_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES F and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_F_1, obdd_T_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES T and F sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_F_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(false)));

            AssertThat(out_nodes.can_read(), Is().False());
          });

        it("should IMPLIES T and T sink-only OBDDs", [&]() {
            tpie::file_stream<node> out_nodes;
            out_nodes.open();

            apply(obdd_T_1, obdd_T_2, implies_op, out_nodes);

            out_nodes.seek(0);

            AssertThat(out_nodes.can_read(), Is().True());
            AssertThat(out_nodes.read(), Is().EqualTo(create_sink_node(true)));

            AssertThat(out_nodes.can_read(), Is().False());
          });
      });
  });
