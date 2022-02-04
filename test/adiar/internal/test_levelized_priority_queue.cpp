#include <adiar/internal/levelized_priority_queue.h>

using namespace adiar;

struct pq_test_data {
  label_t label;
  uint64_t nonce;
};

namespace adiar
{
  template <>
  struct FILE_CONSTANTS<pq_test_data>
  {
    static constexpr size_t files = 1u;
  };
}

bool operator== (const pq_test_data &a, const pq_test_data &b)
{
  return a.label == b.label && a.nonce == b.nonce;
}

struct pq_test_label_ext {
  static label_t label_of(const pq_test_data &d)
  {
    return d.label;
  }
};

struct pq_test_lt {
  bool operator()(const pq_test_data &a, const pq_test_data &b)
  {
    return a.label < b.label || (a.label == b.label && a.nonce < b.nonce);
  }
};

struct pq_test_gt {
  bool operator()(const pq_test_data &a, const pq_test_data &b)
  {
    return a.label > b.label || (a.label == b.label && a.nonce > b.nonce);
  }
};

typedef meta_file<pq_test_data> pq_test_file;
typedef meta_file_writer<pq_test_data> pq_test_writer;

template <typename file_t, size_t LOOK_AHEAD>
using test_priority_queue = levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                                     internal_sorter, internal_priority_queue,
                                                     file_t, 1u, std::less<label_t>,
                                                     1u,
                                                     LOOK_AHEAD>;

