#include "../../../test.h"

#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/statistics.h>

////////////////////////////////////////////////////////////////////////////////
/// Policy that provides the most simplistic top-down sweep to test the Inner
/// Down Sweep in isolation. The algorithm within is a simple 'mark and sweep'
/// GC that somewhat negates the entire DAG (but kills the nodes on the nesting
/// level).
///
/// \tparam only_gc when set to true, then each request created will only have a
///                 single non-nil member, i.e. it is a request preserving a
///                 subtree but (presumably) not changing it.
////////////////////////////////////////////////////////////////////////////////
template<bool only_gc = false>
class test_not_sweep : public bdd_policy
{
private:
  const size_t _nesting_modulo;

public:
  // HACK: The request is for a 'tuple' to allow us to 'lie' to the Nested
  //       Sweeping framework that we are not "just" preserving some node
  //       underneath or inversely that we are not "changing" subgraph.
  using request_t = request_data<2, with_parent, 0, 1>;

  using request_pred_t = request_data_fst_lt<request_t>;

  template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
  using pq_t = levelized_node_priority_queue<request_t, request_pred_t,
                                             LOOK_AHEAD, mem_mode>;

public:
  test_not_sweep(const size_t nm)
    : _nesting_modulo(nm)
  { }

public:
  //////////////////////////////////////////////////////////////////////////////
  static size_t stream_memory()
  { return node_stream<>::memory_usage() + arc_writer::memory_usage(); }

  //////////////////////////////////////////////////////////////////////////////
  static size_t pq_memory(const size_t inner_memory)
  { return inner_memory; }

