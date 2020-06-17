#include <tpie/tpie.h>
#include <tpie/tempname.h>
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
    describe("TPIE", [&]() {
        auto test_stream_1_name = "test_stream_1.tpie";
        create_test_stream_1(test_stream_1_name);


        describe("File Streams", [&test_stream_1_name]() {
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

            it("should overwrite entry in the middle of the stream", [&]() {
                tpie::file_stream<int> xs;
                xs.open();
                AssertThat(xs.can_read(), Is().False());

                xs.write(1);
                xs.write(2);
                xs.write(3);

                AssertThat(xs.can_read(), Is().False());

                // go back to beginning (one can only seek the beginning with 0 or the end).
                xs.seek(0);

                // read the first again and overwrite the next
                AssertThat(xs.read(), Is().EqualTo(1));
                xs.write(42);
                AssertThat(xs.read(), Is().EqualTo(3));

                // go back to beginning, and check the change is there
                xs.seek(0);

                AssertThat(xs.read(), Is().EqualTo(1));
                AssertThat(xs.read(), Is().EqualTo(42));
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
          });

        describe("Merge Sort Queue", [&test_stream_1_name]() {
            it("should sort data", [&]() {
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
          });

        describe("Priority Queue", [&test_stream_1_name]() {
            it("should sort data on the fly", [&]() {
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

        describe("Temporary Files", [&]() {
            tpie::temp_file temporary_file = tpie::temp_file(); // one can parse a persistency boolean if needed

            it("should open a (shareable) temporary file", [&]() {
                tpie::file_stream<int> xs;
                xs.open(temporary_file);

                xs.write(42);
              });

            it("should open an anonymous temporary files", [&]() {
                tpie::file_stream<int> xs;
                xs.open();
                AssertThat(xs.can_read(), Is().False());

                xs.write(1);
                xs.write(2);
                xs.write(3);

                AssertThat(xs.can_read(), Is().False());

                // go back to beginning
                xs.seek(0);

                AssertThat(xs.can_read(), Is().True());
                AssertThat(xs.read(), Is().EqualTo(1));

                AssertThat(xs.can_read(), Is().True());
                AssertThat(xs.read(), Is().EqualTo(2));

                AssertThat(xs.can_read(), Is().True());
                AssertThat(xs.read(), Is().EqualTo(3));

                // when going out of scope, xs is killed, and it will be cleaned up
              });


            it("should reopen the (shareable) temporary file", [&]() {
                tpie::file_stream<int> xs;
                xs.open(temporary_file);

                AssertThat(xs.can_read(), Is().True());
                AssertThat(xs.read(), Is().EqualTo(42));
              });
          });
      });
  });