go_bandit([]() {
  describe("adiar/internal/levelized_priority_queue.h", []() {

    describe("label_merger", [&]() {
      it("can pull from one level_info stream", [&]() {
        pq_test_file f;
        { // Garbage collect the writer
          pq_test_writer fw(f);

          fw.unsafe_push(create_level_info(4,1u));
          fw.unsafe_push(create_level_info(3,2u));
          fw.unsafe_push(create_level_info(2,2u));
          fw.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 1u> merger;

        merger.hook({f});

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
        pq_test_file f;

        { // Garbage collect the writer
          pq_test_writer fw(f);

          fw.unsafe_push(create_level_info(4,1u));
          fw.unsafe_push(create_level_info(3,2u));
          fw.unsafe_push(create_level_info(2,1u));
          fw.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 1> merger;

        merger.hook({f});

        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can pull from merge of two level_info streams, where one is empty [1]", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writer
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 2> merger;

        merger.hook({f1, f2});

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });


      it("can pull from merge of two level_info streams, where one is empty [2]", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writer
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(1,1u));
          fw1.unsafe_push(create_level_info(2,1u));
        }

        label_merger<pq_test_file, std::greater<>, 2> merger;

        merger.hook({f1, f2});

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [1]", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writers
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(4,1u));
          fw1.unsafe_push(create_level_info(2,2u));
          fw1.unsafe_push(create_level_info(1,1u));

          pq_test_writer fw2(f2);

          fw2.unsafe_push(create_level_info(4,1u));
          fw2.unsafe_push(create_level_info(3,1u));
        }

        label_merger<pq_test_file, std::less<>, 2> merger;

        merger.hook({f1, f2});

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
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writers
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(2,1u));

          pq_test_writer fw2(f2);

          fw2.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 2> merger;

        merger.hook({f1, f2});

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [2] (std::greater)", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writers
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(2,1u));
          pq_test_writer fw2(f2);

          fw2.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::greater<>, 2> merger;

        merger.hook({f1, f2});

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can peek merge of two level_info stream", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writers
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(4,2u));
          fw1.unsafe_push(create_level_info(2,1u));

          pq_test_writer fw2(f2);

          fw2.unsafe_push(create_level_info(4,3u));
          fw2.unsafe_push(create_level_info(3,2u));
          fw2.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 2> merger;

        merger.hook({f1, f2});

        AssertThat(merger.peek(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.peek(), Is().EqualTo(4u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can pull, even after the original files have been deleted", [&]() {
        pq_test_file* f1 = new pq_test_file();
        pq_test_file* f2 = new pq_test_file();

        { // Garbage collect the writers
          pq_test_writer fw1(*f1);

          fw1.unsafe_push(create_level_info(4,2u));
          fw1.unsafe_push(create_level_info(2,1u));

          pq_test_writer fw2(*f2);

          fw2.unsafe_push(create_level_info(4,1u));
          fw2.unsafe_push(create_level_info(3,2u));
          fw2.unsafe_push(create_level_info(1,1u));
        }

        label_merger<pq_test_file, std::less<>, 2> merger;

        merger.hook({*f1, *f2});

        delete f1;
        delete f2;

        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can use a single label_file", [&]() {
          label_file f;

          { // Garbage collect the writers
            label_writer w(f);
            w << 0 << 2 << 3;
          }

          label_merger<label_file, std::less<>, 1> merger;

          merger.hook({f});

          AssertThat(merger.can_pull(), Is().True());
          AssertThat(merger.pull(), Is().EqualTo(0u));

          AssertThat(merger.can_pull(), Is().True());
          AssertThat(merger.pull(), Is().EqualTo(2u));

          AssertThat(merger.can_pull(), Is().True());
          AssertThat(merger.pull(), Is().EqualTo(3u));

          AssertThat(merger.can_pull(), Is().False());
        });

      it("can merge two label_files", [&]() {
          label_file f1;
          label_file f2;

          { // Garbage collect the writers
            label_writer w1(f1);
            w1 << 0 << 2 << 3;

            label_writer w2(f2);
            w2 << 0 << 1 << 3;
          }

          label_merger<label_file, std::less<>, 2> merger;

          merger.hook({f1, f2});

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

    ////////////////////////////////////////////////////////////////////////////
    // TODO: All the 'set up' tests should be removed. Instead, move this
    //       initialisation cases into some simple pushing and pulling tests.
    //
    // TODO: Most level files should be replaced with a simpler label_file (and
    //       use the << operator). Yet, we of course need one test or two with a
    //       meta file.
    //
    // TODO: Are we not missing some unit tests for the very simple accessors?

    describe("levelized_priority_queue<..., INIT_LEVEL=1, LOOK_AHEAD=1>", [&]() {
      //////////////////////////////////////////////////////////////////////////
      //                          initialisation                              //
      it("initialises #levels = 0", [&]() {
          pq_test_file f;

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());
        });

      it("initialises with #levels = 1 (which is skipped)", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 2;
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());
        });

      it("initialises with #levels = 2 (#buckets = 1)", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 1 << 2;
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));
        });

      it("initialises with #buckets == #levels", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 1       // skipped
               << 3 << 4; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));
        });

      it("initialises with #buckets < #levels", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 1      // skipped
               << 2 << 4 // buckets
               << 5;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));
        });

      //////////////////////////////////////////////////////////////////////////
      //                           level state                                //

      // TODO: allow us to forward with no elements within? If so, then we might
      // want to consider simplifying these unit tests.

      describe(".setup_next_level()", [&]() {
        it("can forward until the first non-empty bucket [1]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 0      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {2, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));
        });

        it("can forward until the first non-empty bucket [2]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {3, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("can forward up until the overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(4u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().False());
        });

        it("can forward until next bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {3, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.empty_level(), Is().False());

          pq.pop();

          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("can forward past buckets until top of overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1       // skipped
               << 2 << 3  // buckets
               << 4 << 5; // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.empty_level(), Is().False());

          pq.pop();

          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(4u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(5u));
        });

        it("can relabel buckets until top of overflow queue [1]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1                 // skipped
               << 2 << 3            // buckets
               << 4 << 5            // overflow that turn into buckets
               << 6 << 7 << 8 << 9; // overflow that will relabel
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {8, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(8u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(9u));
        });

        it("can relabel buckets until top of overflow queue [2]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1             // skipped
               << 2 << 3 << 4   // buckets (after element in 2)
               << 5 << 6        // overflow that turn into buckets
               << 7             // overflow that is skipped
               << 8 << 9 << 10; // overflow that will relabel
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {8, 1});
          pq.push(pq_test_data {2, 2});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.empty_level(), Is().False());
          pq.pop();
          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(8u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(9u));
        });

        it("can relabel fewer levels than buckets", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1            // skipped
               << 2 << 3 << 4  // buckets (after element in 2)
               << 5 << 6       // overflow that turn into buckets
               << 7            // overflow that is skipped
               << 8;           // overflow that will relabel
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {8, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level();

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(8u));

          AssertThat(pq.has_next_level(), Is().False());
        });
      });

      describe(".setup_next_level(stop_label)", [&]() {
        it("forward to known level of first bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {3, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(2u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));
        });

        it("forwards to known level of second bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(3u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("forwards to known level of next bucket with content", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {3, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(2u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level(3u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("forwards to known level of next bucket without content", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(2u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level(3u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("stops early at bucket with content", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(2u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(2u));

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));

          pq.setup_next_level(3u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("stops at known level of prior to content of overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1      // skipped
               << 2 << 3 // buckets
               << 4;     // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(3u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.empty_level(), Is().True());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(4u));
        });

        it("stops early at content of overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);

            fw << 1            // skipped
               << 2 << 3       // buckets
               << 4 << 5 << 6; // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));

          pq.setup_next_level(6u);

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(4u));

          AssertThat(pq.empty_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(5u));
        });

       it("does nothing when given unknown level prior to first bucket", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer fw(f);

              fw << 1      // skipped
                 << 3 << 4 // buckets
                 << 5;     // overflow
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.has_current_level(), Is().False());

            pq.push(pq_test_data {3, 1});

            AssertThat(pq.has_current_level(), Is().False());
            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(3u));

            pq.setup_next_level(2u);

            AssertThat(pq.has_current_level(), Is().False());
            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(3u));
          });

        it("forward to first bucket for unknown level prior to second bucket", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer fw(f);

              fw << 1      // skipped
                 << 2 << 4 // buckets
                 << 5;     // overflow
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.has_current_level(), Is().False());

            pq.push(pq_test_data {4, 1});

            AssertThat(pq.has_current_level(), Is().False());
            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(2u));

            pq.setup_next_level(3u);

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(2u));

            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(4u));
          });

        it("does nothing for unknown level prior to next bucket", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer fw(f);

              fw << 1      // skipped
                 << 2      // read bucket
                 << 4      // next buckets
                 << 5;     // overflow
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            pq.push(pq_test_data {2, 1});
            pq.setup_next_level();
            pq.pop();

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(2u));

            pq.push(pq_test_data {4, 2});

            pq.setup_next_level(3u);

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(2u));

            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(4u));
          });

        it("forward to next bucket for unknown level after it", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer fw(f);

              fw << 1      // skipped
                 << 2 << 3 // buckets
                 << 5;     // overflow
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            pq.push(pq_test_data {2, 1});
            pq.setup_next_level();
            pq.pop();

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(2u));

            pq.push(pq_test_data {5, 2});

            pq.setup_next_level(4u);

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(3u));

            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(5u));
          });

        it("can relabel for unknown level", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer fw(f);

              fw << 1              // skipped
                 << 2 << 3 << 4    // buckets (after element in 2)
                 << 5 << 6         // overflow that turn into buckets
                 << 7              // overflow that is skipped
                 << 8 << 10 << 11; // overflow that will relabel
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            pq.push(pq_test_data {2, 1});
            pq.setup_next_level();
            pq.pop();

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(2u));

            pq.push(pq_test_data {10, 2});

            pq.setup_next_level(9u);

            AssertThat(pq.has_current_level(), Is().True());
            AssertThat(pq.current_level(), Is().EqualTo(8u));

            AssertThat(pq.has_next_level(), Is().True());
            AssertThat(pq.next_level(), Is().EqualTo(10u));
          });
      });

      //////////////////////////////////////////////////////////////////////////
      //                          .push / pull                                //
      describe(".push(elem_t &) + pull()", [&]{
        it("can push when there are fewer levels than buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {2, 2});

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push to buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(4,2u)); // overflow
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {2, 2});

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

          pq.push(pq_test_data {3, 2});

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));

          pq.push(pq_test_data {3, 1});

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));

          pq.push(pq_test_data {4, 1});

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push to overflow queue [1]", [&]() {
            pq_test_file f;

            { // Garbage collect the writer early
              pq_test_writer fw(f);

              fw.unsafe_push(create_level_info(4,1u)); // overflow
              fw.unsafe_push(create_level_info(3,3u)); // bucket
              fw.unsafe_push(create_level_info(2,2u)); // bucket
              fw.unsafe_push(create_level_info(1,1u)); // skipped
            }

            test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            pq.push(pq_test_data {4, 1});

            pq.setup_next_level(); // 4

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

            AssertThat(pq.can_pull(), Is().False());
          });

        it("can push to overflow queue [2]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(4,2u)); // overflow
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // bucket
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {3, 1});
          pq.push(pq_test_data {4, 1});

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push to overflow queue [3]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(5,1u)); // .
            fw.unsafe_push(create_level_info(4,2u)); // overflow
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // bucket
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {5, 2});
          pq.push(pq_test_data {5, 1});
          pq.push(pq_test_data {3, 3});

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 3}));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 2}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can merge content of bucket with overflow queue", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(5,2u)); // .
            fw.unsafe_push(create_level_info(4,2u)); // overflow
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {5, 2}); // overflow

          pq.push(pq_test_data {4, 1}); // overflow

          pq.setup_next_level(); // read: 4, write: 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

          pq.push(pq_test_data {5, 1}); // bucket

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // read: 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 2}));

          AssertThat(pq.can_pull(), Is().False());
        });

        /////////////////////////////////////////////////////////////
        //                         TODO                            //
        it("can set up next level with a stop_label [1]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(4,2u));
            fw.unsafe_push(create_level_info(3,4u));
            fw.unsafe_push(create_level_info(2,2u));
            fw.unsafe_push(create_level_info(1,1));
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {4, 1});

          pq.setup_next_level(3u); // 3

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can set up next level with a stop_label [2]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(5,2u));
            fw.unsafe_push(create_level_info(4,4u));
            fw.unsafe_push(create_level_info(3,4u));
            fw.unsafe_push(create_level_info(2,2u));
            fw.unsafe_push(create_level_info(1,1u));
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {5, 1});

          pq.setup_next_level(4u); // 3

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can use buckets after relabelling", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(16,2u)); // .
            fw.unsafe_push(create_level_info(15,3u)); // .
            fw.unsafe_push(create_level_info(14,5u)); // .
            fw.unsafe_push(create_level_info(12,8u)); // .
            fw.unsafe_push(create_level_info(10,8u)); // .
            fw.unsafe_push(create_level_info(9,7u));  // .
            fw.unsafe_push(create_level_info(8,3u));  // .        (bucket)
            fw.unsafe_push(create_level_info(6,3u));  // overflow (bucket)
            fw.unsafe_push(create_level_info(5,3u));  // bucket
            fw.unsafe_push(create_level_info(4,2u));  // bucket
            fw.unsafe_push(create_level_info(1,1u));  // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {10, 2}); // overflow
          pq.push(pq_test_data {12, 3}); // overflow

          pq.setup_next_level(); // 10

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {10,2}));

          pq.push(pq_test_data {12, 1}); // bucket
          pq.push(pq_test_data {14, 1}); // overflow

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 12

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {12,1}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {12,3}));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 14

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {14,1}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can set up next level with a stop_label prior to the level of next bucket)", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer w(f);
              w << 0 // Skipped
                << 2 // Bucket
                << 3 // Bucket
                << 4 // Overflow
              ;
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {2, 1});

            pq.setup_next_level(1u); // not changed

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {2, 2});

            pq.setup_next_level(); // 2

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));

            AssertThat(pq.can_pull(), Is().False());
          });

        it("can set up next level with a stop_label for prior to second next bucket", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer w(f);
              w << 0 // Skipped
                << 1 // Bucket
                << 3 // Bucket
                << 4 // Overflow
                ;
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {3, 1});

            pq.setup_next_level(2u); // 1

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {3, 2});

            pq.setup_next_level(); // 3

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));

            AssertThat(pq.can_pull(), Is().False());
          });

        it("can set up next level with a stop_label prior to the level of the overflow queue", [&]()     {
            label_file f;

            { // Garbage collect the writer early
              label_writer w(f);
              w << 0 // Skipped
                << 1 // Bucket
                << 2 // Bucket
                << 4 // Overflow
                << 5 // .
              ;
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {4, 1});

            pq.setup_next_level(3u); // 2

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {4, 2});

            pq.setup_next_level(); // 4

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));

            AssertThat(pq.can_pull(), Is().False());
          });

        it("can relabel buckets with a stop_label for an unknown level", [&]() {
            label_file f;

            { // Garbage collect the writer early
              label_writer w(f);
              w << 0 // Skipped
                << 1 // Bucket
                << 2 // Bucket
                << 3 // Overflow
                << 5 // .
                << 6 // .
              ;
            }

            test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {5, 1});

            pq.setup_next_level(4u); // 3

            AssertThat(pq.can_pull(), Is().False());

            pq.push(pq_test_data {5, 2});

            pq.setup_next_level(); // 5

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));

            AssertThat(pq.can_pull(), Is().True());
            AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 2}));

            AssertThat(pq.can_pull(), Is().False());
          });
      });

      describe(".pop()", [&]{
        // TODO
      });

      //////////////////////////////////////////////////////////////////////////
      //                           .can_pull()                                //
      describe(".empty_level() / .can_pull()", [&]{
        label_file f;

        { // Garbage collect the writer early
          label_writer fw(f);

          fw << 1            // skipped
             << 2 << 3       // buckets
             << 4 << 5 << 6; // overflow
        }

        it("cannot pull after initialisation", [&]() {
          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.can_pull(), Is().False());
        });

        it("shows element after forwarding to level", [&]() {
          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data { 2,1 });
          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
        });

        it("shows a level becomes empty", [&]() {
          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data { 2,1 });
          pq.setup_next_level(); // 2

          AssertThat(pq.has_current_level(), Is().True());

          AssertThat(pq.empty_level(), Is().False());
          AssertThat(pq.can_pull(), Is().True());

          pq.pop();

          AssertThat(pq.empty_level(), Is().True());
          AssertThat(pq.can_pull(), Is().False());
        });

        it("shows forwarding to an empty level", [&]() {
          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data { 3,1 });
          pq.setup_next_level(2); // 2

          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.empty_level(), Is().True());
          AssertThat(pq.can_pull(), Is().False());
        });
      });

      describe(".top() / .peek()", [&]{
        it("can look into bucket without side-effects", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(6,2u)); // .
            fw.unsafe_push(create_level_info(5,4u)); // .
            fw.unsafe_push(create_level_info(4,5u)); // overflow
            fw.unsafe_push(create_level_info(3,4u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {2, 42}); // bucket
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.setup_next_level();

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.peek(), Is().EqualTo(pq_test_data {2, 42}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 42}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can look into overflow priority queue without side-effects", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(6,2u)); // .
            fw.unsafe_push(create_level_info(5,2u)); // .
            fw.unsafe_push(create_level_info(4,3u)); // overflow
            fw.unsafe_push(create_level_info(3,2u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {5, 3});  // overflow
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.setup_next_level();

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.peek(), Is().EqualTo(pq_test_data {5, 3}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 3}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        // TODO: peek -> push -> peek
      });

      describe(".size()", [&]{
        it("increments on push to bucket [1]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {1, 1});
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {2, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));
        });

        it("increments on push to bucket [2]", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {1, 1});
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.setup_next_level();

          pq.push(pq_test_data {2, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));
        });

        it("increments on push to overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2  // buckets
               << 3 << 4; // overflow

          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {4, 1});
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {5, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));
        });

        it("decrements on pull from bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {1, 1});
          pq.push(pq_test_data {1, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));
          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 2}));
          AssertThat(pq.size(), Is().EqualTo(0u));
        });

        it("decrements on pull from overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2  // buckets
               << 3 << 4; // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {4, 1});
          pq.push(pq_test_data {4, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
          AssertThat(pq.size(), Is().EqualTo(0u));
        });

        it("decrements on pull from bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {1, 1});
          pq.push(pq_test_data {1, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          pq.pop();
          AssertThat(pq.size(), Is().EqualTo(1u));
          pq.pop();
          AssertThat(pq.size(), Is().EqualTo(0u));
        });

        it("decrements on pull from overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2  // buckets
               << 3 << 4; // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {4, 1});
          pq.push(pq_test_data {4, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          pq.pop();
          AssertThat(pq.size(), Is().EqualTo(1u));
          pq.pop();
          AssertThat(pq.size(), Is().EqualTo(0u));
        });

        it("is unchanged on top from bucket", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2; // buckets
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {1, 1});
          pq.push(pq_test_data {1, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(pq.top(), Is().EqualTo(pq_test_data {1, 1}));
          AssertThat(pq.size(), Is().EqualTo(2u));
        });

        it("is unchanged on top from overflow queue", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 0       // skipped
               << 1 << 2  // buckets
               << 3 << 4; // overflow
          }

          test_priority_queue<label_file, 1> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {4, 1});
          pq.push(pq_test_data {4, 2});
          pq.setup_next_level();

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(pq.top(), Is().EqualTo(pq_test_data {4, 1}));
          AssertThat(pq.size(), Is().EqualTo(2u));
        });
      });
    });

    describe("levelized_priority_queue<..., pq_test_gt, ..., std::greater<label_g>, ...>", [&]() {
      label_file f;

      { // Garbage collect the writer early
        label_writer fw(f);

        fw << 3      // skipped
           << 2 << 1 // buckets
           << 0      // overflow
          ;
      }

      it("can sort elements from buckets", [&]() {
          levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_gt,
                                   internal_sorter, internal_priority_queue,
                                   label_file, 1u, std::greater<label_t>,
                                   1u,
                                   1u>
            pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {1, 1});
          pq.push(pq_test_data {2, 2});

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2,2}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2,1}));

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {1, 2});

          pq.setup_next_level(); // 1

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1,2}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1,1}));

          AssertThat(pq.can_pull(), Is().False());
      });

      it("can sort elements in overflow priority queue", [&]() {
          levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_gt,
                                   internal_sorter, internal_priority_queue,
                                   label_file, 1u, std::greater<label_t>,
                                   1u,
                                   1u>
            pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {0, 1});
          pq.push(pq_test_data {0, 2});

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 0

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {0,2}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {0,1}));

          AssertThat(pq.can_pull(), Is().False());
      });

      it("can merge elements from buckets and overflow", [&]() {
        levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_gt,
                                 internal_sorter, internal_priority_queue,
                                 label_file, 1u, std::greater<label_t>,
                                 1u,
                                 1u>
          pq({f}, tpie::get_memory_manager().available(), 32);

        AssertThat(pq.has_current_level(), Is().False());

        pq.push(pq_test_data {2, 1}); // bucket

        AssertThat(pq.can_pull(), Is().False());

        pq.setup_next_level(); // 2

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2,1}));

        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {1,1}); // bucket
        pq.push(pq_test_data {0,2}); // overflow

        pq.setup_next_level(); // 1

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1,1}));

        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {0,1}); // bucket

        pq.setup_next_level(); // 0

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {0,2}));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {0,1}));

        AssertThat(pq.can_pull(), Is().False());
      });
    });

    describe("levelized_priority_queue<..., INIT_LEVEL=0, LOOK_AHEAD=1>", [&]() {
      it("initialises #levels = 0", [&]() {
          label_file f;

          levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                   internal_sorter, internal_priority_queue,
                                   label_file, 1u, std::less<label_t>,
                                   0u,
                                   1u>
            pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.can_push(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());
        });

      it("initialises with #levels = 1 < #buckets", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 2;
          }

          levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                   internal_sorter, internal_priority_queue,
                                   label_file, 1u, std::less<label_t>,
                                   0u,
                                   1u>
            pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.can_push(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(2u));
        });

      it("initialises #buckets <= #levels", [&]() {
          label_file f;

          { // Garbage collect the writer early
            label_writer fw(f);
            fw << 1 << 3 << 4;
          }

          levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                   internal_sorter, internal_priority_queue,
                                   label_file, 1u, std::less<label_t>,
                                   0u,
                                   1u>
            pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());

          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.can_push(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(1u));
        });

      it("can push into and pull from root level bucket [1]", [&]() {
        pq_test_file f;

        { // Garbage collect the writer early
          pq_test_writer fw(f);

          fw.unsafe_push(create_level_info(1,1u)); // bucket
        }

        levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                 internal_sorter, internal_priority_queue,
                                 pq_test_file, 1u, std::less<label_t>,
                                 0u,
                                 1u>
          pq({f}, tpie::get_memory_manager().available(), 32);

        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {1, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {1, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.setup_next_level(); // 1

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.has_next_level(), Is().False());
      });

      it("can push into and pull from root level bucket [2]", [&]() {
        pq_test_file f;

        { // Garbage collect the writer early
          pq_test_writer fw(f);

          fw.unsafe_push(create_level_info(4,2u)); // .
          fw.unsafe_push(create_level_info(3,3u)); // overflow
          fw.unsafe_push(create_level_info(2,2u)); // bucket
          fw.unsafe_push(create_level_info(1,1u)); // bucket
        }

        levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                 internal_sorter, internal_priority_queue,
                                 pq_test_file, 1u, std::less<label_t>,
                                 0u,
                                 1u>
          pq({f}, tpie::get_memory_manager().available(), 32);

        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {1, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {1, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.setup_next_level(); // 1

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.has_next_level(), Is().True());
      });
    });

    describe("levelized_priority_queue<..., FILES=2, ...>", [&]() {
      it("can push into and pull from merge of two meta files' levels", [&]() {
        pq_test_file f1;
        pq_test_file f2;

        { // Garbage collect the writer early
          pq_test_writer fw1(f1);

          fw1.unsafe_push(create_level_info(4,2u));
          fw1.unsafe_push(create_level_info(3,3u));
          fw1.unsafe_push(create_level_info(1,1u));

          pq_test_writer fw2(f2);

          fw2.unsafe_push(create_level_info(2,1u));
          fw2.unsafe_push(create_level_info(1,2u));
        }

        levelized_priority_queue<pq_test_data, pq_test_label_ext, pq_test_lt,
                                 internal_sorter, internal_priority_queue,
                                 pq_test_file, 2u, std::less<label_t>,
                                 0u,
                                 1u>
          pq({f1,f2}, tpie::get_memory_manager().available(), 32);

        AssertThat(pq.has_current_level(), Is().False());

        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {1, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {1, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.push(pq_test_data {2, 3});
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_current_level(), Is().False());
        AssertThat(pq.has_next_level(), Is().True());
        pq.setup_next_level();
        AssertThat(pq.has_current_level(), Is().True());
        AssertThat(pq.current_level(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 2}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_current_level(), Is().True());
        AssertThat(pq.current_level(), Is().EqualTo(1u));
        AssertThat(pq.has_next_level(), Is().True());
        pq.setup_next_level();
        AssertThat(pq.has_current_level(), Is().True());
        AssertThat(pq.current_level(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 3}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.has_next_level(), Is().True());
      });
    });

    describe("levelized_priority_queue<..., INIT_LEVEL=1, LOOK_AHEAD=3>", [&]() {
      it("initialises with #levels = 0", [&]() {
          pq_test_file f;

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());

          AssertThat(pq.can_pull(), Is().False());

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());
        });

      it("initialises with #levels = 1", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().False());
        });

      it("initialises with 1 < #levels < #buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(4,1u)); // bucket
            fw.unsafe_push(create_level_info(3,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());

          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          AssertThat(pq.next_level(), Is().EqualTo(3u));
        });

      it("initialises with #buckets < #levels", [&]() {
        pq_test_file f;

        { // Garbage collect the writer early
          pq_test_writer fw(f);

          fw.unsafe_push(create_level_info(8,1u)); // .
          fw.unsafe_push(create_level_info(7,3u)); // .
          fw.unsafe_push(create_level_info(6,4u)); // .
          fw.unsafe_push(create_level_info(5,6u)); // .
          fw.unsafe_push(create_level_info(4,5u)); // overflow
          fw.unsafe_push(create_level_info(3,4u)); // bucket
          fw.unsafe_push(create_level_info(2,2u)); // bucket
          fw.unsafe_push(create_level_info(1,1u)); // skipped
        }

        test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_current_level(), Is().False());
        AssertThat(pq.has_next_level(), Is().True());
        AssertThat(pq.next_level(), Is().EqualTo(2u));
      });

      describe(".push(elem_t &) + pull()", [&]{
        it("can push into and pull from buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(7,2u)); // .
            fw.unsafe_push(create_level_info(6,3u)); // overflow
            fw.unsafe_push(create_level_info(5,6u)); // bucket
            fw.unsafe_push(create_level_info(4,8u)); // bucket
            fw.unsafe_push(create_level_info(3,4u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {2, 2});
          pq.push(pq_test_data {3, 42});
          pq.push(pq_test_data {3, 21});
          pq.push(pq_test_data {5, 3});
          AssertThat(pq.size(), Is().EqualTo(5u));

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
          AssertThat(pq.size(), Is().EqualTo(4u));

          pq.push(pq_test_data {3, 2});
          pq.push(pq_test_data {6, 2});
          pq.push(pq_test_data {4, 2});
          AssertThat(pq.size(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));
          AssertThat(pq.size(), Is().EqualTo(6u));

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {3, 3});
          pq.push(pq_test_data {5, 2});
          pq.push(pq_test_data {3, 1});
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          pq.push(pq_test_data {5, 1});
          pq.push(pq_test_data {4, 3});
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {6, 3});
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 3}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {7, 4});
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 21}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {6, 1});
          pq.push(pq_test_data {7, 3});
          AssertThat(pq.size(), Is().EqualTo(11u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 42}));
          AssertThat(pq.size(), Is().EqualTo(10u));

          pq.push(pq_test_data {7, 5});
          pq.push(pq_test_data {4, 1});
          AssertThat(pq.size(), Is().EqualTo(12u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
          AssertThat(pq.size(), Is().EqualTo(11u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 3}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {7, 1});
          pq.push(pq_test_data {7, 2});
          AssertThat(pq.size(), Is().EqualTo(11u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));
          AssertThat(pq.size(), Is().EqualTo(10u));

          pq.push(pq_test_data {6, 4});
          AssertThat(pq.size(), Is().EqualTo(11u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 2}));
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 3}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 6

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 1}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 2}));
          AssertThat(pq.size(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 3}));
          AssertThat(pq.size(), Is().EqualTo(6u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 4}));
          AssertThat(pq.size(), Is().EqualTo(5u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 7

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 1}));
          AssertThat(pq.size(), Is().EqualTo(4u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 2}));
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 3}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 4}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 5}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push into overflow queue [1]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,1u));  // .
            fw.unsafe_push(create_level_info(7,4u));  // .
            fw.unsafe_push(create_level_info(6,7u));  // overflow
            fw.unsafe_push(create_level_info(5,10u)); // bucket
            fw.unsafe_push(create_level_info(4,8u));  // bucket
            fw.unsafe_push(create_level_info(3,4u));  // bucket
            fw.unsafe_push(create_level_info(2,2u));  // bucket
            fw.unsafe_push(create_level_info(1,1u));  // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {6, 4});
          pq.push(pq_test_data {8, 2});
          AssertThat(pq.size(), Is().EqualTo(2u));

          pq.setup_next_level(); // 6

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 4}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 8

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push into overflow queue [2]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,2u)); // .
            fw.unsafe_push(create_level_info(7,3u)); // .
            fw.unsafe_push(create_level_info(6,4u)); // overflow
            fw.unsafe_push(create_level_info(5,5u)); // bucket
            fw.unsafe_push(create_level_info(4,3u)); // bucket
            fw.unsafe_push(create_level_info(3,2u)); // bucket
            fw.unsafe_push(create_level_info(2,1u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {8, 2});
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.setup_next_level(); // 8

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push into overflow queue [3]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(12,2u)); // .
            fw.unsafe_push(create_level_info(11,4u)); // .
            fw.unsafe_push(create_level_info(10,8u)); // .
            fw.unsafe_push(create_level_info(9,16u)); // .
            fw.unsafe_push(create_level_info(8,32u)); // .
            fw.unsafe_push(create_level_info(7,64u)); // .
            fw.unsafe_push(create_level_info(6,32u)); // overflow
            fw.unsafe_push(create_level_info(5,16u)); // bucket
            fw.unsafe_push(create_level_info(4,8u));  // bucket
            fw.unsafe_push(create_level_info(3,4u));  // bucket
            fw.unsafe_push(create_level_info(2,2u));  // bucket
            fw.unsafe_push(create_level_info(1,1u));  // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {10, 2});
          pq.push(pq_test_data {12, 3});
          pq.push(pq_test_data {10, 1});
          AssertThat(pq.size(), Is().EqualTo(3u));

          pq.setup_next_level(); // 10

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {10,1}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {10,2}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 12

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {12,3}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can skip unpushed levels until stop_label [1]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,1u)); // .
            fw.unsafe_push(create_level_info(7,1u)); // .
            fw.unsafe_push(create_level_info(6,2u)); // overflow
            fw.unsafe_push(create_level_info(5,3u)); // bucket
            fw.unsafe_push(create_level_info(4,4u)); // bucket
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {8, 2});
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.setup_next_level(3u); // 3

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 8

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can skip nonempty levels until stop_label [2]", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,1));  // .
            fw.unsafe_push(create_level_info(7,2u)); // .
            fw.unsafe_push(create_level_info(6,3u)); // overflow
            fw.unsafe_push(create_level_info(5,2u)); // bucket
            fw.unsafe_push(create_level_info(4,3u)); // bucket
            fw.unsafe_push(create_level_info(3,1u)); // bucket
            fw.unsafe_push(create_level_info(2,1u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {8, 2});
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(7u); // 7

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 8

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can merge content of bucket with overflow queue", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,2u)); // .
            fw.unsafe_push(create_level_info(7,2u)); // .
            fw.unsafe_push(create_level_info(6,4u)); // overflow
            fw.unsafe_push(create_level_info(5,3u)); // bucket
            fw.unsafe_push(create_level_info(4,5u)); // bucket
            fw.unsafe_push(create_level_info(3,4u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.size(), Is().EqualTo(0u));

          // Push something into overflow
          pq.push(pq_test_data {6, 4});
          pq.push(pq_test_data {7, 1});
          pq.push(pq_test_data {8, 2});
          pq.push(pq_test_data {6, 2});
          AssertThat(pq.size(), Is().EqualTo(4u));

          // And into buckets
          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {3, 2});
          AssertThat(pq.size(), Is().EqualTo(6u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
          AssertThat(pq.size(), Is().EqualTo(5u));

          pq.push(pq_test_data {3, 1}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(6u));

          pq.push(pq_test_data {8, 1}); // Overflow
          AssertThat(pq.size(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
          AssertThat(pq.size(), Is().EqualTo(6u));

          pq.push(pq_test_data {4, 1}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(7u));

          pq.push(pq_test_data {5, 2}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(8u));

          pq.push(pq_test_data {4, 2}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(9u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          pq.push(pq_test_data {5, 1}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {4, 3}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {5, 3}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 3}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          pq.push(pq_test_data {8, 4}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {7, 2}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 5

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {6, 3}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 2}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {6, 1}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 3}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {7, 3}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level();// 6

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 1}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {8, 3}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 2}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 3}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          pq.push(pq_test_data {7, 6}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(9u));

          pq.push(pq_test_data {7, 5}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(10u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 4}));
          AssertThat(pq.size(), Is().EqualTo(9u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 7

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 1}));
          AssertThat(pq.size(), Is().EqualTo(8u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 2}));
          AssertThat(pq.size(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 3}));
          AssertThat(pq.size(), Is().EqualTo(6u));

          pq.push(pq_test_data {8, 5}); // Bucket
          AssertThat(pq.size(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 5}));
          AssertThat(pq.size(), Is().EqualTo(6u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 6}));
          AssertThat(pq.size(), Is().EqualTo(5u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 8

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8, 1}));
          AssertThat(pq.size(), Is().EqualTo(4u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8, 2}));
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8, 3}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8, 4}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8, 5}));
          AssertThat(pq.size(), Is().EqualTo(0u));
        });

        it("can deal with exactly as many levels as buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(4,2u)); // bucket
            fw.unsafe_push(create_level_info(3,3u)); // bucket
            fw.unsafe_push(create_level_info(2,4u)); // bucket
            fw.unsafe_push(create_level_info(1,2u)); // bucket
            fw.unsafe_push(create_level_info(0,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {4, 3});
          pq.push(pq_test_data {2, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {3, 2});
          pq.push(pq_test_data {3, 1});
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          pq.push(pq_test_data {4, 1});
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          pq.push(pq_test_data {4, 2});
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 4

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 3}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can deal with fewer levels than buckets", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,2u)); // bucket
            fw.unsafe_push(create_level_info(0,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {2, 1});
          pq.push(pq_test_data {1, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 1

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {2, 2});
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 2

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can forward to stop_label with an empty overflow queue", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,2u)); // .
            fw.unsafe_push(create_level_info(7,4u)); // .
            fw.unsafe_push(create_level_info(6,2u)); // overflow
            fw.unsafe_push(create_level_info(5,6u)); // bucket
            fw.unsafe_push(create_level_info(4,8u)); // bucket
            fw.unsafe_push(create_level_info(3,4u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(0,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {3, 3});
          pq.push(pq_test_data {3, 1});
          AssertThat(pq.size(), Is().EqualTo(2u));

          pq.setup_next_level(2u); // 2

          AssertThat(pq.can_pull(), Is().False());

          pq.push(pq_test_data {3, 2});
          AssertThat(pq.size(), Is().EqualTo(3u));

          pq.setup_next_level(4u); // 3

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 3}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });

        it("can push into buckets after bucket level rewrite", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(17,2u));  // .
            fw.unsafe_push(create_level_info(16,4u));  // .
            fw.unsafe_push(create_level_info(15,8u));  // .
            fw.unsafe_push(create_level_info(14,11u)); // .
            fw.unsafe_push(create_level_info(13,13u)); // .
            fw.unsafe_push(create_level_info(12,17u)); // .
            fw.unsafe_push(create_level_info(11,19u)); // .
            fw.unsafe_push(create_level_info(10,23u)); // .
            fw.unsafe_push(create_level_info(9,19u));  // .
            fw.unsafe_push(create_level_info(8,17u));  // .
            fw.unsafe_push(create_level_info(7,13u));  // .
            fw.unsafe_push(create_level_info(6,11u));  // overflow
            fw.unsafe_push(create_level_info(5,7u));   // bucket
            fw.unsafe_push(create_level_info(4,5u));   // bucket
            fw.unsafe_push(create_level_info(3,3u));   // bucket
            fw.unsafe_push(create_level_info(2,2u));   // bucket
            fw.unsafe_push(create_level_info(1,1u));   // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          AssertThat(pq.size(), Is().EqualTo(0u));

          pq.push(pq_test_data {9, 2});
          pq.push(pq_test_data {10, 3});
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 9

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {9,2}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {10, 1}); // write bucket
          pq.push(pq_test_data {11, 1}); // write bucket
          pq.push(pq_test_data {12, 1}); // write bucket
          pq.push(pq_test_data {13, 1}); // write bucket
          pq.push(pq_test_data {14, 1}); // overflow

          AssertThat(pq.size(), Is().EqualTo(6u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 10

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {10,1}));
          AssertThat(pq.size(), Is().EqualTo(5u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {10,3}));
          AssertThat(pq.size(), Is().EqualTo(4u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 1

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {11,1}));
          AssertThat(pq.size(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 12

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {12,1}));
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 13

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {13,1}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          pq.push(pq_test_data {14, 2}); // write bucket (same as the {14,1} in overflow above)
          AssertThat(pq.size(), Is().EqualTo(2u));

          AssertThat(pq.can_pull(), Is().False());

          pq.setup_next_level(); // 14

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {14,1}));
          AssertThat(pq.size(), Is().EqualTo(1u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {14,2}));
          AssertThat(pq.size(), Is().EqualTo(0u));

          AssertThat(pq.can_pull(), Is().False());
        });
      });

      describe(".top() / .peek()", [&]{
        it("can pull after a peek in bucket", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(6,1u)); // overflow
            fw.unsafe_push(create_level_info(5,2u)); // bucket
            fw.unsafe_push(create_level_info(4,4u)); // bucket
            fw.unsafe_push(create_level_info(3,4u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {3, 42}); // bucket

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          pq.setup_next_level();
          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(3u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.peek(), Is().EqualTo(pq_test_data {3, 42}));
          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 42}));
          AssertThat(pq.can_pull(), Is().False());
        });

        it("can pull after a peek in overflow", [&]() {
          pq_test_file f;

          { // Garbage collect the writer early
            pq_test_writer fw(f);

            fw.unsafe_push(create_level_info(8,1u)); // .
            fw.unsafe_push(create_level_info(7,2u)); // .
            fw.unsafe_push(create_level_info(6,4u)); // overflow
            fw.unsafe_push(create_level_info(5,8u)); // bucket
            fw.unsafe_push(create_level_info(4,4u)); // bucket
            fw.unsafe_push(create_level_info(3,2u)); // bucket
            fw.unsafe_push(create_level_info(2,2u)); // bucket
            fw.unsafe_push(create_level_info(1,1u)); // skipped
          }

          test_priority_queue<pq_test_file, 3> pq({f}, tpie::get_memory_manager().available(), 32);

          pq.push(pq_test_data {7, 3});  // overflow

          AssertThat(pq.can_pull(), Is().False());
          AssertThat(pq.has_current_level(), Is().False());
          AssertThat(pq.has_next_level(), Is().True());
          pq.setup_next_level();
          AssertThat(pq.has_current_level(), Is().True());
          AssertThat(pq.current_level(), Is().EqualTo(7u));

          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.peek(), Is().EqualTo(pq_test_data {7, 3}));
          AssertThat(pq.can_pull(), Is().True());
          AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 3}));
          AssertThat(pq.can_pull(), Is().False());
        });
      });
    });
  });
 });