  //////////////////////////////////////////////////////////////////////////////
  static size_t pq_bound(const shared_levelized_file<node> &/*outer_file*/,
                         const size_t /*outer_roots*/)
  { return 8; }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the PQ access mode case.
  //////////////////////////////////////////////////////////////////////////////
  template<typename inner_pq_t>
  shared_levelized_file<arc>
  sweep_pq(const shared_levelized_file<node> &outer_file,
          inner_pq_t &inner_pq,
          const size_t /*inner_remaining_memory == 0*/)
  {
    node_stream<> ns(outer_file);

    shared_levelized_file<arc> af;
    af->max_1level_cut = 1;

    arc_writer aw(af);

    while (!inner_pq.empty()) {
      inner_pq.setup_next_level();

      const node::label_t level_label = inner_pq.current_level();
      node::id_t level_size = 0u;

      while (!inner_pq.empty_level()) {
        // Give node a new name
        const node::uid_t u(level_label, level_size++);

        // Get target of next request
        adiar_debug(!inner_pq.top().target.fst().is_flagged(),
                    "Double checking decorator indeed hides taint");

        const node::ptr_t next = inner_pq.top().target.fst();

        // Seek node in stream and forward its out-going edges
        const node n = ns.seek(next);
        forward_arc<false>(inner_pq, aw, u, n.low());
        forward_arc<true >(inner_pq, aw, u, n.high());

        // Output all in-going edges that match the target
        while (inner_pq.can_pull() && inner_pq.top().target.fst() == next) {
          request_t req = inner_pq.pull();
          if (req.data.source.is_nil()) {
            continue;
          }

          aw.push_internal({ req.data.source, u });
        }
      }

      aw.push(level_info(level_label, level_size));
      af->max_1level_cut = std::max(af->max_1level_cut, inner_pq.size());
    }

    return af;
  }

private:
  //////////////////////////////////////////////////////////////////////////////
  template<bool is_high, typename inner_pq_t>
  void forward_arc(inner_pq_t &inner_pq, arc_writer &aw,
                   const node::uid_t &uid, const node::ptr_t &c)
  {
    if (c.is_terminal()) {
      aw.push_terminal({ uid, is_high, ~c });
    } else {
      inner_pq.push({{c, node::ptr_t::NIL()}, {}, {uid.with(is_high)}});
    }
  }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it wants to sweep on some level.
  //////////////////////////////////////////////////////////////////////////////
  bool has_sweep(node::ptr_t::label_t l) const
  { return (l % _nesting_modulo) == 0u; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pick PQ type and Run Sweep.
  //////////////////////////////////////////////////////////////////////////////
  template<typename outer_roots_t>
  __bdd
  sweep(const shared_levelized_file<node> &outer_file,
        outer_roots_t &outer_roots,
        const size_t inner_memory)
  {
    return nested_sweeping::inner::down__sweep_switch(*this, outer_file, outer_roots, inner_memory);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create request
  //////////////////////////////////////////////////////////////////////////////
  static inline request_t
  request_from_node(const node &n, const ptr_uint64 &parent)
  {
    // Always pick high child
    return request_t({n.high(), only_gc ? node::ptr_t::NIL() : n.high()},
                     {},
                     {parent});
  }
};

class test_terminal_sweep : public bdd_policy
{
private:
  const size_t _nesting_modulo;

public:
  // HACK: The request is for a 'tuple' to allow us to tell the Nested Sweeping
  //       framework we are indeed changing subgraph.
  using request_t = request_data<2, with_parent, 0, 1>;
  using request_pred_t = request_fst_lt<request_t>;

  template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
  using pq_t = levelized_node_priority_queue<request_t, request_pred_t,
                                             LOOK_AHEAD, mem_mode>;

public:
  test_terminal_sweep(const size_t nm)
    : _nesting_modulo(nm)
  { }

public:
  //////////////////////////////////////////////////////////////////////////////
  static size_t stream_memory()
  { return 0u; }

  //////////////////////////////////////////////////////////////////////////////
  static size_t pq_memory(const size_t inner_memory)
  { return inner_memory; }

  //////////////////////////////////////////////////////////////////////////////
  static size_t pq_bound(const shared_levelized_file<node> &/*outer_file*/,
                         const size_t /*outer_roots*/)
  { return 1u; }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the PQ access mode case.
  //////////////////////////////////////////////////////////////////////////////
  template<typename inner_pq_t>
  __bdd
  sweep_pq(const shared_levelized_file<node> &outer_file,
          inner_pq_t &inner_pq,
          const size_t /*inner_remaining_memory == 0*/)
  {
    node_stream<> ns(outer_file);

    shared_levelized_file<arc> af;
    af->max_1level_cut = 1;

    arc_writer aw(af);

    while (!inner_pq.empty()) {
      inner_pq.setup_next_level();

      const node::label_t level_label = inner_pq.current_level();
      node::id_t level_size = 0u;

      while (!inner_pq.empty_level()) {
        // Get target of next request
        const node::ptr_t next = inner_pq.top().target.fst();
        node::ptr_t t;

        if (next.label() % _nesting_modulo == 1) {
          // Collapse immediately to a terminal
          t = node::ptr_t(level_label % (_nesting_modulo+1) > 0);
        } else {
          // Create a simple (i, _, _) node with two terminals.
          t = node::ptr_t(level_label, level_size++);

          const node::ptr_t t0((level_label)   % (_nesting_modulo) > 1);
          aw.push_terminal({ t, false, t0 });

          const node::ptr_t t1((level_label+1) % (_nesting_modulo) > 1);
          aw.push_terminal({ t, true,  t1 });
        }

        // Output all in-going edges that match the target
        while (inner_pq.can_pull() && inner_pq.top().target.fst() == next) {
          request_t req = inner_pq.pull();
          if (req.data.source.is_nil()) {
            if (t.is_terminal()) { return bdd(t.value()); }
          } else {
            aw.push({ req.data.source, t });
          }
        }
      }
      if (level_size > 0) {
        aw.push(level_info(level_label, level_size));
      }
    }
    return af;
  }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it wants to sweep on some level.
  //////////////////////////////////////////////////////////////////////////////
  bool has_sweep(node::ptr_t::label_t l) const
  { return (l % _nesting_modulo) == 0u; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pick PQ type and Run Sweep.
  //////////////////////////////////////////////////////////////////////////////
  template<typename outer_roots_t>
  __bdd
  sweep(const shared_levelized_file<node> &outer_file,
        outer_roots_t &outer_roots,
        const size_t inner_memory)
  {
    return nested_sweeping::inner::down__sweep_switch(*this, outer_file, outer_roots, inner_memory);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create request
  //////////////////////////////////////////////////////////////////////////////
  static inline request_t
  request_from_node(const node &n, const ptr_uint64 &parent)
  {
    // Always pick low child
    return request_t({n.low()}, {}, {parent});
  }
};

go_bandit([]() {
  describe("adiar/internal/algorithms/nested_sweeping.h", []() {
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

    const ptr_uint64 outer_n1 = ptr_uint64(0,0);
    const ptr_uint64 outer_n2 = ptr_uint64(1,0);
    const ptr_uint64 outer_n3 = ptr_uint64(2,0);
    const ptr_uint64 outer_n4 = ptr_uint64(3,0);
    const ptr_uint64 outer_n5 = ptr_uint64(4,0);

    shared_levelized_file<arc> outer_dag;

    { // Garbage collect writer to free write-lock
      arc_writer aw(outer_dag);

      aw.push_internal({ outer_n1, true, outer_n2 });
      aw.push_internal({ outer_n2, true, outer_n3 });
      aw.push_internal({ outer_n3, true, outer_n4 });
      aw.push_internal({ outer_n4, true, outer_n5 });

      aw.push_terminal({ outer_n1, false, terminal_F });
      aw.push_terminal({ outer_n2, false, terminal_F });
      aw.push_terminal({ outer_n3, false, terminal_F });
      aw.push_terminal({ outer_n4, false, terminal_F });
      aw.push_terminal({ outer_n5, false, terminal_F });
      aw.push_terminal({ outer_n5, true,  terminal_T });

      aw.push(level_info(0,1u));
      aw.push(level_info(1,1u));
      aw.push(level_info(2,1u));
      aw.push(level_info(3,1u));
      aw.push(level_info(4,1u));
    }

    describe("nested_sweeping:: _ ::decorators", [&]() {
      describe("outer::roots_sorter", []() {
        it("can sort pushed 'requests_t' [cardinality = 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
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

        it("provides the deepest source from arcs [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const reduce_arc a1 = arc(arc::ptr_t(0,0), false, arc::ptr_t(1,1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(0u));

          const reduce_arc a2 = arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));

          const reduce_arc a3 = arc(arc::ptr_t(0,0), true, arc::ptr_t(1,0));
          s.push(a3);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });

        it("provides the deepest source from arcs [2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const reduce_arc a1 = arc(arc::ptr_t(2,0), false, arc::ptr_t(3,1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          const reduce_arc a2 = arc(arc::ptr_t(1,0), false, arc::ptr_t(4,0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));
        });

        it("provides the deepest source from requests [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const test_request_t r1({arc::ptr_t(1,1)}, {}, {arc::ptr_t(0,0, false)});
          s.push(r1);

          AssertThat(s.deepest_source(), Is().EqualTo(0u));

          const test_request_t r2({arc::ptr_t(2,0)}, {}, {arc::ptr_t(1,0, false)});
          s.push(r2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));

          const test_request_t r3({arc::ptr_t(1,0)}, {}, {arc::ptr_t(0,0, true)});
          s.push(r3);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });

        it("provides the deepest source from requests [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const test_request_t r1({arc::ptr_t(3,1)}, {}, {arc::ptr_t(2,0, false)});
          s.push(r1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          const test_request_t r2({arc::ptr_t(4,0)}, {}, {arc::ptr_t(1,0, false)});
          s.push(r2);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));
        });

        it("provides a different deepest source after reset", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t =
            nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                                 test_request_t,
                                                 request_fst_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const reduce_arc a1 = arc(arc::ptr_t(2,0), false, arc::ptr_t(1,1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          s.reset();

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::NO_LEVEL));

          const reduce_arc a2 = arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });
      });

      describe("outer::up__pq_decorator", [&]() {
        using test_request_t = request_data<2, with_parent, 0, 1>;

        using test_roots_sorter_t =
          nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                               test_request_t,
                                               request_fst_lt<test_request_t>>;

        using test_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode_t::INTERNAL>;

        using test_decorator =
          nested_sweeping::outer::up__pq_decorator<test_pq_t, test_roots_sorter_t>;

        it("forwards internal arcs to PQ when below threshold", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n4, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal arcs to PQ when at threshold", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n3, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal arcs to Sorter when above threshold", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n2, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("forwards terminal arcs to PQ even if at threshold", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards terminal arcs to PQ even if above threshold", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n2, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards correctly with multiple arcs", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal below threshold
          d.push(arc(outer_n4, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // terminal at threshold
          d.push(arc(outer_n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal at threshold
          d.push(arc(outer_n3, true,  outer_n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal above threshold
          d.push(arc(outer_n2, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(1u));

          // terminal above threshold
          d.push(arc(outer_n1, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(4u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("provides combined size (w and w/o terminals)", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          AssertThat(d.empty(), Is().True());

          // internal below threshold
          d.push(arc(outer_n4, false, outer_n5));
          // terminal at threshold
          d.push(arc(outer_n3, false, terminal_F));
          // internal at threshold
          d.push(arc(outer_n3, true,  outer_n5));
          // internal above threshold
          d.push(arc(outer_n2, false, outer_n5));
          // terminal above threshold
          d.push(arc(outer_n1, false, terminal_T));

          AssertThat(d.empty(), Is().False());
          AssertThat(d.size(), Is().EqualTo(5u));
          AssertThat(d.terminals(false), Is().EqualTo(1u));
          AssertThat(d.terminals(true), Is().EqualTo(1u));
          AssertThat(d.size_without_terminals(), Is().EqualTo(3u));
        });

        it("forwards internal request to PQ", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(test_request_t({outer_n5}, {}, {with_out_idx(outer_n1, true)}));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("forwards terminal request to PQ", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(test_request_t({terminal_T}, {}, {outer_n1}));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards arc with NIL source to sorter", [&]() {
          test_pq_t pq({outer_dag}, memory_available(), 16);
          pq.setup_next_level(0u); // <-- set up pq to have no extra levels.

          AssertThat(pq.has_next_level(), Is().False()); // sanity check

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(arc(arc::ptr_t::NIL(), arc::ptr_t(42,0)));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });
      });

      describe("outer::inner_iterator", [&terminal_F, &terminal_T]() {
        using test_iter_t = nested_sweeping::outer::inner_iterator<test_not_sweep<>>;

        it("provides {NONE} for {3,2,1} % 4", [&]() {
          /*
          //
          //         1       ---- x1
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F T
          */
          const node::ptr_t n1(1,0);
          const node::ptr_t n2(2,0);
          const node::ptr_t n3(3,0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_writer aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true,  terminal_T });

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));
            aw.push(level_info(3,1u));
          }

          test_not_sweep<> inner_impl(4);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::NONE));
        });

        it("provides {2, NONE} for {3,2,1} % 2", [&]() {
          /*
          //
          //         1       ---- x1
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F T
          */
          const node::ptr_t n1(1,0);
          const node::ptr_t n2(2,0);
          const node::ptr_t n3(3,0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_writer aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true,  terminal_T });

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));
            aw.push(level_info(3,1u));
          }

          test_not_sweep<> inner_impl(2);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(2u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::NONE));
        });

        it("provides {4, 2, 0, NONE} for {4,3,2,0} % 2", [&]() {
          /*
          //
          //         1       ---- x0
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F 4    ---- x4
          //           / \
          //           F T
          */
          const node::ptr_t n1(0,0);
          const node::ptr_t n2(2,0);
          const node::ptr_t n3(3,0);
          const node::ptr_t n4(4,0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_writer aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });
            aw.push_internal({ n3, true, n4 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n4, false, terminal_F });
            aw.push_terminal({ n4, true,  terminal_T });

            aw.push(level_info(0,1u));
            aw.push(level_info(2,1u));
            aw.push(level_info(3,1u));
            aw.push(level_info(4,1u));
          }

          test_not_sweep<> inner_impl(2);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(4u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(2u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(0u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::NONE));
        });
      });

