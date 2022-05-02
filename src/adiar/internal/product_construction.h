#ifndef ADIAR_INTERNAL_PRODUCT_CONSTRUCTION_H
#define ADIAR_INTERNAL_PRODUCT_CONSTRUCTION_H

#include <variant>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/bdd/bdd.h>

#include <adiar/internal/build.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/tuple.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::product_construction_t stats_product_construction;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct prod_tuple_1 : tuple
  {
    ptr_t source;
  };

#ifndef NDEBUG
  struct prod_tuple_1_lt : public std::binary_function<tuple, tuple, bool>
  {
    bool operator()(const prod_tuple_1 &a, const prod_tuple_1 &b)
    {
      return tuple_fst_lt()(a,b)
        || (!tuple_fst_lt()(b,a) && a.source < b.source)
        ;
    }
  };
#else
  typedef tuple_fst_lt prod_tuple_1_lt;
#endif

  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using prod_priority_queue_1_t =
    levelized_node_priority_queue<prod_tuple_1, tuple_label, prod_tuple_1_lt,
                                  sorter_template, priority_queue_template,
                                  2>;

  struct prod_tuple_2 : tuple_data
  {
    ptr_t source;
  };

#ifndef NDEBUG
  struct prod_tuple_2_lt : public std::binary_function<tuple, tuple, bool>
  {
    bool operator()(const prod_tuple_2 &a, const prod_tuple_2 &b)
    {
      return tuple_snd_lt()(a,b)
        || (!tuple_snd_lt()(b,a) && a.source < b.source)
        ;
    }
  };
#else
  typedef tuple_snd_lt prod_tuple_2_lt;
