#include <coom/data.cpp>
#include <coom/obdd.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: OBDD", []() {

        describe("Sink-only OBDD", [&](){
            OBDD_sink obdd_F(false);
            OBDD_sink obdd_T(true);

            it("should claim to be sink-only", [&obdd_F, &obdd_T]() {
                AssertThat(obdd_F.is_sink(), Is().True());
                AssertThat(obdd_T.is_sink(), Is().True());
              });

            it("should not claim to be node-based", [&obdd_F, &obdd_T]() {
                AssertThat(obdd_F.has_nodes(), Is().False());
                AssertThat(obdd_T.has_nodes(), Is().False());
              });

            it("should be castable to sink-only", [&obdd_F, &obdd_T]() {
                auto obdd_F_as_sink = obdd_F.as_sink();
                AssertThat(value_of(obdd_F_as_sink), Is().False());

                auto obdd_T_as_sink = obdd_T.as_sink();
                AssertThat(value_of(obdd_T_as_sink), Is().True());
              });
          });

        describe("unpersisted OBDD with nodes", [&](){
            // Create a small realistic OBDD
            auto obdd_filename = "coom_obdd_test_1.tpie";
            OBDD_node obdd(obdd_filename);
            auto obdd_nodes = obdd.open_filestream();

            obdd_nodes.write(create_node(2, MAX_ID, create_sink(false), create_sink(true)));
            obdd_nodes.write(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_sink(true)));
            obdd_nodes.write(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID)));

            obdd_nodes.close();

            it("should not claim to be sink-only", [&obdd]() {
                AssertThat(obdd.is_sink(), Is().False());
                AssertThat(obdd.is_sink(), Is().False());
              });

            it("should not claim to be node-based", [&obdd]() {
                AssertThat(obdd.has_nodes(), Is().True());
                AssertThat(obdd.has_nodes(), Is().True());
              });

            it("should be castable to a nodes OBDD", [&obdd]() {
                auto obdd_casted = obdd.as_node();
                AssertThat(obdd.negated, Is().False());

                auto obdd_nodes = obdd_casted.open_filestream();

                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, create_sink(false), create_sink(true))));
                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_sink(true))));
                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
                AssertThat(obdd_nodes.can_read(), Is().False());
              });
          });

        describe("persisted OBDD with nodes", [&](){
            // Create a small realistic OBDD
            auto obdd_filename = "coom_obdd_test_2.tpie";
            OBDD_node obdd(obdd_filename, true);
            auto obdd_nodes = obdd.open_filestream();

            obdd_nodes.write(create_node(2, MAX_ID, create_sink(false), create_sink(true)));
            obdd_nodes.write(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_sink(true)));
            obdd_nodes.write(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID)));

            obdd_nodes.close();

            it("should be castable and readable", [&obdd]() {
                auto obdd_casted = obdd.as_node();
                AssertThat(obdd.negated, Is().False());

                auto obdd_nodes = obdd_casted.open_filestream();

                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(2, MAX_ID, create_sink(false), create_sink(true))));
                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_sink(true))));
                AssertThat(obdd_nodes.can_read(), Is().True());

                AssertThat(obdd_nodes.read(), Is().EqualTo(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID))));
                AssertThat(obdd_nodes.can_read(), Is().False());
              });
          });
      });
  });