      describe("inner::down__pq_decorator", [&terminal_F, &terminal_T]() {
        const uid_uint64 inner_n1 = ptr_uint64(3,0);
        const uid_uint64 inner_n2 = ptr_uint64(3,1);
        const uid_uint64 inner_n3 = ptr_uint64(4,0);
        const uid_uint64 inner_n4 = ptr_uint64(4,1);
        const uid_uint64 inner_n5 = ptr_uint64(5,0);
        const uid_uint64 inner_n6 = ptr_uint64(5,1);

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

          nw << node(inner_n6, terminal_F, terminal_T)
             << node(inner_n5, terminal_T, terminal_F)
             << node(inner_n4, terminal_T, inner_n5)
             << node(inner_n3, terminal_F, terminal_T)
             << node(inner_n2, inner_n4,   inner_n6)
             << node(inner_n1, inner_n3,   inner_n4)
            ;
        }

        using test_request_t = request_data<1, with_parent, 0, 1>;

        using test_roots_sorter_t =
          nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                               test_request_t,
                                               request_fst_lt<test_request_t>>;

        const test_request_t root1({inner_n1}, {}, {ptr_uint64(1,0, false)});
        const test_request_t root2({inner_n2}, {}, {ptr_uint64(1,0, true)});
        const test_request_t root3({inner_n6}, {}, {ptr_uint64(1,1, false)});

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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());