#endif

  template<template<typename, typename> typename priority_queue_template>
  using prod_priority_queue_2_t = priority_queue_template<prod_tuple_2, prod_tuple_2_lt>;

  struct prod_rec_output {
    tuple low;
    tuple high;
  };

  struct prod_rec_skipto : tuple { };

  typedef std::variant<prod_rec_output, prod_rec_skipto> prod_rec;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename pq_1_t>
  inline void prod_recurse_out(pq_1_t &prod_pq_1, arc_writer &aw,
                               const bool_op &op,
                               ptr_t source, tuple target)
  {
    if (is_sink(target.t1) && is_sink(target.t2)) {
      arc_t out_arc = { source, op(target.t1, target.t2) };
      aw.unsafe_push_sink(out_arc);
    } else {
      adiar_debug(label_of(source) < label_of(std::min(target.t1, target.t2)),
                  "should always push recursion for 'later' level");

      prod_pq_1.push({ target.t1, target.t2, source });
    }
  }

  template<typename out_policy, typename extra_arg, typename pq_1_t, typename pq_2_t>
  inline void prod_recurse_in(pq_1_t &prod_pq_1, pq_2_t &prod_pq_2,
                              arc_writer &aw,
                              const extra_arg &ea, ptr_t t1, ptr_t t2)
  {
    while (prod_pq_1.can_pull() && prod_pq_1.top().t1 == t1 && prod_pq_1.top().t2 == t2) {
      out_policy::go(prod_pq_1, aw, ea, prod_pq_1.pull().source);
    }

    while (!prod_pq_2.empty() && prod_pq_2.top().t1 == t1 && prod_pq_2.top().t2 == t2) {
      out_policy::go(prod_pq_1, aw, ea, prod_pq_2.top().source);
      prod_pq_2.pop();
    }
  }

  struct prod_recurse_in__output_node
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t& /*prod_pq_1*/, arc_writer &aw,
                          uid_t out_uid, ptr_t source)
    {
      if (!is_nil(source)) {
        aw.unsafe_push_node({ source, out_uid });
      }
    }
  };

  struct prod_recurse_in__output_sink
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t& /*prod_pq_1*/, arc_writer &aw,
                          ptr_t out_sink, ptr_t source)
    {
      aw.unsafe_push_sink({ source, out_sink });
    }
  };

  struct prod_recurse_in__forward
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t &prod_pq_1, arc_writer&,
                          const prod_rec_skipto &r, ptr_t source)
    {
      prod_pq_1.push({ r.t1, r.t2, source });
    }
  };

  inline node_file prod_sink(ptr_t t1, ptr_t t2, const bool_op &op)
  {
    // TODO: Abuse that op(t1,t2) already is a pointer.
    return build_sink(value_of(op(t1,t2)));
  }

  inline bool prod_from_1(ptr_t t1, ptr_t t2)
  {
    return fst(t1,t2) == t1;
  }

  //////////////////////////////////////////////////////////////////////////////
  class prod_same_level_merger
  {
  public:
    static void merge_root(ptr_t &low1, ptr_t &high1, ptr_t &low2, ptr_t &high2,
                           label_t /* level */,
                           const node_t &v1, const node_t &v2)
    {
      low1 = v1.low;
      high1 = v1.high;

      low2 = v2.low;
      high2 = v2.high;
    }

  public:
    static void merge_data(ptr_t &low1, ptr_t &high1, ptr_t &low2, ptr_t &high2,
                           ptr_t t1, ptr_t t2, ptr_t t_seek,
                           const node_t &v1, const node_t &v2,
                           ptr_t data_low, ptr_t data_high)
    {
      low1  = t1 < t_seek ? data_low  : v1.low;
      high1 = t1 < t_seek ? data_high : v1.high;

      low2  = t2 < t_seek ? data_low  : v2.low;
      high2 = t2 < t_seek ? data_high : v2.high;
    }
  };

  class prod_mixed_level_merger
  {
  private:
    static void __merge_root(const node_t &v, label_t level,
                             ptr_t &low, ptr_t &high)
    {
      if (!is_sink(v) && label_of(v) == level) {
        low = v.low;
        high = v.high;
      } else {
        low = high = v.uid;
      }
    }

  public:
    static void merge_root(ptr_t &low1, ptr_t &high1, ptr_t &low2, ptr_t &high2,
                           label_t level,
                           const node_t &v1, const node_t &v2)
    {
      __merge_root(v1, level, low1, high1);
      __merge_root(v2, level, low2, high2);
    }

  public:
    static void merge_data(ptr_t &low1, ptr_t &high1, ptr_t &low2, ptr_t &high2,
                           ptr_t t1, ptr_t t2, ptr_t t_seek,
                           const node_t &v1, const node_t &v2,
                           ptr_t data_low, ptr_t data_high)
    {
      if (is_sink(t1) || is_sink(t2) || label_of(t1) != label_of(t2)) {
        if (t1 < t2) { // ==> label_of(t1) < label_of(t2) || is_sink(t2)
          low1 = v1.low;
          high1 = v1.high;
          low2 = high2 = t2;
        } else { // ==> label_of(t1) > label_of(t2) || is_sink(t1)
          low1 = high1 = t1;
          low2 = v2.low;
          high2 = v2.high;
        }
      } else {
        prod_same_level_merger::merge_data(low1,high1, low2,high2,
                                           t1, t2, t_seek,
                                           v1, v2,
                                           data_low, data_high);
      }
    }
  };

  template<typename prod_policy, typename pq_1_t, typename pq_2_t>
  typename prod_policy::unreduced_t
  __product_construction(const typename prod_policy::reduced_t &in_1,
                         const typename prod_policy::reduced_t &in_2,
                         const bool_op &op,
                         const size_t pq_1_memory,
                         const size_t pq_2_memory,
                         const size_t max_pq_size)
  {
    node_stream<> in_nodes_1(in_1);
    node_stream<> in_nodes_2(in_2);

    node_t v1 = in_nodes_1.pull();
    node_t v2 = in_nodes_2.pull();

    if (is_sink(v1) || is_sink(v2)) {
      typename prod_policy::unreduced_t maybe_resolved = prod_policy::resolve_sink_root(v1, in_1, v2, in_2, op);

      if (!(maybe_resolved.template has<no_file>())) {
        return maybe_resolved;
      }
    }

    // Set-up for Product Construction Algorithm
    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_1_t prod_pq_1({in_1, in_2}, pq_1_memory, max_pq_size);
    pq_2_t prod_pq_2(pq_2_memory, max_pq_size);

    // Process root and create initial recursion requests
    label_t out_label = label_of(fst(v1.uid, v2.uid));
    id_t out_id = 0;

    ptr_t low1, low2, high1, high2;
    prod_policy::merge_root(low1,high1, low2,high2, out_label, v1, v2);

    // Shortcut the root (maybe)
    {
      prod_policy::compute_cofactor(on_level(v1, out_label), low1, high1);
      prod_policy::compute_cofactor(on_level(v2, out_label), low2, high2);

      prod_rec root_rec = prod_policy::resolve_request(op, low1, low2, high1, high2);

      if (std::holds_alternative<prod_rec_output>(root_rec)) {
        prod_rec_output r = std::get<prod_rec_output>(root_rec);
        uid_t out_uid = create_node_uid(out_label, out_id++);

        prod_recurse_out(prod_pq_1, aw, op, out_uid, r.low);
        prod_recurse_out(prod_pq_1, aw, op, flag(out_uid), r.high);
      } else { // std::holds_alternative<prod_rec_skipto>(root_rec)
        prod_rec_skipto r = std::get<prod_rec_skipto>(root_rec);

        if (is_sink(r.t1) && is_sink(r.t2)) {
          return prod_sink(r.t1, r.t2, op);
        } else {
          prod_pq_1.push({ r.t1, r.t2, NIL });
        }
      }
    }

    size_t max_1level_cut = 0;

    // Process nodes in topological order of both BDDs
    while (!prod_pq_1.empty() || !prod_pq_2.empty()) {
      if (prod_pq_1.empty_level() && prod_pq_2.empty()) {
        if (prod_policy::no_skip || out_id > 0) {
          // Only output level_info information on prior level, if output
          aw.unsafe_push(create_level_info(out_label, out_id));
        }

        prod_pq_1.setup_next_level();
        out_label = prod_pq_1.current_level();
        out_id = 0;

        max_1level_cut = std::max(max_1level_cut, prod_pq_1.size());
      }

      ptr_t source, t1, t2;
      bool with_data = false;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from  prod_pq_1 or prod_pq_2
      if (prod_pq_1.can_pull() && (prod_pq_2.empty() ||
                                   fst(prod_pq_1.top()) < snd(prod_pq_2.top()))) {
        source = prod_pq_1.top().source;
        t1 = prod_pq_1.top().t1;
        t2 = prod_pq_1.top().t2;
      } else {
        source = prod_pq_2.top().source;
        t1 = prod_pq_2.top().t1;
        t2 = prod_pq_2.top().t2;

        with_data = true;
        data_low = prod_pq_2.top().data_low;
        data_high = prod_pq_2.top().data_high;
      }

      adiar_invariant(is_sink(t1) || out_label <= label_of(t1),
                      "Request should never level-wise be behind current position");
      adiar_invariant(is_sink(t2) || out_label <= label_of(t2),
                      "Request should never level-wise be behind current position");

      // Seek request partially in stream
      ptr_t t_seek = with_data ? snd(t1,t2) : fst(t1,t2);
      while (v1.uid < t_seek && in_nodes_1.can_pull()) {
        v1 = in_nodes_1.pull();
      }
      while (v2.uid < t_seek && in_nodes_2.can_pull()) {
        v2 = in_nodes_2.pull();
      }

      // Forward information across the level
      if (is_node(t1) && is_node(t2) && label_of(t1) == label_of(t2)
          && !with_data && (v1.uid != t1 || v2.uid != t2)) {
        node_t v0 = t1 == v1.uid /*prod_from_1(t1,t2)*/ ? v1 : v2;

        while (prod_pq_1.can_pull() && prod_pq_1.top().t1 == t1 && prod_pq_1.top().t2 == t2) {
          source = prod_pq_1.pull().source;
          prod_pq_2.push({ t1, t2, v0.low, v0.high, source });
        }
        continue;
      }

      // Resolve current node and recurse
      // remember from above: ptr_t low1, low2, high1, high2;
      prod_policy::merge_data(low1,high1, low2,high2,
                              t1, t2, t_seek,
                              v1, v2,
                              data_low, data_high);

      // Resolve request
      prod_policy::compute_cofactor(on_level(t1, out_label), low1, high1);
      prod_policy::compute_cofactor(on_level(t2, out_label), low2, high2);

      prod_rec rec_res = prod_policy::resolve_request(op, low1, low2, high1, high2);

      if (prod_policy::no_skip || std::holds_alternative<prod_rec_output>(rec_res)) {
        prod_rec_output r = std::get<prod_rec_output>(rec_res);

        adiar_debug(out_id < MAX_ID, "Has run out of ids");
        uid_t out_uid = create_node_uid(out_label, out_id++);

        prod_recurse_out(prod_pq_1, aw, op, out_uid, r.low);
        prod_recurse_out(prod_pq_1, aw, op, flag(out_uid), r.high);

        prod_recurse_in<prod_recurse_in__output_node>(prod_pq_1, prod_pq_2, aw, out_uid, t1, t2);

      } else { // std::holds_alternative<prod_rec_skipto>(root_rec)
        prod_rec_skipto r = std::get<prod_rec_skipto>(rec_res);
        if (is_sink(r.t1) && is_sink(r.t2)) {
          if (is_nil(source)) {
            // Skipped in both DAGs all the way from the root until a pair of sinks.
            return prod_sink(r.t1, r.t2, op);
          }
          prod_recurse_in<prod_recurse_in__output_sink>(prod_pq_1, prod_pq_2, aw, op(r.t1, r.t2), t1, t2);
        } else {
          prod_recurse_in<prod_recurse_in__forward>(prod_pq_1, prod_pq_2, aw, r, t1, t2);
        }
      }
    }

    if (prod_policy::no_skip || out_id > 0) {
      // Push the level of the very last iteration
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    out_arcs->max_1level_cut = max_1level_cut;

    return out_arcs;
  }

  template<typename prod_policy>
  cut_size_t __prod_2level_upper_bound(const typename prod_policy::reduced_t &in_1,
                                       const typename prod_policy::reduced_t &in_2,
                                       const bool_op &op)
  {
    const cut_type left_ct = prod_policy::left_cut(op);
    const cut_size_t left_2level_cut = in_1.max_2level_cut(left_ct);

    const cut_type right_ct = prod_policy::right_cut(op);
    const cut_size_t right_2level_cut = in_2.max_2level_cut(right_ct);

    const bits_approximation left_bits(left_2level_cut);
    const bits_approximation right_bits(right_2level_cut);

    return (left_bits * right_bits + 2).may_overflow()
      ? MAX_CUT
      : left_2level_cut * right_2level_cut + 2;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Creates the product construction of the given two DAGs.
  ///
  /// Behaviour of the product construction is controlled with the 'prod_policy'
  /// class, which exposes static void strategy functions.
  ///
  /// - resolve_same_file:
  ///   Creates the output based on knowing both inputs refer to the same
  ///   underlying file.
  ///
  /// - resolve_sink_root:
  ///   Resolves (if possible) the cases for one of the two DAGs only being a
  ///   sink. Uses the _union in the 'out_t' to trigger an early termination. If
  ///   it holds an 'adiar::no_file', then the algorithm proceeds to the actual
  ///   product construction.
  ///
  /// - resolve_request:
  ///   Given all information collected for the two nodes (both children, if
  ///   they are on the same level. Otherwise, only the first-seen node),
  ///   returns whether (a) a node should be output and its two children to
  ///   recurse to or (b) no node should be output (i.e. it is skipped) and what
  ///   child to forward the request to.
  ///
  /// - no_skip:
  ///   Constexpr boolean whether the strategy guarantees never to skip a level.
  ///   This shortcuts some boolean conditions at compile-time.
  ///
  /// This 'prod_policy' also should inherit the general policy for the
  /// decision_diagram used (i.e. bdd_policy in bdd/bdd.h, zdd_policy in
  /// zdd/zdd.h and so on). This provides the following functions
  ///
  /// - compute_cofactor:
  ///   Used to change the low and high children retrieved from the input during
  ///   the product construction.
  ///
  /// Other parameters are:
  ///
  /// \param in_i  DAGs to combine into one.
  ///
  /// \param op    Binary boolean operator to be applied. See data.h for the
  ///              ones directly provided by Adiar.
  ///
  /// \return      A class that inherits from __decision_diagram and describes
  ///              the product of the two given DAGs.
  //////////////////////////////////////////////////////////////////////////////
  template<typename prod_policy>
  typename prod_policy::unreduced_t product_construction(const typename prod_policy::reduced_t &in_1,
                                                         const typename prod_policy::reduced_t &in_2,
                                                         const bool_op &op)
  {
    if (in_1.file_ptr() == in_2.file_ptr()) {
      return prod_policy::resolve_same_file(in_1, in_2, op);
    }

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory::available()
      // Input streams
      - 2*node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      prod_priority_queue_1_t<internal_sorter, internal_priority_queue>::DATA_STRUCTURES;

    constexpr size_t data_structures_in_pq_2 =
      prod_priority_queue_2_t<internal_priority_queue>::DATA_STRUCTURES;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t max_pq_size = __prod_2level_upper_bound<prod_policy>(in_1, in_2, op);

    const size_t pq_1_memory_fits =
      prod_priority_queue_1_t<internal_sorter, internal_priority_queue>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      prod_priority_queue_2_t<internal_priority_queue>::memory_fits(pq_2_internal_memory);

    // TODO: maximum 1-level cut suffices for pq_2!
    if(max_pq_size <= pq_1_memory_fits && max_pq_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_product_construction.lpq_internal++;
#endif
      return __product_construction<prod_policy,
                                    prod_priority_queue_1_t<internal_sorter, internal_priority_queue>,
                                    prod_priority_queue_2_t<internal_priority_queue>>
        (in_1, in_2, op, pq_1_internal_memory, pq_2_internal_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_product_construction.lpq_external++;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __product_construction<prod_policy,
                                    prod_priority_queue_1_t<external_sorter, external_priority_queue>,
                                    prod_priority_queue_2_t<external_priority_queue>>
        (in_1, in_2, op, pq_1_memory, pq_2_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_PRODUCT_CONSTRUCTION_H
