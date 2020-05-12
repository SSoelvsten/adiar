#include <tpie/tpie.h>
#include <tpie/tpie_log.h>
#include <tpie/sort.h>
#include <tpie/priority_queue.h>

/**
 * These test cases are all derivations of the TPIE Hello World program kindly
 * provided to us by Mathias Rav (Mortal):
 * https://github.com/Mortal/tpieex
 */

struct Item {
	unsigned long long a, b, c;
};

void create_test_stream_1(const char * filename) {
	tpie::file_stream<int> xs;
  // Remove "compression_normal" to create an uncompressed stream instead.
	xs.open(filename, tpie::open::write_only | tpie::open::compression_normal);
	xs.write(42);
	xs.write(7);
	xs.write(18);
}

go_bandit([]() {
    tpie::tpie_init();

    size_t available_memory_mb = 128;
    tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

    describe("TPIE", [&]() {
        auto test_stream_1_name = "test_stream_1.tpie";
        create_test_stream_1(test_stream_1_name);

        it("should read stream forwards", [&test_stream_1_name]() {
            tpie::file_stream<int> xs;
            xs.open(test_stream_1_name, tpie::open::read_only);

            AssertThat(xs.can_read(), Is().True());
            AssertThat(xs.read(), Is().EqualTo(42));

            AssertThat(xs.can_read(), Is().True());
            AssertThat(xs.read(), Is().EqualTo(7));

            AssertThat(xs.can_read(), Is().True());
            AssertThat(xs.read(), Is().EqualTo(18));

            AssertThat(xs.can_read(), Is().False());
          });

        it("should read stream backwards", [&test_stream_1_name]() {
            tpie::file_stream<int> xs;
            xs.open(test_stream_1_name, tpie::open::read_only);
            xs.seek(0, tpie::file_stream_base::end);

            AssertThat(xs.can_read_back(), Is().True());
            AssertThat(xs.read_back(), Is().EqualTo(18));

            AssertThat(xs.can_read_back(), Is().True());
            AssertThat(xs.read_back(), Is().EqualTo(7));

            AssertThat(xs.can_read_back(), Is().True());
            AssertThat(xs.read_back(), Is().EqualTo(42));

            AssertThat(xs.can_read_back(), Is().False());
          });

        it("should sort stream", [&test_stream_1_name]() {
            tpie::file_stream<int> xs;
            xs.open(test_stream_1_name);

            tpie::progress_indicator_null pi;
            tpie::sort(xs, [&](const int& a, const int& b) -> bool { return a < b; }, pi);

            AssertThat(xs.read(), Is().EqualTo(7));
            AssertThat(xs.read(), Is().EqualTo(18));
            AssertThat(xs.read(), Is().EqualTo(42));
          });

        // remember that after the sorting test, that the stream now is in sorted order.

        it("should seek prior position into stream", [&test_stream_1_name]() {
            tpie::file_stream<int> xs;
            xs.open(test_stream_1_name);

            xs.read(); // Pop 7

            auto position = xs.get_position();
            xs.read(); // Pop 18

            xs.read(); // Pop 42

            xs.set_position(position);
            AssertThat(xs.read(), Is().EqualTo(18));
          });

        it("should sort data generated on the fly with merge_sorter", [&]() {
            // Setup sorter
            auto pred = [&](const int& a, const int& b) -> bool { return a < b; };
            tpie::merge_sorter<int, true, decltype(pred)> sorter(pred);

            sorter.set_available_memory(tpie::get_memory_manager().available());
            tpie::progress_indicator_null pi;

            // Start sorter
            sorter.begin();

            // Push into sorter
            sorter.push(42);
            sorter.push(7);
            sorter.push(18);

            // Finalize sorter
            sorter.end();
            sorter.calc(pi);

            // Pop from sorter
            AssertThat(sorter.can_pull(), Is().True());
            AssertThat(sorter.pull(), Is().EqualTo(7));
            AssertThat(sorter.pull(), Is().EqualTo(18));
            AssertThat(sorter.pull(), Is().EqualTo(42));
            AssertThat(sorter.can_pull(), Is().False());
          });

        it("should sort data with priority_queue", [&]() {
            tpie::priority_queue<int> pq;

            // Push a little
            pq.push(42);
            pq.push(7);
            pq.push(18);

            // Pop one
            AssertThat(pq.size(), Is().EqualTo(3));
            AssertThat(pq.empty(), Is().False());

            AssertThat(pq.top(), Is().EqualTo(7));
            pq.pop();

            // Push some more
            pq.push(4);

            // Pop the rest
            AssertThat(pq.top(), Is().EqualTo(4));
            pq.pop();
            AssertThat(pq.top(), Is().EqualTo(18));
            pq.pop();
            AssertThat(pq.top(), Is().EqualTo(42));
            pq.pop();
          });
      });

      tpie::tpie_finish();
  });

