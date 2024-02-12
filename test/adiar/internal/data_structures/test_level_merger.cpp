#include "../../../test.h"

#include <adiar/internal/data_structures/level_merger.h>

go_bandit([]() {
  describe("adiar/internal/level_merger.h", []() {
    describe("level_merger<shared_levelized_file<int>, ...>", [&]() {
      it("can pull from one level_info stream", [&]() {
        shared_levelized_file<int> f;
        { // Garbage collect the writer
          levelized_file_writer<int> fw(f);

          fw.push(level_info(4, 1u));
          fw.push(level_info(3, 2u));
          fw.push(level_info(2, 2u));
          fw.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 1u> merger;

        merger.hook({ f });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(4u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can peek from one level_info streams", [&]() {
        shared_levelized_file<int> f;

        { // Garbage collect the writer
          levelized_file_writer<int> fw(f);

          fw.push(level_info(4, 1u));
          fw.push(level_info(3, 2u));
          fw.push(level_info(2, 1u));
          fw.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 1> merger;

        merger.hook({ f });

        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can pull from merge of two level_info streams, where one is empty [1]", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writer
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams, where one is empty [2]", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writer
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(1, 1u));
          fw1.push(level_info(2, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [1]", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(4, 1u));
          fw1.push(level_info(2, 2u));
          fw1.push(level_info(1, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(4, 1u));
          fw2.push(level_info(3, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(4u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [2] (std::less)", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(2, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [2] (std::greater)", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(2, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can peek merge of two level_info stream", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(4, 3u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.peek(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.peek(), Is().EqualTo(4u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can merge levels in reverse", [&]() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(4, 3u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2, true> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.peek(), Is().EqualTo(4u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.peek(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(1u));
      });

      it("can pull, even after the original files have been deleted", [&]() {
        shared_levelized_file<int> f1 = shared_levelized_file<int>();
        shared_levelized_file<int> f2 = shared_levelized_file<int>();

        { // Garbage collect the writers
          levelized_file_writer<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_file_writer<int> fw2(f2);

          fw2.push(level_info(4, 1u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        f1.reset();
        f2.reset();

        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });
    });

    describe("level_merger<shared_file<label_type>, ...>", [&]() {
      it("can use a single label_file", [&]() {
        shared_file<ptr_uint64::label_type> f;

        { // Garbage collect the writers
          label_writer w(f);
          w << 0 << 2 << 3;
        }

        level_merger<shared_file<ptr_uint64::label_type>, std::less<>, 1> merger;

        merger.hook({ f });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge two label_files", [&]() {
        shared_file<ptr_uint64::label_type> f1;
        shared_file<ptr_uint64::label_type> f2;

        { // Garbage collect the writers
          label_writer w1(f1);
          w1 << 0 << 2 << 3;

          label_writer w2(f2);
          w2 << 0 << 1 << 3;
        }

        level_merger<shared_file<ptr_uint64::label_type>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });
    });
  });
});
