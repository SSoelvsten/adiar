#include <tpie/tpie.h>

#include <coom/data.cpp>

#include <coom/count.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Count", [&]() {
        /*
             1       ---- x0
            / \
            | 2      ---- x1
            |/ \
            3  |     ---- x2
           / \ /
           F  4      ---- x3
             / \
             F T
        */

        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        tpie::file_stream<node_t> obdd_1;
        obdd_1.open();

        node_t n4 = create_node(3,0, sink_F, sink_T);
        obdd_1.write(n4);

        node_t n3 = create_node(2,0, sink_F, n4.uid);
        obdd_1.write(n3);

        node_t n2 = create_node(1,0, n3.uid, n4.uid);
        obdd_1.write(n2);

        node_t n1 = create_node(0,0, n3.uid, n2.uid);
        obdd_1.write(n1);

        /*
                     ---- x0

              1      ---- x1
             / \
            2  |     ---- x2
           / \ /
           F  T
        */

        tpie::file_stream<node_t> obdd_2;
        obdd_2.open();

        node_t n2_2 = create_node(2,0, sink_F, sink_T);
        obdd_2.write(n2_2);

        node_t n2_1 = create_node(1,0, n2_2.uid, sink_T);
        obdd_2.write(n2_1);

        describe("Paths", [&]() {
            it("can count number of non-disjunct paths", [&obdd_1]() {
                AssertThat(coom::count_paths(obdd_1), Is().EqualTo(8));
              });

            it("can count paths leading to T sinks [1]", [&obdd_1]() {
                auto number_of_true_paths = coom::count_paths(obdd_1, is_true);
                AssertThat(number_of_true_paths, Is().EqualTo(3));
              });

            it("can count paths leading to T sinks [2]", [&obdd_2]() {
                auto number_of_true_paths = coom::count_paths(obdd_2, is_true);
                AssertThat(number_of_true_paths, Is().EqualTo(2));
              });

            it("can count paths leading to F sinks [1]", [&obdd_1]() {
                auto number_of_false_paths = coom::count_paths(obdd_1, is_false);
                AssertThat(number_of_false_paths, Is().EqualTo(5));
              });

            it("can count paths leading to F sinks [2]", [&obdd_2]() {
                auto number_of_false_paths = coom::count_paths(obdd_2, is_false);
                AssertThat(number_of_false_paths, Is().EqualTo(1));
              });

            it("can count paths leading to any sinks [1]", [&obdd_1]() {
                auto number_of_false_paths = coom::count_paths(obdd_1, is_any);
                AssertThat(number_of_false_paths, Is().EqualTo(8));
              });

            it("can count paths leading to any sinks [2]", [&obdd_2]() {
                auto number_of_false_paths = coom::count_paths(obdd_2, is_any);
                AssertThat(number_of_false_paths, Is().EqualTo(3));
              });

            it("can count paths on a never happy predicate", [&obdd_1]() {
                auto all_paths_rejected = coom::count_paths(obdd_1,
                                                            [](uint64_t /* sink */) -> bool {
                                                              return false;
                                                            });
                AssertThat(all_paths_rejected, Is().EqualTo(0));
              });

            it("should count no paths in a true sink-only OBDD", [&]() {
                tpie::file_stream<node_t> obdd;
                obdd.open();
                obdd.write(create_sink(true));

                AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
                AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
              });

            it("should count no paths in a false sink-only OBDD", [&]() {
                tpie::file_stream<node_t> obdd;
                obdd.open();
                obdd.write(create_sink(false));

                AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
                AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
              });

            it("should count paths of a root-only OBDD [1]", [&]() {
                tpie::file_stream<node_t> obdd_1;
                obdd_1.open();
                obdd_1.write(create_node(1,0, sink_F, sink_T));

                AssertThat(coom::count_assignments(obdd_1, is_false), Is().EqualTo(1));
                AssertThat(coom::count_assignments(obdd_1, is_true), Is().EqualTo(1));
              });

            it("should count paths of a root-only OBDD [2]", [&]() {
                // Technically not correct input, but...
                tpie::file_stream<node_t> obdd_2;
                obdd_2.open();
                obdd_2.write(create_node(1,0, sink_T, sink_T));

                AssertThat(coom::count_assignments(obdd_2, is_false), Is().EqualTo(0));
                AssertThat(coom::count_assignments(obdd_2, is_true), Is().EqualTo(2));
              });
          });

        describe("Assignment", [&]() {
            it("can count assignments leading to T sinks [1]", [&obdd_1]() {
                auto number_of_true_assignments = coom::count_assignments(obdd_1, is_true);
                AssertThat(number_of_true_assignments, Is().EqualTo(5));
              });

            it("can count assignments leading to T sinks [2]", [&obdd_2]() {
                auto number_of_true_assignments = coom::count_assignments(obdd_2, is_true);
                AssertThat(number_of_true_assignments, Is().EqualTo(3));
              });

            it("can count assignments leading to F sinks [1]", [&obdd_1]() {
                auto number_of_false_assignments = coom::count_assignments(obdd_1, is_false);
                AssertThat(number_of_false_assignments, Is().EqualTo(11));
              });

            it("can count assignments leading to F sinks [2]", [&obdd_2]() {
                auto number_of_false_assignments = coom::count_assignments(obdd_2, is_false);
                AssertThat(number_of_false_assignments, Is().EqualTo(1));
              });

            it("can count assignments leading to any sinks [1]", [&obdd_1]() {
                auto number_of_assignments = coom::count_assignments(obdd_1, is_any);
                AssertThat(number_of_assignments, Is().EqualTo(16));
              });

            it("can count assignments leading to any sinks [2]", [&obdd_2]() {
                auto number_of_assignments = coom::count_assignments(obdd_2, is_any);
                AssertThat(number_of_assignments, Is().EqualTo(4));
              });

            it("should count no assignments in a true sink-only OBDD", [&]() {
                tpie::file_stream<node_t> obdd;
                obdd.open();
                obdd.write(create_sink(true));

                AssertThat(coom::count_assignments(obdd, is_false), Is().EqualTo(0));
                AssertThat(coom::count_assignments(obdd, is_true), Is().EqualTo(0));
              });

            it("should count no assignments in a false sink-only OBDD", [&]() {
                tpie::file_stream<node_t> obdd;
                obdd.open();
                obdd.write(create_sink(false));

                AssertThat(coom::count_assignments(obdd, is_false), Is().EqualTo(0));
                AssertThat(coom::count_assignments(obdd, is_true), Is().EqualTo(0));
              });

            it("should count assignments of a root-only OBDD [1]", [&]() {
                tpie::file_stream<node_t> obdd_1;
                obdd_1.open();
                obdd_1.write(create_node(1,0, sink_F, sink_T));

                AssertThat(coom::count_assignments(obdd_1, is_false), Is().EqualTo(1));
                AssertThat(coom::count_assignments(obdd_1, is_true), Is().EqualTo(1));
              });

            it("should count assignments of a root-only OBDD [2]", [&]() {
                // Technically not correct input, but...
                tpie::file_stream<node_t> obdd_2;
                obdd_2.open();
                obdd_2.write(create_node(1,0, sink_T, sink_T));

                AssertThat(coom::count_assignments(obdd_2, is_false), Is().EqualTo(0));
                AssertThat(coom::count_assignments(obdd_2, is_true), Is().EqualTo(2));
              });
          });
      });
  });
