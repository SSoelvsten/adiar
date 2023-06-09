#include "../../../test.h"

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>

go_bandit([]() {
  describe("adiar/internal/algorithms/nested_sweeping.h", []() {
    describe("nested_sweeping:: _ ::decorators", []() {
      describe("inner::roots_sorter", []() {
        it("can sort pushed 'requests_t' [cardinality = 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({arc::ptr_t(1,1), arc::ptr_t::NIL()}, {}, {arc::ptr_t(0,0, false)});
          s.push(i1);

          const test_request_t i2({arc::ptr_t(1,0), arc::ptr_t::NIL()}, {}, {arc::ptr_t(0,0, true)});
          s.push(i2);

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target,          Is().EqualTo(i2.target));
          AssertThat(o1.data.source,     Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target,          Is().EqualTo(i1.target));
          AssertThat(o2.data.source,     Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can convert and sort 'reduced_arc' [cardinality == 1]", []() {
          using test_request_t = request_data<1, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          {
            const reduce_arc i1 = arc(arc::ptr_t(0,0), false, arc::ptr_t(1,1));
            s.push(i1);

            const reduce_arc i2 = arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,0));
            s.push(i2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e1({arc::ptr_t(1,0)}, {}, {arc::ptr_t(0,0, true)});
          const test_request_t o1 = s.pull();

          AssertThat(o1.target,      Is().EqualTo(e1.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(e1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e2({arc::ptr_t(1,1)}, {}, {arc::ptr_t(0,0, false)});
          const test_request_t o2 = s.pull();

          AssertThat(o2.target,      Is().EqualTo(e2.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(e2.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can convert and sort 'reduced_arc' [cardinality == 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          {
            const reduce_arc a1 = arc(arc::ptr_t(0,0), false, arc::ptr_t(1,1));
            s.push(a1);

            const reduce_arc a2 = arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,0));
            s.push(a2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e1({arc::ptr_t(1,0), arc::ptr_t::NIL()}, {}, {arc::ptr_t(0,0, true)});
          const test_request_t o1 = s.pull();

          AssertThat(o1.target,      Is().EqualTo(e1.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(e1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e2({arc::ptr_t(1,1), arc::ptr_t::NIL()}, {}, {arc::ptr_t(0,0, false)});
          const test_request_t o2 = s.pull();

          AssertThat(o2.target,      Is().EqualTo(e2.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(e2.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can mix insertion of 'request_t' and 'reduced_arc' [cardinality == 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({arc::ptr_t(3,3), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,0, false)});
          const test_request_t i2({arc::ptr_t(3,1), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,0, true)});
          const test_request_t i3({arc::ptr_t(3,2), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,1, false)});
          const test_request_t i4({arc::ptr_t(3,4), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,1, true)});

          {
            s.push(reduce_arc(arc(i1.data.source, i1.target[0])));
            s.push(i2);
            s.push(i3);
            s.push(reduce_arc(arc(i4.data.source, i4.target[0])));
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target,      Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target,      Is().EqualTo(i3.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i3.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o3 = s.pull();

          AssertThat(o3.target,      Is().EqualTo(i1.target));
          AssertThat(o3.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o4 = s.pull();

          AssertThat(o4.target,      Is().EqualTo(i4.target));
          AssertThat(o4.data.source, Is().EqualTo(flag(i4.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can reset after sorting", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({arc::ptr_t(3,3), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,0, false)});
          const test_request_t i2({arc::ptr_t(3,2), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,0, true)});
          const test_request_t i3({arc::ptr_t(3,2), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,1, false)});
          const test_request_t i4({arc::ptr_t(3,1), arc::ptr_t::NIL()}, {}, {arc::ptr_t(2,1, true)});

          {
            s.push(reduce_arc(arc(i1.data.source, i1.target[0])));
            s.push(i2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target,      Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target,      Is().EqualTo(i1.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().False());

          s.reset();

          {
            s.push(i3);
            s.push(reduce_arc(arc(i4.data.source, i4.target[0])));
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o3 = s.pull();

          AssertThat(o3.target,      Is().EqualTo(i4.target));
          AssertThat(o3.data.source, Is().EqualTo(flag(i4.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o4 = s.pull();

          AssertThat(o4.target,      Is().EqualTo(i3.target));
          AssertThat(o4.data.source, Is().EqualTo(flag(i3.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("updates its 'size'", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.size(), Is().EqualTo(0u));

          s.push(arc(arc::ptr_t(2,0), false, arc::ptr_t(3,0)));
          AssertThat(s.size(), Is().EqualTo(1u));

          s.push({ {arc::ptr_t(3,0)}, {}, {arc::ptr_t(2,0, true)} });
          AssertThat(s.size(), Is().EqualTo(2u));

          s.reset();
          AssertThat(s.size(), Is().EqualTo(0u));

          s.push({ {arc::ptr_t(1,0)}, {}, {arc::ptr_t(0,0, true)} });
          AssertThat(s.size(), Is().EqualTo(1u));
        });
      });

      describe("outer::up__pq_decorator", []() {
        using test_request_t = request_data<2, with_parent, 0, 1>;

        using test_roots_sorter_t =
          nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                               test_request_t,
                                               request_fst_lt<test_request_t>>;

        using test_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode_t::INTERNAL>;

        using test_decorator =
          nested_sweeping::outer::up__pq_decorator<test_pq_t, test_roots_sorter_t>;

        /*
        //       1         ---- x0
        //      / \
        //      F 2        ---- x1
        //       / \
        //       F 3       ---- x2
        //        / \
        //        F 4      ---- x3
        //         / \
        //         F 5     ---- x4
        //          / \
        //          F T
        */

        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1 = ptr_uint64(0,0);
        const ptr_uint64 n2 = ptr_uint64(1,0);
        const ptr_uint64 n3 = ptr_uint64(2,0);
        const ptr_uint64 n4 = ptr_uint64(3,0);
        const ptr_uint64 n5 = ptr_uint64(4,0);

        shared_levelized_file<arc> dag;

        { // Garbage collect writer to free write-lock
          arc_writer aw(dag);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, true, n3 });
          aw.push_internal({ n3, true, n4 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
        }

        it("forwards internal to PQ when below threshold", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(n4, false, n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal to PQ when at threshold", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(n3, false, n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal to Sorter when above threshold", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(n2, false, n5));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("forwards terminal to PQ even if at threshold", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards terminal to PQ even if above threshold", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(n2, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards correctly with multiple requests", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal below threshold
          d.push(arc(n4, false, n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // terminal at threshold
          d.push(arc(n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal at threshold
          d.push(arc(n3, true, n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal above threshold
          d.push(arc(n2, false, n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(1u));

          // terminal above threshold
          d.push(arc(n1, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(4u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("provides combined size (w and w/o terminals)", [&]() {
          test_pq_t pq({dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          AssertThat(d.empty(), Is().True());

          // internal below threshold
          d.push(arc(n4, false, n5));
          // terminal at threshold
          d.push(arc(n3, false, terminal_F));
          // internal at threshold
          d.push(arc(n3, true, n5));
          // internal above threshold
          d.push(arc(n2, false, n5));
          // terminal above threshold
          d.push(arc(n1, false, terminal_T));

          AssertThat(d.empty(), Is().False());
          AssertThat(d.size(), Is().EqualTo(5u));
          AssertThat(d.terminals(false), Is().EqualTo(1u));
          AssertThat(d.terminals(true), Is().EqualTo(1u));
          AssertThat(d.size_without_terminals(), Is().EqualTo(3u));
        });
      });

      describe("inner::down__pq_decorator", []() {
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const uid_uint64 n1 = ptr_uint64(3,0);
        const uid_uint64 n2 = ptr_uint64(3,1);
        const uid_uint64 n3 = ptr_uint64(4,0);
        const uid_uint64 n4 = ptr_uint64(4,1);
        const uid_uint64 n5 = ptr_uint64(5,0);
        const uid_uint64 n6 = ptr_uint64(5,1);

        shared_levelized_file<node> dag;

        /*
        //        1   2        ---- x3
        //       / \ / \
        //       3  4   \      ---- x4
        //      / \/ \   \
        //      F T  5   6     ---- x5
        //          / \ / \
        //          T F F T
        */

        { // Garbage collect writer to free write-lock
          node_writer nw(dag);

          nw << node(n6, terminal_F, terminal_T)
             << node(n5, terminal_T, terminal_F)
             << node(n4, terminal_T, n5)
             << node(n3, terminal_F, terminal_T)
             << node(n2, n4,         n6)
             << node(n1, n3,         n4)
            ;
        }

        using test_request_t = request_data<1, with_parent, 0, 1>;

        using test_roots_sorter_t =
          nested_sweeping::inner::roots_sorter<memory_mode_t::INTERNAL,
                                               test_request_t,
                                               request_fst_lt<test_request_t>>;

        const test_request_t root1({n1}, {}, {ptr_uint64(1,0, false)});
        const test_request_t root2({n2}, {}, {ptr_uint64(1,0, true)});
        const test_request_t root3({n6}, {}, {ptr_uint64(1,1, false)});

        /*
        //
        //       n1p/n2p  n6p
        //        /   \    |
        //        |   |    |   ---- x2 (nested sweep level)
        //        |   |    |
        //        1   2    |   ---- x3
        //                 |
        //                 |   ---- x4
        //                 |
        //                 6   ---- x5
        */

        test_roots_sorter_t sorter(1024, 16);

        before_each([&]() {
          sorter.reset();

          sorter.push(root1);
          sorter.push(root2);
          sorter.push(root3);

          sorter.sort();
        });

        stats_t::levelized_priority_queue_t lpq_stats;

        using test_pq_t = levelized_node_priority_queue<test_request_t,
                                                        request_fst_lt<test_request_t>,
                                                        1, memory_mode_t::INTERNAL,
                                                        1,
                                                        0 /* <-- this is important for nested sweeping */>;

        using test_decorator =
          nested_sweeping::inner::down__pq_decorator<test_pq_t, test_roots_sorter_t>;

        it("initializes with levels of PQ but with size of Sorter", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());

          AssertThat(dec.has_next_level(), Is().True());
          AssertThat(dec.next_level(), Is().EqualTo(3u));

          AssertThat(dec.empty(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("merges size of PQ and Sorter", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          const test_request_t root4({n2}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          AssertThat(dec.size(), Is().EqualTo(4u));
        });

        it("setup_next_level() forward to first Sorter level", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(3u));
        });

        it("can pull requests from Sorter", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target,      Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull merge of PQ and Sorter [tie on 'n2']", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n2}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target,      Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.pull();
          AssertThat(r3.target,      Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull merge of PQ and Sorter [tie on 'n1']", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n1}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target,      Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.pull();
          AssertThat(r3.target,      Is().EqualTo(root2.target));
          AssertThat(r3.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull when Sorter is empty", [&]() {
          // empty out sorter
          AssertThat(sorter.size(), Is().EqualTo(3u));
          sorter.pull(); sorter.pull(); sorter.pull();
          AssertThat(sorter.size(), Is().EqualTo(0u));
          AssertThat(sorter.empty(), Is().True());

          // Start test
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n1}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r = dec.pull();
          AssertThat(r.target,      Is().EqualTo(root4.target));
          AssertThat(r.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(0u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can top the same request from Sorter multiple times", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top the sorter
          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.top();
          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top it again
          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.top();
          AssertThat(r2.target,      Is().EqualTo(root1.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("can top the same request from PQ multiple times", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          const test_request_t root4({n1}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          // Skip over the one request from the sorter (but check it truly is that one)
          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top the priority queue once
          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.top();
          AssertThat(r2.target,      Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top it again
          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.top();
          AssertThat(r3.target,      Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("can top and pop requests from Sorter", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target,      Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can top and pop merge of PQ and Sorter [tie on 'n2']", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n2}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target,      Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.top();
          dec.pop();

          AssertThat(r3.target,      Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can top and pop merge of PQ and Sorter [tie on 'n1']", [&]() {
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n1}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target,      Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target,      Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.top();
          dec.pop();

          AssertThat(r3.target,      Is().EqualTo(root2.target));
          AssertThat(r3.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can top and pop when Sorter is empty", [&]() {
          // empty out sorter
          AssertThat(sorter.size(), Is().EqualTo(3u));
          sorter.pull(); sorter.pull(); sorter.pull();
          AssertThat(sorter.size(), Is().EqualTo(0u));
          AssertThat(sorter.empty(), Is().True());

          // Start test
          test_pq_t pq({dag}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({n1}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r = dec.top();
          dec.pop();

          AssertThat(r.target,      Is().EqualTo(root4.target));
          AssertThat(r.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(0u));

          AssertThat(dec.can_pull(), Is().False());
        });
      });

      describe("inner::up__pq_decorator", []() {
        // Outer PQ
        using outer_test_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode_t::INTERNAL>;
        /*
        //       1         ---- x0
        //      / \
        //      F 2        ---- x1
        //       / \
        //       F 3       ---- x2
        //        / \
        //        F 4      ---- x3
        //         / \
        //         F 5     ---- x4
        //          / \
        //          F T
        */

        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1 = ptr_uint64(0,0);
        const ptr_uint64 n2 = ptr_uint64(1,0);
        const ptr_uint64 n3 = ptr_uint64(2,0);
        const ptr_uint64 n4 = ptr_uint64(3,0);
        const ptr_uint64 n5 = ptr_uint64(4,0);

        shared_levelized_file<arc> outer_dag;

        { // Garbage collect writer to free write-lock
          arc_writer aw(outer_dag);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, true, n3 });
          aw.push_internal({ n3, true, n4 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
        }

        // Inner PQ
        using inner_test_pq_t = nested_sweeping::inner::up__pq_t<1, memory_mode_t::INTERNAL>;
        /*
        //
        //         3       ---- x2
        //        / \
        //        F 4      ---- x3
        //         / \
        //         F 5     ---- x4
        //          / \
        //          F T
        */

        shared_levelized_file<arc> inner_dag;

        { // Garbage collect writer to free write-lock
          arc_writer aw(inner_dag);

          aw.push_internal({ n3, true, n4 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
        }

        using test_decorator =
          nested_sweeping::inner::up__pq_decorator<inner_test_pq_t, outer_test_pq_t>;

        const size_t pq_mem = memory_available()/2;

        it("forwards unflagged terminal to Inner PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(n4, false, terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards unflagged internal node to Inner PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(n4, true, n5));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards flagged terminal to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(with_out_idx(n4, false)), terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("forwards flagged internal node to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(with_out_idx(n4, true)), n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("unflags source when handing over to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);
          dec.push(arc(flag(with_out_idx(n4, true)), n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          outer_pq.setup_next_level();

          AssertThat(outer_pq.can_pull(), Is().True());
          AssertThat(outer_pq.pull(), Is().EqualTo(arc(n4, true, n5)));
        });

        it("sets up next level only based on Inner PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(with_out_idx(n3, false),      terminal_F)); // inner (x2)
          dec.push(arc(flag(with_out_idx(n4, true)), n5));         // outer (x3)
          dec.push(arc(with_out_idx(n3, true),       n5));         // inner (x2)

          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(2u));
        });

        it("counts arcs from both Inner and Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(with_out_idx(n4, false),      terminal_F)); // inner (x3)
          dec.push(arc(flag(with_out_idx(n4, true)), n5));         // outer (x3)
          dec.push(arc(with_out_idx(n4, true),       n5));         // inner (x3)

          AssertThat(outer_pq.size(), Is().EqualTo(1u));

          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.top(),  Is().EqualTo(arc(n4, true, n5)));
          AssertThat(dec.pull(), Is().EqualTo(arc(n4, true, n5)));

          AssertThat(dec.size(), Is().EqualTo(2u));
          AssertThat(dec.empty(), Is().False());

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.top(),  Is().EqualTo(arc(n4, false, terminal_F)));
          AssertThat(dec.pull(), Is().EqualTo(arc(n4, false, terminal_F)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.can_pull(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());

          dec.push(arc(flag(with_out_idx(n3, false)), terminal_F)); // outer
          dec.push(arc(with_out_idx(n3, true),        n5));         // inner

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());
        });

        it("empty() is independent of Outer PQ size", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(flag(with_out_idx(n4, true)), n5));         // outer (x3)

          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());
        });
      });
    });

    describe("nested_sweeping:: _ ::sweeps", []() {
      describe("inner::down(...)", []() {
        // TODO: test with mock GC policy?
      });

      describe("inner::up(...)", []() {
        using inner_up_sweep = nested_sweeping::inner::up__policy_t<bdd_policy>;
        using outer_pq_t     = nested_sweeping::outer::up__pq_t<1, memory_mode_t::INTERNAL>;

        /*
        //       1         ---- x0
        //      / \
        //      F 2        ---- x1
        //       / \
        //       F 3       ---- x2
        //        / \
        //        F 4      ---- x3
        //         / \
        //         F 5     ---- x4
        //          / \
        //          F T
        */

        shared_levelized_file<arc> outer_dag;

        {
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1 = ptr_uint64(0,0);
          const ptr_uint64 n2 = ptr_uint64(1,0);
          const ptr_uint64 n3 = ptr_uint64(2,0);
          const ptr_uint64 n4 = ptr_uint64(3,0);
          const ptr_uint64 n5 = ptr_uint64(4,0);

          arc_writer aw(outer_dag);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, true, n3 });
          aw.push_internal({ n3, true, n4 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
        }

        it("reduces forest and pushes roots back out", [&outer_dag]() {
          /* input
          //       1     _2_           ---- x1
          //  -   - \ - / - \  -   -   -    -
          //        3   4   5          ---- x2
          //       / \ / \ / \
          //       T | T | F T
          //         \_ _/
          //           6               ---- x3
          //          / \
          //          T F
           */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(1,1);
          const ptr_uint64 n3(2,0);
          const ptr_uint64 n4(2,1);
          const ptr_uint64 n5(2,2);
          const ptr_uint64 n6(3,0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in);

            aw.push_internal({ flag(with_out_idx(n1, true)), n3 });
            aw.push_internal({ flag(with_out_idx(n2, false)), n4 });
            aw.push_internal({ flag(with_out_idx(n2, true)),  n5 });
            aw.push_internal({ with_out_idx(n3, true), n6 });
            aw.push_internal({ with_out_idx(n4, true), n6 });

            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n5, true,  terminal_T });
            aw.push_terminal({ n6, false, terminal_T });
            aw.push_terminal({ n6, true,  terminal_F });

            // NOTE: 'level_info(1,2u)' is not a processable part of the forest;
            aw.push(level_info(2,3u));
            aw.push(level_info(3,1u));

            in->max_1level_cut = 2;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({outer_dag}, available_memory / 2, 8);

          /* output
          //      1   2           ---- x1
          //  -   -\ /-\  -   -   -    -
          //        3   5         ---- x2
          //       / \ / \
          //       T 6 F T        ---- x3
          //        / \
          //        T F
          */
          const auto global_1level_cut =
            nested_sweeping::inner::up<inner_up_sweep>(in, out_pq, out_writer, available_memory / 2);

          // Check 'global_1level_cut'
          AssertThat(global_1level_cut, Is().EqualTo(cuts_t({ 0u, 0u, 0u, 0u })));

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));

          // Over-approximation, since T-terminal from level (2) is removed
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));

          // Check node and meta files are correct
          out_writer.detach();

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_F,
                                                         terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID-1,
                                                         terminal_T,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,2u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(),  Is().EqualTo(3u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n2, true, ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n2, false, ptr_uint64(2, ptr_uint64::MAX_ID-1))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true,  ptr_uint64(2, ptr_uint64::MAX_ID-1))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("includes outer_pq terminals in cut size", [&outer_dag]() {
          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       F T
           */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true, terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in->max_1level_cut = 0;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({outer_dag}, available_memory / 2, 8);
          out_pq.push({ n1, false, terminal_F });

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       F T
          */
          const auto global_1level_cut =
            nested_sweeping::inner::up<inner_up_sweep>(in, out_pq, out_writer, available_memory / 2);

          // Check 'global_1level_cut'
          AssertThat(global_1level_cut, Is().EqualTo(cuts_t({ 0u, 0u, 0u, 0u })));

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL],       Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE],  Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL],            Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));

          // Check node and meta files are correct
          out_writer.detach();

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(),  Is().EqualTo(2u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, false, terminal_F)));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("returns globally counted arcs for 1-level cut", [&outer_dag]() {
          /* input
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       F 3          ---- x4
          //        / \
          //        T T
          */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);
          const ptr_uint64 n3(4,0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n3, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));
            aw.push(level_info(4,1u));

            in->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({outer_dag}, available_memory / 2, 8);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       F T <-- T counted in global cut
          */
          const auto global_1level_cut =
            nested_sweeping::inner::up<inner_up_sweep>(in, out_pq, out_writer, available_memory / 2);

          // Check 'global_1level_cut'
          AssertThat(global_1level_cut, Is().EqualTo(cuts_t({ 0u, 0u, 1u, 1u })));

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL],       Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE],  Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::ALL],            Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));

          // Check node and meta files are correct
          out_writer.detach();

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(),  Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("leaves file empty if everything collapsed to a terminal", [&outer_dag]() {
          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       T T
          */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n2, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in->max_1level_cut = 0;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({outer_dag}, available_memory / 2, 8);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        T <-- in 'out_pq', not in 'out'
          */
          const auto global_1level_cut =
            nested_sweeping::inner::up<inner_up_sweep>(in, out_pq, out_writer, available_memory / 2);

          // Check 'global_1level_cut'
          AssertThat(global_1level_cut, Is().EqualTo(cuts_t({ 0u, 0u, 0u, 0u })));

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL],       Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE],  Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::ALL],            Is().EqualTo(0u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(0u));

          // Check node and meta files are correct
          out_writer.detach();

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(),  Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, flag(terminal_T))));

          AssertThat(out_pq.can_pull(), Is().False());
        });
      });
    });
  });
 });
