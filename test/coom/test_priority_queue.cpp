#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <coom/priority_queue.cpp>

using namespace coom;

struct pq_test_data {
  label_t label;
  uint64_t nonce;
};

bool operator== (const pq_test_data &a, const pq_test_data &b)
{
  return a.label == b.label && a.nonce == b.nonce;
}

struct pq_test_label_ext {
  label_t label_of(const pq_test_data &d) const
  {
    return d.label;
  }
};

struct pq_test_comp {
  bool operator()(const pq_test_data &a, const pq_test_data &b)
  {
    return a.label < b.label || (a.label == b.label && a.nonce < b.nonce);
  }
};

go_bandit([]() {
  describe("COOM: Priority Queue", []() {

    describe("Label Manager", [&]() {
      it("can pull from one meta stream", [&]() {
        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq_label_mgr<std::less<>, 1> mgr;

        AssertThat(mgr.hook_meta_stream(meta_stream), Is().True());

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(1u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(2u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(3u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(4u));

        AssertThat(mgr.can_pull(), Is().False());
      });

      it("can peek from one meta streams", [&]() {
        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq_label_mgr<std::less<>, 1> mgr;

        AssertThat(mgr.hook_meta_stream(meta_stream), Is().True());

        AssertThat(mgr.pull(), Is().EqualTo(1u));

        AssertThat(mgr.peek(), Is().EqualTo(2u));
        AssertThat(mgr.pull(), Is().EqualTo(2u));

        AssertThat(mgr.peek(), Is().EqualTo(3u));
        AssertThat(mgr.pull(), Is().EqualTo(3u));

        AssertThat(mgr.pull(), Is().EqualTo(4u));
      });


      it("can pull from merge of two meta streams, where one is empty", [&]() {
         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write(meta_t {1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         pq_label_mgr<std::less<>, 2> mgr;

         AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
         AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

         AssertThat(mgr.can_pull(), Is().True());
         AssertThat(mgr.pull(), Is().EqualTo(1u));

         AssertThat(mgr.can_pull(), Is().False());
      });

      it("can pull from merge of two meta streams [1]", [&]() {
        tpie::file_stream<meta_t> meta_stream_1;
        meta_stream_1.open();

        meta_stream_1.write(meta_t {4});
        meta_stream_1.write(meta_t {2});
        meta_stream_1.write(meta_t {1});

        tpie::file_stream<meta_t> meta_stream_2;
        meta_stream_2.open();

        meta_stream_2.write(meta_t {4});
        meta_stream_2.write(meta_t {3});

        pq_label_mgr<std::less<>, 2> mgr;

        AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
        AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(1u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(2u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(3u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(4u));

        AssertThat(mgr.can_pull(), Is().False());
      });

      it("can pull from merge of two meta streams [2] (std::less)", [&]() {
        tpie::file_stream<meta_t> meta_stream_1;
        meta_stream_1.open();

        meta_stream_1.write(meta_t {2});

        tpie::file_stream<meta_t> meta_stream_2;
        meta_stream_2.open();

        meta_stream_2.write(meta_t {1});

        pq_label_mgr<std::less<>, 2> mgr;

        AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
        AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(1u));

        AssertThat(mgr.can_pull(), Is().True());
        AssertThat(mgr.pull(), Is().EqualTo(2u));

        AssertThat(mgr.can_pull(), Is().False());
      });

      it("can pull from merge of two meta streams [2] (std::greater)", [&]() {
         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write(meta_t {2});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         meta_stream_2.write(meta_t {1});

         pq_label_mgr<std::greater<label_t>, 2> mgr;

         AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
         AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

         AssertThat(mgr.can_pull(), Is().True());
         AssertThat(mgr.pull(), Is().EqualTo(2u));

         AssertThat(mgr.can_pull(), Is().True());
         AssertThat(mgr.pull(), Is().EqualTo(1u));

         AssertThat(mgr.can_pull(), Is().False());
      });

      it("can pull from merge of two meta streams, where one is empty", [&]() {
         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write(meta_t {1});
         meta_stream_1.write(meta_t {2});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         pq_label_mgr<std::greater<label_t>, 2> mgr;

         AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
         AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

         AssertThat(mgr.can_pull(), Is().True());
         AssertThat(mgr.pull(), Is().EqualTo(2u));

         AssertThat(mgr.can_pull(), Is().True());
         AssertThat(mgr.pull(), Is().EqualTo(1u));

         AssertThat(mgr.can_pull(), Is().False());
      });

      it("can peek merge of two meta stream", [&]() {
        tpie::file_stream<meta_t> meta_stream_1;
        meta_stream_1.open();

        meta_stream_1.write(meta_t {4});
        meta_stream_1.write(meta_t {2});

        tpie::file_stream<meta_t> meta_stream_2;
        meta_stream_2.open();

        meta_stream_2.write(meta_t {4});
        meta_stream_2.write(meta_t {3});
        meta_stream_2.write(meta_t {1});

        pq_label_mgr<std::less<>, 2> mgr;

        AssertThat(mgr.hook_meta_stream(meta_stream_1), Is().False());
        AssertThat(mgr.hook_meta_stream(meta_stream_2), Is().True());

        AssertThat(mgr.peek(), Is().EqualTo(1u));
        AssertThat(mgr.pull(), Is().EqualTo(1u));
        AssertThat(mgr.peek(), Is().EqualTo(2u));
        AssertThat(mgr.pull(), Is().EqualTo(2u));
        AssertThat(mgr.peek(), Is().EqualTo(3u));
        AssertThat(mgr.pull(), Is().EqualTo(3u));
        AssertThat(mgr.peek(), Is().EqualTo(4u));
        AssertThat(mgr.pull(), Is().EqualTo(4u));
      });
    });

    describe("with 0 Buckets", [&]() {
      it("can set up priority queue", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);
      });

      it("can set up priority queue with empty meta stream", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         pq.hook_meta_stream(meta_stream);

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can push to and pull from immediate next layer", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());

        pq.push(pq_test_data {2, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));
        pq.push(pq_test_data {2, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {3, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {3, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can skip unpushed layers", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can skip unpushed layers up until the given stop_label", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer(3u);
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());
      });

      it("can sort elements given out of layer order", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());

        pq.push(pq_test_data {4, 2});
        AssertThat(pq.size(), Is().EqualTo(1u));
        pq.push(pq_test_data {3, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {2, 1});
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {3, 2});
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
      });

      it("can pull after a peek", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 0> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);

         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().False());
         AssertThat(pq.size(), Is().EqualTo(0u));

         pq.push(pq_test_data {2, 2});
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(2u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.peek(), Is().EqualTo(pq_test_data {2, 2}));
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));
         AssertThat(pq.size(), Is().EqualTo(0u));
         AssertThat(pq.can_pull(), Is().False());
      });
    });

    describe("with 1 Bucket", [&]() {
      it("can set up priority queue with more layers than buckets", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {5});
         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);
      });

      it("can set up priority queue with fewer layers than buckets", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);
      });

      it("can set up priority queue with empty meta stream", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         pq.hook_meta_stream(meta_stream);

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can set up priority queue for two meta streams, where one is empty", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 2, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write({1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         pq.hook_meta_stream(meta_stream_1);
         pq.hook_meta_stream(meta_stream_2);

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can set up priority queue for two meta streams", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 2, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write({1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         meta_stream_2.write({2});

         pq.hook_meta_stream(meta_stream_1);
         pq.hook_meta_stream(meta_stream_2);

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can push into and pull from bucket", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {2, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {2, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {3, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {3, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can push into overflow queue [1]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {3, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
      });

      it("can push into overflow queue [2]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
      });

      it("can skip unpushed layers up to stop_label [1]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer(3u);
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can skip unpushed layers up to stop_label [2]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {5});
        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {5, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer(4u);
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(5u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 1}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can merge content of bucket with overflow queue", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {4, 2}); // overflow
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {3, 1}); // overflow
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.push(pq_test_data {2, 1}); // bucket
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {3, 2}); // bucket
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.push(pq_test_data {4, 1}); // bucket
        AssertThat(pq.size(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());
      });

      it("can pull after a peek in bucket", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {6});
         meta_stream.write(meta_t {5});
         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);

         AssertThat(pq.size(), Is().EqualTo(0u));

         pq.push(pq_test_data {2, 42}); // bucket
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(2u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.peek(), Is().EqualTo(pq_test_data {2, 42}));
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 42}));
         AssertThat(pq.size(), Is().EqualTo(0u));

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can pull after a peek in overflow", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 1> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {6});
         meta_stream.write(meta_t {5});
         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);

         pq.push(pq_test_data {5, 3});  // overflow
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(5u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.peek(), Is().EqualTo(pq_test_data {5, 3}));
         AssertThat(pq.size(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {5, 3}));
         AssertThat(pq.size(), Is().EqualTo(0u));

         AssertThat(pq.can_pull(), Is().False());
      });
    });

    describe("with 4 Bucket", [&]() {
      it("can set up priority queue with more layers than buckets", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8});
        meta_stream.write(meta_t {7});
        meta_stream.write(meta_t {6});
        meta_stream.write(meta_t {4});
        meta_stream.write(meta_t {3});
        meta_stream.write(meta_t {2});
        meta_stream.write(meta_t {1});

        pq.hook_meta_stream(meta_stream);
      });

      it("can set up priority queue with fewer layers than buckets [1]", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);
       });

      it("can set up priority queue with fewer layers than buckets [2]", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);
       });

      it("can set up priority queue with empty meta stream", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         pq.hook_meta_stream(meta_stream);

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.has_next_layer(), Is().False());
      });


      it("can set up priority queue for two meta streams, where one is empty [1]", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 2, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write({1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         pq.hook_meta_stream(meta_stream_1);
         pq.hook_meta_stream(meta_stream_2);

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can set up priority queue for two meta streams, where one is empty [2]", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 2, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write({2});
         meta_stream_1.write({1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         pq.hook_meta_stream(meta_stream_1);
         pq.hook_meta_stream(meta_stream_2);

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can set up priority queue for two meta streams", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 2, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream_1;
         meta_stream_1.open();

         meta_stream_1.write({1});

         tpie::file_stream<meta_t> meta_stream_2;
         meta_stream_2.open();

         meta_stream_2.write({2});

         pq.hook_meta_stream(meta_stream_1);
         pq.hook_meta_stream(meta_stream_2);

         AssertThat(pq.can_pull(), Is().False());
      });

      it("can push into and pull from buckets", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {2, 1});
        AssertThat(pq.size(), Is().EqualTo(1u));
        pq.push(pq_test_data {2, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));
        pq.push(pq_test_data {3, 42});
        AssertThat(pq.size(), Is().EqualTo(3u));
        pq.push(pq_test_data {3, 21});
        AssertThat(pq.size(), Is().EqualTo(4u));
        pq.push(pq_test_data {5, 3});
        AssertThat(pq.size(), Is().EqualTo(5u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.size(), Is().EqualTo(5u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(4u));

        pq.push(pq_test_data {3, 2});
        AssertThat(pq.size(), Is().EqualTo(5u));
        pq.push(pq_test_data {6, 2});
        AssertThat(pq.size(), Is().EqualTo(6u));
        pq.push(pq_test_data {4, 2});
        AssertThat(pq.size(), Is().EqualTo(7u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));
        AssertThat(pq.size(), Is().EqualTo(6u));
        AssertThat(pq.can_pull(), Is().False());

        pq.push(pq_test_data {3, 3});
        AssertThat(pq.size(), Is().EqualTo(7u));
        pq.push(pq_test_data {5, 2});
        AssertThat(pq.size(), Is().EqualTo(8u));
        pq.push(pq_test_data {3, 1});
        AssertThat(pq.size(), Is().EqualTo(9u));

        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));
        AssertThat(pq.size(), Is().EqualTo(8u));

        pq.push(pq_test_data {5, 1});
        AssertThat(pq.size(), Is().EqualTo(9u));
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
        AssertThat(pq.size(), Is().EqualTo(10u));
        pq.push(pq_test_data {7, 3});
        AssertThat(pq.size(), Is().EqualTo(11u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 42}));
        AssertThat(pq.size(), Is().EqualTo(10u));

        pq.push(pq_test_data {7, 5});
        AssertThat(pq.size(), Is().EqualTo(11u));
        pq.push(pq_test_data {4, 1});
        AssertThat(pq.size(), Is().EqualTo(12u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

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
        AssertThat(pq.size(), Is().EqualTo(10u));
        pq.push(pq_test_data {7, 2});
        AssertThat(pq.size(), Is().EqualTo(11u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(4u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(5u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(5u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(6u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(6u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(7u));

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
        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can push into overflow queue [1]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8}); // overflow
        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {6, 4});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {8, 2});
        AssertThat(pq.size(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(6u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {6, 4}));
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(6u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(8u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can push into overflow queue [2]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8}); // overflow
        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {8, 2});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(8u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can skip unpushed layers until stop_label [1]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8}); // overflow
        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {8, 2});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer(3u);
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(8u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can skip nonempty layers until stop_label [2]", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8}); // overflow
        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));

        pq.push(pq_test_data {8, 2});
        AssertThat(pq.size(), Is().EqualTo(1u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer(7u);
        AssertThat(pq.current_layer(), Is().EqualTo(7u));

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(7u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(8u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {8,2}));
        AssertThat(pq.size(), Is().EqualTo(0u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can merge content of bucket with overflow queue", [&]() {
        priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

        tpie::file_stream<meta_t> meta_stream;
        meta_stream.open();

        meta_stream.write(meta_t {8}); // overflow
        meta_stream.write(meta_t {7}); // overflow
        meta_stream.write(meta_t {6}); // overflow
        meta_stream.write(meta_t {5}); // write bucket
        meta_stream.write(meta_t {4}); // write bucket
        meta_stream.write(meta_t {3}); // write bucket
        meta_stream.write(meta_t {2}); // write bucket
        meta_stream.write(meta_t {1}); // read bucket

        pq.hook_meta_stream(meta_stream);

        AssertThat(pq.can_pull(), Is().False());
        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.size(), Is().EqualTo(0u));

        // Push something into overflow
        pq.push(pq_test_data {6, 4});
        AssertThat(pq.size(), Is().EqualTo(1u));

        pq.push(pq_test_data {7, 1});
        AssertThat(pq.size(), Is().EqualTo(2u));

        pq.push(pq_test_data {8, 2});
        AssertThat(pq.size(), Is().EqualTo(3u));

        pq.push(pq_test_data {6, 2});
        AssertThat(pq.size(), Is().EqualTo(4u));

        // And into buckets
        pq.push(pq_test_data {2, 1});
        AssertThat(pq.size(), Is().EqualTo(5u));

        pq.push(pq_test_data {3, 2});
        AssertThat(pq.size(), Is().EqualTo(6u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(1u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(2u));

        AssertThat(pq.can_pull(), Is().True());
        AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));
        AssertThat(pq.size(), Is().EqualTo(5u));

        pq.push(pq_test_data {3, 1}); // Bucket
        AssertThat(pq.size(), Is().EqualTo(6u));

        pq.push(pq_test_data {8, 1}); // Overflow
        AssertThat(pq.size(), Is().EqualTo(7u));

        AssertThat(pq.can_pull(), Is().False());

        AssertThat(pq.current_layer(), Is().EqualTo(2u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(3u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(3u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(4u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(4u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(5u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(5u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(6u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(6u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(7u));

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

        AssertThat(pq.current_layer(), Is().EqualTo(7u));
        AssertThat(pq.has_next_layer(), Is().True());
        pq.setup_next_layer();
        AssertThat(pq.current_layer(), Is().EqualTo(8u));

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

      it("can pull after a peek in bucket", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {6});
         meta_stream.write(meta_t {5});
         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);

         pq.push(pq_test_data {3, 42}); // bucket

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(3u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.peek(), Is().EqualTo(pq_test_data {3, 42}));
         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 42}));
         AssertThat(pq.can_pull(), Is().False());
      });

      it("can pull after a peek in overflow", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {8});
         meta_stream.write(meta_t {7});
         meta_stream.write(meta_t {6});
         meta_stream.write(meta_t {5});
         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});

         pq.hook_meta_stream(meta_stream);

         pq.push(pq_test_data {7, 3});  // overflow

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(7u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.peek(), Is().EqualTo(pq_test_data {7, 3}));
         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {7, 3}));
         AssertThat(pq.can_pull(), Is().False());
      });

      it("can deal with exactly as many layers as buckets", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {4});
         meta_stream.write(meta_t {3});
         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});
         meta_stream.write(meta_t {0});

         pq.hook_meta_stream(meta_stream);

         pq.push(pq_test_data {4, 3});
         pq.push(pq_test_data {2, 1});

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(0u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(2u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

         pq.push(pq_test_data {3, 2});
         pq.push(pq_test_data {3, 1});

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(2u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(3u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 1}));

         pq.push(pq_test_data {4, 1});

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {3, 2}));

         pq.push(pq_test_data {4, 2});

         AssertThat(pq.can_pull(), Is().False());

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(3u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(4u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 1}));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 2}));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {4, 3}));

         AssertThat(pq.has_next_layer(), Is().False());
      });

      it("can deal with fewer layers as buckets", [&]() {
         priority_queue<pq_test_data, pq_test_label_ext, pq_test_comp, 1, std::less<>, 4> pq;

         tpie::file_stream<meta_t> meta_stream;
         meta_stream.open();

         meta_stream.write(meta_t {2});
         meta_stream.write(meta_t {1});
         meta_stream.write(meta_t {0});

         pq.hook_meta_stream(meta_stream);

         pq.push(pq_test_data {2, 1});
         pq.push(pq_test_data {1, 1});

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(0u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(1u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {1, 1}));

         pq.push(pq_test_data {2, 2});

         AssertThat(pq.can_pull(), Is().False());
         AssertThat(pq.current_layer(), Is().EqualTo(1u));
         AssertThat(pq.has_next_layer(), Is().True());
         pq.setup_next_layer();
         AssertThat(pq.current_layer(), Is().EqualTo(2u));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 1}));

         AssertThat(pq.can_pull(), Is().True());
         AssertThat(pq.pull(), Is().EqualTo(pq_test_data {2, 2}));

         AssertThat(pq.can_pull(), Is().False());

         AssertThat(pq.has_next_layer(), Is().False());
      });
    });
  });
 });