          AssertThat(dec.has_next_level(), Is().True());
          AssertThat(dec.next_level(), Is().EqualTo(3u));

          AssertThat(dec.empty(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("merges size of PQ and Sorter", [&]() {
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          const test_request_t root4({inner_n2}, {}, {ptr_uint64(1,2, false)});
          pq.push(root4);

          test_decorator dec(pq, sorter);

          AssertThat(dec.size(), Is().EqualTo(4u));
        });

        it("setup_next_level() forward to first Sorter level", [&]() {
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(3u));
        });

        it("setup_next_level(k) forward to Sorter's level if k is larger", [&]() {
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level(5);

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(3u));
        });

        it("setup_next_level(k) forward to level k if k is smaller", [&]() {
          sorter.pull(); sorter.pull(); // Next is at level 6

          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level(4);

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(4u));
        });

        it("can pull requests from Sorter", [&]() {
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n2}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n1}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n1}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          const test_request_t root4({inner_n1}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n2}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n1}, {}, {ptr_uint64(1,2, false)});
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
          test_pq_t pq({bdd(dag)}, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({inner_n1}, {}, {ptr_uint64(1,2, false)});
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

      describe("inner::up__pq_decorator", [&]() {
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

          aw.push_internal({ outer_n3, true, outer_n4 });
          aw.push_internal({ outer_n4, true, outer_n5 });

          aw.push_terminal({ outer_n3, false, terminal_F });
          aw.push_terminal({ outer_n4, false, terminal_F });
          aw.push_terminal({ outer_n5, false, terminal_F });
          aw.push_terminal({ outer_n5, true,  terminal_T });

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

          dec.push(arc(outer_n4, false, terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards unflagged internal node to Inner PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(outer_n4, true, outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards flagged terminal to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(with_out_idx(outer_n4, false)), terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("forwards flagged internal node to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(with_out_idx(outer_n4, true)), outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("unflags source when handing over to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);
          dec.push(arc(flag(with_out_idx(outer_n4, true)), outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          outer_pq.setup_next_level();

          AssertThat(outer_pq.can_pull(), Is().True());
          AssertThat(outer_pq.pull(), Is().EqualTo(arc(outer_n4, true, outer_n5)));
        });

        it("sets up next level only based on Inner PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(with_out_idx(outer_n3, false),      terminal_F)); // inner (x2)
          dec.push(arc(flag(with_out_idx(outer_n4, true)), outer_n5));   // outer (x3)
          dec.push(arc(with_out_idx(outer_n3, true),       outer_n5));   // inner (x2)

          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(2u));
        });

        it("counts arcs from both Inner and Outer PQ", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(with_out_idx(outer_n4, false),      terminal_F)); // inner (x3)
          dec.push(arc(flag(with_out_idx(outer_n4, true)), outer_n5));   // outer (x3)
          dec.push(arc(with_out_idx(outer_n4, true),       outer_n5));   // inner (x3)

          AssertThat(outer_pq.size(), Is().EqualTo(1u));

          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.top(),  Is().EqualTo(arc(outer_n4, true, outer_n5)));
          AssertThat(dec.pull(), Is().EqualTo(arc(outer_n4, true, outer_n5)));

          AssertThat(dec.size(), Is().EqualTo(2u));
          AssertThat(dec.empty(), Is().False());

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.top(),  Is().EqualTo(arc(outer_n4, false, terminal_F)));
          AssertThat(dec.pull(), Is().EqualTo(arc(outer_n4, false, terminal_F)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.can_pull(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());

          dec.push(arc(flag(with_out_idx(outer_n3, false)), terminal_F)); // outer
          dec.push(arc(with_out_idx(outer_n3, true),        outer_n5));   // inner

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());
        });

        it("empty() is independent of Outer PQ size", [&]() {
          outer_test_pq_t outer_pq({outer_dag}, pq_mem, 16);
          inner_test_pq_t inner_pq({inner_dag}, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(flag(with_out_idx(outer_n4, true)), outer_n5));  // outer (x3)

          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());
        });
      });

      describe("inner::up__arc_stream__decorator", [&terminal_F, &terminal_T]() {
        const arc::ptr_t n0(0,0);
        const arc::ptr_t n1(1,0);
        const arc::ptr_t n2(2,0);

        shared_levelized_file<arc> outer;
        { // Garbage collect writer to free write-lock
          arc_writer aw(outer);

          aw.push_internal({ n0, false, n1 });
          aw.push_terminal({ n0, true,  terminal_F });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,2u));

          outer->max_1level_cut = 1;
        }

        shared_levelized_file<arc> inner;
        { // Garbage collect writer to free write-lock
          arc_writer aw(inner);

          aw.push_internal({ n1, true,  n2 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));

          inner->max_1level_cut = 1;
        }

        it("only reads from inner stream", [&]() {
          const arc_stream<> outer_stream(outer);
          arc_stream<> inner_stream(inner);

          nested_sweeping::inner::up__arc_stream__decorator dec(inner_stream, outer_stream);

          AssertThat(dec.can_pull_internal(), Is().True());
          AssertThat(dec.peek_internal(), Is().EqualTo(arc(n1, true, n2)));
          AssertThat(dec.pull_internal(), Is().EqualTo(arc(n1, true, n2)));
          AssertThat(dec.can_pull_internal(), Is().False());

          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n2, true,  terminal_T)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n2, true,  terminal_T)));
          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n2, false, terminal_F)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n2, false, terminal_F)));
          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n1, false, terminal_T)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n1, false, terminal_T)));
          AssertThat(dec.can_pull_terminal(), Is().False());
        });

        it("sums unread_terminals from both streams", [&outer, &inner]() {
          const arc_stream<> outer_stream(outer);
          arc_stream<> inner_stream(inner);

          nested_sweeping::inner::up__arc_stream__decorator dec(inner_stream, outer_stream);

          AssertThat(dec.unread_terminals(), Is().EqualTo(4u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(2u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(3u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(1u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(1u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(0u));
        });
      });
    });

    describe("nested_sweeping:: _ ::sweeps", [&terminal_F, &terminal_T, &outer_dag]() {
      describe("inner::down(...)", [&]() {
        using inner_down_sweep = test_not_sweep<>;
        using inner_roots_t =
          nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                               inner_down_sweep::request_t,
                                               request_fst_lt<inner_down_sweep::request_t>>;

        /*
        //       ?    ?        ---- x0
        // -   -   -    -   -
        //       2   3   4     ---- x1
        //      / \ / \ / \
        //      F T F  5  T    ---- x2
        //            / \
        //            F T
        */

        const node n5 = node(2,0, terminal_F, terminal_T);
        const node n4 = node(1,2, n5.uid(),   terminal_F);
        const node n3 = node(1,1, terminal_F, n5.uid());
        const node n2 = node(1,0, terminal_F, terminal_T);

        shared_levelized_file<node> outer_file;
        { // Garbage collect writer to free write-lock
          node_writer nw(outer_file);
          nw << n5 << n4 << n3 << n2;
        }

        it("can be run from a single root", [&]() {
          inner_down_sweep test_policy(2);

          /*
          //         _1_         ---- x0
          // -   -  / - \   -
          //        2   3   4    ---- x1
          */
          inner_roots_t inner_roots(1024, 8);
          const node::uid_t u1 = node::uid_t(0,0);

          inner_roots.push({{n2.uid()}, {}, {u1.with(false)}});
          inner_roots.push({{n3.uid()}, {}, {u1.with(true)}});

          const shared_levelized_file<arc> out =
            nested_sweeping::inner::down(test_policy, outer_file, inner_roots, memory_available())
            .template get<shared_levelized_file<arc>>();

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u1.with(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u1.with(true)),  n3.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n3.uid(), true,  n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));

          // Side effects on `inner_roots`.
          AssertThat(inner_roots.size(),  Is().EqualTo(0u));
          AssertThat(inner_roots.can_push(),  Is().True());
        });

        it("can be run from multiple roots", [&]() {
          inner_down_sweep test_policy(2);

          /*
          //       0   1        ---- x0
          // -   -/_\_/-\   -
          //      2 3   |       ---- x1
          //            |
          //            5       ---- x2
          //           / \
          //           F T
          */

          inner_roots_t inner_roots(1024, 8);
          const node::uid_t u0 = node::uid_t(0,0);
          const node::uid_t u1 = node::uid_t(0,1);

          inner_roots.push({{n2.uid()}, {}, {u0.with(false)}});
          inner_roots.push({{n3.uid()}, {}, {u0.with(true)}});

          inner_roots.push({{n2.uid()}, {}, {u1.with(false)}});
          inner_roots.push({{n5.uid()}, {}, {u1.with(true)}});

          const shared_levelized_file<arc> out =
            nested_sweeping::inner::down(test_policy, outer_file, inner_roots, memory_available())
            .template get<shared_levelized_file<arc>>();

          arc_test_stream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u0.with(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u1.with(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u0.with(true)),  n3.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { flag(u1.with(true)),  n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc { n3.uid(), true,  n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n2.uid(), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n3.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { n5.uid(), true,  terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_stream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));

          // Side effects on `inner_roots`.
          AssertThat(inner_roots.size(),  Is().EqualTo(0u));
          AssertThat(inner_roots.can_push(),  Is().True());
        });

        it("can collapse to a terminal", []() {
          test_terminal_sweep test_policy(2);

          /*
          //  NIL
          //   |
          //   1     ---- x1
          //  / \
          //  F T
          */
          const node n1(1, node::MAX_ID, node::ptr_t(false), node::ptr_t(true));

          shared_levelized_file<node> outer_file;
          { // Garbage collect writer to free write-lock
            node_writer nw(outer_file);
            nw << n1;
          }

          inner_roots_t inner_roots(1024, 8);

          inner_roots.push({{n1.uid()}, {}, {node::ptr_t::NIL()}});

          const shared_levelized_file<node> out =
            nested_sweeping::inner::down(test_policy, outer_file, inner_roots, memory_available())
            .template get<shared_levelized_file<node>>();

          // Check it looks all right
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });
      });

      describe("inner::up(...)", []() {
        using inner_up_sweep = nested_sweeping::inner::up__policy_t<bdd_policy>;
        using outer_pq_t     = nested_sweeping::outer::up__pq_t<1, memory_mode_t::INTERNAL>;

        it("reduces forest and pushes roots back out", []() {
          /* input
          //         _0__              ---- x0
          //        /    \
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

          const ptr_uint64 n0(0,0);
          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(1,1);
          const ptr_uint64 n3(2,0);
          const ptr_uint64 n4(2,1);
          const ptr_uint64 n5(2,2);
          const ptr_uint64 n6(3,0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push_internal({ n0, false, n1 });
            aw.push_internal({ n0, true,  n2 });

            aw.push(level_info(0,1u));
            aw.push(level_info(1,2u));
            aw.push(level_info(2,3u));

            in_outer->max_1level_cut = 3;
          }
          arc_stream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

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

            // NOTE: 'level_info(0,1u)' is not a processable part of the forest;
            // NOTE: 'level_info(1,2u)' is not a processable part of the forest;
            aw.push(level_info(2,3u));
            aw.push(level_info(3,1u));

            in_inner->max_1level_cut = 2;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //      1   2           ---- x1
          //  -   -\ /-\  -   -   -    -
          //        3   5         ---- x2
          //       / \ / \
          //       T 6 F T        ---- x3
          //        / \
          //        T F
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

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

        it("includes outer_pq terminals in cut size", []() {
          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
           */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));
            aw.push(level_info(3,1u));

            in_outer->max_1level_cut = 2;
          }
          arc_stream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true, terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.push({ n1, false, terminal_F });

          out_pq.setup_next_level(2u);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

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

        it("includes outer_arcs terminals in cut size", []() {
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

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push_terminal({ n1, false, terminal_F });

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));

            in_outer->max_1level_cut = 1;
          }
          arc_stream<> stream_outer(in_outer);
          AssertThat(stream_outer.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(stream_outer.unread_terminals(true),  Is().EqualTo(0u));

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            // NOTE: 'level_info(2,?u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2u);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       F T
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

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
          AssertThat(out_pq.size(),  Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("includes account for tainted arcs of 1-level cut", []() {
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

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));

            in_outer->max_1level_cut = 1;
          }
          arc_stream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n3, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));
            aw.push(level_info(4,1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       F T <-- T tainted in global cut
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut_type::INTERNAL],       Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE],  Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL],            Is().EqualTo(2u));

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

        it("leaves file empty if everything collapsed to a terminal", []() {
          /* input
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       T T
          */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));

            in_outer->max_1level_cut = 1;
          }
          arc_stream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n2, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        T <-- in 'out_pq', not in 'out'
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

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

        it("can deal with some root arcs go to a terminal", []() {
          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          const ptr_uint64 terminal_F(false);
          const ptr_uint64 terminal_T(true);

          const ptr_uint64 n1(1,0);
          const ptr_uint64 n2(3,0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_writer aw(in_outer);

            aw.push(level_info(1,1u));
            aw.push(level_info(2,1u));

            in_outer->max_1level_cut = 3;
          }
          arc_stream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in_inner);

            aw.push_internal({ flag(with_out_idx(n1, true)), n2 });

            aw.push_terminal({ flag(with_out_idx(n1, false)), terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true,  terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3,1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_writer out_writer(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({in_outer}, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          nested_sweeping::inner::up<inner_up_sweep>(stream_outer, out_pq, out_writer,
                                                     in_inner, available_memory / 2);

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

          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, node::ptr_t(3, node::MAX_ID))));
          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, false, terminal_F)));

          AssertThat(out_pq.can_pull(), Is().False());
        });
      });
    });

    describe("nested_sweep", []() {
      using test_up_sweep = nested_sweeping::outer::up__policy_t<bdd_policy>;

      it("returns input if it is a terminal", []() {
        /* input
        //    T
        */
        shared_levelized_file<node> in;
        {
          node_writer nw(in);
          nw << node(true);
        }

        test_not_sweep<> inner_impl(2);

        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        AssertThat(out.file_ptr(), Is().EqualTo(in));
      });

      it("accumulates a single nested sweep [non-root level] [1]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    T  _2_         ---- x2 (%2)
        //      /   \
        //      3   4       ---- x3
        //     / \ / \
        //     T T F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(3,0);
        const ptr_uint64 n4(3,1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, true,  n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true,  n4 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true,  terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1      ---- x1
        //    / \
        //    T |     ---- x2 (%2)
        //      |
        //      4     ---- x3
        //     / \
        //     T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       terminal_T,
                                                       node::ptr_t(3, node::MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("accumulates a single nested sweep [non-root level] [2]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    2   3         ---- x2 (%2)
        //   / \ / \
        //   F  4   5       ---- x3
        //     / \ / \
        //     F T T T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(2,1);
        const ptr_uint64 n4(3,0);
        const ptr_uint64 n5(3,1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true,  n3 });
          aw.push_internal({ n2, true,  n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true,  n5 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true,  terminal_T });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,2u));
          aw.push(level_info(3,2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //       1     ---- x1
        //      / \
        //      | T    ---- x2 (%2)
        //      |
        //      4      ---- x3
        //     / \
        //     T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(3, ptr_uint64::MAX_ID),
                                                       terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));

        // NOTE: Since the outer priority queue includes a tainted arc to T, it
        //       is in fact counted twice: first it is counted in the inner
        //       sweep to then be counted once more in the outer
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("accumulates a single nested sweep [root level]", []() {
        /* input
        //          1       ---- x0 (%2)
        //         / \
        //         F 2      ---- x1
        //          / \
        //          F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //             ---- x0
        //
        //     2       ---- x1
        //    / \
        //    T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      // TODO (bail-out optimisation): add non-gc and non-terminal policy.
      it("accumulates multiple nested sweeps [excl. root]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    2   3         ---- x2 (%2)
        //   / \ / \
        //   F  4   5       ---- x3
        //     / \  ||
        //     F |  6       ---- x4 (%2)
        //       \ / \
        //        7  8      ---- x5
        //       / \/ \
        //       F  T F
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(2,1);
        const ptr_uint64 n4(3,0);
        const ptr_uint64 n5(3,1);
        const ptr_uint64 n6(4,0);
        const ptr_uint64 n7(5,0);
        const ptr_uint64 n8(5,1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true,  n3 });
          aw.push_internal({ n2, true,  n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true,  n5 });
          aw.push_internal({ n5, false, n6 });
          aw.push_internal({ n5, true,  n6 });
          aw.push_internal({ n4, true,  n7 });
          aw.push_internal({ n6, false, n7 });
          aw.push_internal({ n6, true,  n8 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n7, false, terminal_F });
          aw.push_terminal({ n7, true,  terminal_T });
          aw.push_terminal({ n8, false, terminal_T });
          aw.push_terminal({ n8, true,  terminal_F });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,2u));
          aw.push(level_info(3,2u));
          aw.push(level_info(4,1u));
          aw.push(level_info(5,2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output (note, they are doubly-negated!)
        //       1     ---- x1
        //      / \
        //     /   \   ---- x2
        //     |   |
        //     4   /   ---- x3
        //    / \ /
        //    T  X     ---- x4
        //      / \
        //      7  8   ---- x5
        //     / \/ \
        //     T F  T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n8
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID, terminal_F, terminal_T)));

        // n7
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-1, terminal_T, terminal_F)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                       terminal_T,
                                                       ptr_uint64(5, ptr_uint64::MAX_ID))));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(3, ptr_uint64::MAX_ID),
                                                       ptr_uint64(5, ptr_uint64::MAX_ID-1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(5u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("accumulates multiple nested sweeps [incl. root]", []() {
        /* input
        //         _1_       ---- x0 (%2)
        //        /   \
        //        2   3      ---- x1
        //       / \ / \
        //       T  4   5    ---- x2 (%2)
        //         / \ / \
        //         T  6  F   ---- x3
        //           / \
        //           F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);
        const ptr_uint64 n3(1,1);
        const ptr_uint64 n4(2,0);
        const ptr_uint64 n5(2,1);
        const ptr_uint64 n6(3,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true,  n3 });
          aw.push_internal({ n2, true,  n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true,  n5 });
          aw.push_internal({ n4, true,  n6 });
          aw.push_internal({ n5, false, n6 });

          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n5, true,  terminal_F });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,2u));
          aw.push(level_info(2,2u));
          aw.push(level_info(3,1u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //                    ---- x0
        //
        //        _3_         ---- x1
        //       /   \
        //       |   T        ---- x2
        //       |
        //       6            ---- x3
        //      / \
        //      F T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n6
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       node::ptr_t(3, node::MAX_ID),
                                                       terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("accumulates multiple nested sweeps on (transposed) node file", []() {
        /* input
        // See 'accumulates multiple nested sweeps [excl. root]' above but with
        // (5) removed since it would realistically be reduced away.
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        // In canonical order (realistic output from Reduce)
        const node n7(5,node::MAX_ID,   terminal_F, terminal_T);
        const node n8(5,node::MAX_ID-1, terminal_T, terminal_F);
        const node n6(4,node::MAX_ID,   n7.uid(),   n8.uid());
        //const node n5(3,_,            n6.uid(),   n6.uid());
        const node n4(3,node::MAX_ID,   terminal_F, n7.uid());
        const node n3(2,node::MAX_ID,   n4.uid(),   n6.uid());
        const node n2(2,node::MAX_ID-1, terminal_F, n4.uid());
        const node n1(1,node::MAX_ID,   n2.uid(),   n3.uid());

        shared_levelized_file<node> in;

        { // Garbage collect writer to free write-lock
          node_writer nw(in);
          nw << n7 << n8 << n6 /*<< n5*/ << n4 << n3 << n2 << n1;
        }

        // Just a sanity check we created the input as intended
        AssertThat(in->canonical, Is().True());

        test_not_sweep<> inner_impl(2);

        /* output
        // See 'accumulates multiple nested sweeps [excl. root]' above.
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n8
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID, terminal_F, terminal_T)));

        // n7
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-1, terminal_T, terminal_F)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                       terminal_T,
                                                       ptr_uint64(5, ptr_uint64::MAX_ID))));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(3, ptr_uint64::MAX_ID),
                                                       ptr_uint64(5, ptr_uint64::MAX_ID-1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(5u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("accumulates multiple nested sweeps with arcs that span multiple sweeps", []() {
        // Here, we have an arc that spans multiple existing levels where an
        // inner sweep has to be started on.

        /* input
        //           _1_        ---- x1
        //          /   \
        //          2    \      ---- x2 (%2)
        //         / \   |
        //         | 3   |      ---- x3
        //         |/ \  /
        //         4_ F /       ---- x4 (%2)
        //        /  \ /
        //        T   5         ---- x5
        //           / \
        //           F T
        */

        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(3,0);
        const ptr_uint64 n4(4,0);
        const ptr_uint64 n5(5,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, true,  n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n1, true,  n5 });
          aw.push_internal({ n4, true,  n5 });

          aw.push_terminal({ n3, true,  terminal_F });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
          aw.push(level_info(5,1u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //         _1_       ---- x1
        //        /   \
        //        |   |      ---- x2
        //        |   |
        //        3   |      ---- x3
        //       / \  /
        //       \ T /       ---- x4
        //        \ /
        //         5         ---- x5
        //        / \
        //        F T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n5
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                       node::ptr_t(5, node::MAX_ID),
                                                       terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       node::ptr_t(3, node::MAX_ID),
                                                       node::ptr_t(5, node::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("handles all root arcs collapsing into terminals", []() {
        // Here, the Inner Down Sweep collapses each arc to a terminal, but
        // the nodes above it in the Outer Sweep will remain alive.

        /* input
        //         _1_        ---- x1
        //        /   \
        //        2   3       ---- x2 (%2)
        //       / \ / \
        //       |  4  |      ---- x3
        //       | / \ |
        //       | F T |      ---- x4
        //       |     |
        //       5     6      ---- x5
        //      / \   / \
        //      T F   F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(2,1);
        const ptr_uint64 n4(3,0);
        const ptr_uint64 n5(5,0);
        const ptr_uint64 n6(5,1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true,  n3 });
          aw.push_internal({ n2, true,  n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n2, false, n5 });
          aw.push_internal({ n3, true,  n6 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true,  terminal_T });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true,  terminal_F });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,2u));
          aw.push(level_info(3,1u));
          aw.push(level_info(5,2u));

          in->max_1level_cut = 4;
        }

        test_terminal_sweep inner_impl(2);

        /* output
        //        1            ---- x1
        //       / \
        //       T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_terminal_sweep>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("handles some root arcs collapsing into terminals", []() {
        // Here, the Inner Down Sweep collapses one arc to a terminal but
        // creates another node.

        /* input
        //         _1_        ---- x2
        //        /   \
        //        2   |       ---- x3 (%3)
        //       / \  |
        //       3 T  |       ---- x4 (collapse to terminal)
        //      / \   |
        //      F T   4       ---- x5 (create node)
        //           / \
        //           F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(2,0);
        const ptr_uint64 n2(3,0);
        const ptr_uint64 n3(4,0);
        const ptr_uint64 n4(5,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n1, true,  n4 });

          aw.push_terminal({ n2, true,  terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true,  terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true,  terminal_T });

          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));
          aw.push(level_info(4,1u));
          aw.push(level_info(5,1u));

          in->max_1level_cut = 2;
        }

        test_terminal_sweep inner_impl(3);

        /* output
        //         _1_        ---- x2
        //        /   \
        //       /    |       ---- x3 (%3)
        //       |    |
        //       F    |       ---- x4
        //            |
        //            4'      ---- x5
        //           / \
        //           T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_terminal_sweep>(in, inner_impl);


        // Check it looks all right
        node_test_stream out_nodes(out);

        // n4'
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       terminal_F,
                                                       node::ptr_t(5, node::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5,1u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can garbage collect temp solution when no parent references it [non-root level]", []() {
        /* input
        //     1       ---- x1
        //    / \
        //    F 2      ---- x2 (%2)
        //     / \
        //     3 T     ---- x3
        //    / \
        //    F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(3,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, true,  n2 });
          aw.push_internal({ n2, false, n3 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1       ---- x1
        //    / \
        //    F T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can suppress nesting-level node into a terminal [non-root level]", []() {
        /* input
        //     1       ---- x1
        //    / \
        //    F 2      ---- x2 (%2)
        //     / \
        //     3 T     ---- x3
        //    / \
        //    T T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);
        const ptr_uint64 n3(3,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, true,  n2 });
          aw.push_internal({ n2, false, n3 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });
          aw.push_terminal({ n3, false, terminal_T });
          aw.push_terminal({ n3, true,  terminal_T });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));
          aw.push(level_info(3,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1       ---- x1
        //    / \
        //    F T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       terminal_F,
                                                       terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can suppress nesting-level node into a terminal [root level]", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    T T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false,  n2 });

          aw.push_terminal({ n1, true,  terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can preserve subtree for suppressed root with inner sweep", []() {
        /* input
        //          1      ---- x0 (%2)
        //          ||
        //          2      ---- x1
        //         / \
        //         F T
        */

        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true,  n2 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));

          in->max_1level_cut = 2;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     2         ---- x1
        //    / \
        //    T F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can solve root-level nesting request directly to a terminal", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    F T
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false,  n2 });

          aw.push_terminal({ n1, true,  terminal_T });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true,  terminal_T });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can collapse to a terminal [outer up]", []() {
        /* input
        //      1      ---- x1
        //     / \
        //     2 F     ---- x2 (%2)
        //    / \
        //    T F
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(1,0);
        const ptr_uint64 n2(2,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false,  n2 });

          aw.push_terminal({ n1, true,  terminal_F });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true,  terminal_F });

          aw.push(level_info(1,1u));
          aw.push(level_info(2,1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     F
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_not_sweep<>>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("can collapse to a terminal [inner down]", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    T F
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(1,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true,  terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true,  terminal_F });

          aw.push(level_info(0,1u));
          aw.push(level_info(1,1u));

          in->max_1level_cut = 1;
        }

        test_terminal_sweep inner_impl(2);

        /* output
        //     T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_terminal_sweep>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can collapse to a terminal [inner up]", []() {
        /* input
        //      1      ---- x0 (%4)
        //     / \
        //     | T     ---- x1
        //     |
        //     2       ---- x2
        //    / \
        //    T F
        */
        const ptr_uint64 terminal_F(false);
        const ptr_uint64 terminal_T(true);

        const ptr_uint64 n1(0,0);
        const ptr_uint64 n2(2,0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true,  terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true,  terminal_F });

          aw.push(level_info(0,1u));
          aw.push(level_info(2,1u));

          in->max_1level_cut = 1;
        }

        test_terminal_sweep inner_impl(4);
        /* output
        //     T   <-- reduced from  2'    ---- x2
        //                          / \
        //                          T T
        */
        const bdd out
          = nested_sweep<test_up_sweep, test_terminal_sweep>(in, inner_impl);

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });
  });
 });
