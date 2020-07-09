#include <tpie/tpie.h>

#include <coom/data.cpp>

#include <coom/count_paths.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Count Paths", [&]() {
        /*
             1
            / \
            | 2
            |/ \
            3  |
           / \ /
           F  4
             / \
             F T
        */

        auto sink_T = create_sink(true);
        auto sink_F = create_sink(false);

        tpie::file_stream<node> obdd;
        obdd.open();

        auto n4 = create_node(3,0, sink_F, sink_T);
        obdd.write(n4);

        auto n3 = create_node(2,0, sink_F, n4.node_ptr);
        obdd.write(n3);

        auto n2 = create_node(1,0, n3.node_ptr, sink_T);
        obdd.write(n2);

        auto n1 = create_node(0,0, n3.node_ptr, n2.node_ptr);
        obdd.write(n1);

        it("can count number of non-disjunct paths", [&obdd]() {
            AssertThat(coom::count_paths(obdd), Is().EqualTo(8));
          });

        it("can count paths leading to T sinks", [&obdd]() {
            auto number_of_true_paths = coom::count_paths(obdd, is_true);
            AssertThat(number_of_true_paths, Is().EqualTo(3));
          });

        it("can count paths leading to F sinks", [&obdd]() {
            auto number_of_false_paths = coom::count_paths(obdd, is_false);
            AssertThat(number_of_false_paths, Is().EqualTo(5));
          });

        it("can count paths leading to any sinks", [&obdd]() {
            auto number_of_false_paths = coom::count_paths(obdd, is_any);
            AssertThat(number_of_false_paths, Is().EqualTo(8));
          });

        it("can count paths on a never happy predicate", [&obdd]() {
            auto all_paths_rejected = coom::count_paths(obdd,
                                                        [](uint64_t sink) -> bool {
                                                          return false;
                                                        });
            AssertThat(all_paths_rejected, Is().EqualTo(0));
          });

        it("should count no paths in a true sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd;
            obdd.open();
            obdd.write(create_sink_node(true));

            AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
          });

        it("should count no paths in a false sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd;
            obdd.open();
            obdd.write(create_sink_node(false));

            AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
          });
      });
  });
