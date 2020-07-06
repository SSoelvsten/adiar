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

        it("can count paths with a sink predicate", [&obdd]() {
            auto number_of_true_paths = coom::count_paths(obdd, is_true);
            AssertThat(number_of_true_paths, Is().EqualTo(3));

            auto number_of_false_paths = coom::count_paths(obdd, is_false);
            AssertThat(number_of_false_paths, Is().EqualTo(5));

            auto all_paths_rejected = coom::count_paths(obdd,
                                                        [](uint64_t sink) -> bool {
                return false;
              });
            AssertThat(all_paths_rejected, Is().EqualTo(0));
          });

        it("can count paths with a node predicate", [&obdd]() {
            auto number_of_even_only_paths = coom::count_paths(obdd,
                                                          [](node n) -> bool {
                return label_of(n) % 2 == 0;
              });
            AssertThat(number_of_even_only_paths, Is().EqualTo(1));

            auto number_of_odd_only_paths = coom::count_paths(obdd,
                                                         [](node n) -> bool {
                return label_of(n) % 2 == 1;
              });
            AssertThat(number_of_odd_only_paths, Is().EqualTo(1));

            auto number_of_paths_less_than_3 = coom::count_paths(obdd,
                                                                 [](node n) -> bool {
                return label_of(n) < 3;
              });
            AssertThat(number_of_paths_less_than_3, Is().EqualTo(4));
          });

        it("can count paths with a node and sink predicate", [&obdd]() {
            auto number_of_even_paths_less_than_3 = coom::count_paths(obdd,
                                                                      [](node n) -> bool {
                                                                        return label_of(n) < 3;
                                                                      },
                                                                      is_false);
            AssertThat(number_of_even_paths_less_than_3, Is().EqualTo(2));
          });

        it("should count no paths in a true sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd;
            obdd.open();
            obdd.write(create_sink_node(true));

            AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd, [](node n) -> bool { return label_of(n) < 3; }),
                       Is().EqualTo(0));
          });

        it("should count no paths in a false sink-only OBDD", [&]() {
            tpie::file_stream<node> obdd;
            obdd.open();
            obdd.write(create_sink_node(false));

            AssertThat(coom::count_paths(obdd), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd, is_true), Is().EqualTo(0));
            AssertThat(coom::count_paths(obdd,
                                         [](node n) -> bool { return label_of(n) % 3 == 0; },
                                         is_false),
                       Is().EqualTo(0));
          });
      });
  });
