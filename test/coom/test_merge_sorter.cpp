#include <tpie/tpie.h>

#include <coom/merge_sorter.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Merge Sorter Decorator", []() {
        it("can sort some given input", [&]() {
            merge_sorter<int> sorter;
            sorter.init(tpie::get_memory_manager().available() / 2);

            sorter.push(42);
            sorter.push(7);
            sorter.push(21);

            sorter.sort();

            AssertThat(sorter.pull(), Is().EqualTo(7));
            AssertThat(sorter.pull(), Is().EqualTo(21));
            AssertThat(sorter.pull(), Is().EqualTo(42));
          });

        it("can peek next element", [&]() {
            merge_sorter<int> sorter;
            sorter.init();

            sorter.push(42);
            sorter.push(7);
            sorter.push(21);

            sorter.sort();

            AssertThat(sorter.peek(), Is().EqualTo(7));
            sorter.pull();
            sorter.pull();
            AssertThat(sorter.peek(), Is().EqualTo(42));
          });

        it("can sort by given predicate", [&]() {
            auto pred = [](const int& a, const int& b) -> bool { return a > b; };

            merge_sorter<int, decltype(pred)> sorter(pred);
            sorter.init();

            sorter.push(42);
            sorter.push(7);
            sorter.push(21);

            sorter.sort();

            AssertThat(sorter.pull(), Is().EqualTo(42));
            AssertThat(sorter.pull(), Is().EqualTo(21));
            AssertThat(sorter.pull(), Is().EqualTo(7));
          });
      });
  });
