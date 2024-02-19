#ifndef ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
#define ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H

#include <algorithm>
#include <limits>
#include <queue>
#include <variant>

#include <adiar/exec_policy.h>
#include <adiar/functional.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/node_arc_stream.h>
#include <adiar/internal/io/node_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/unreachable.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //  Quantification
  // ================
  //
  // Given a Decision Diagram and one (or more) variables, runs a product construction on the
  // children at the desired levels (removing the level in question).
  /*
  //             ____ O ____                    O
  //            /           \                 /   \
  //          (a)          (b)               /     \
  //         /   \    X   /   \     =>      /       \
  //        a0   a1      b1   b2        (a0,b0)   (a1,b1)
  */
  // Examples of uses are `bdd_exists` and `zdd_project`.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  template <uint8_t NodesCarried>
  using quantify_request = request_data<2, with_parent, NodesCarried, 1>;

  /// \brief Type of the primary priority queue for node-based inputs.
  template <size_t LookAhead, memory_mode MemoryMode>
  using quantify_priority_queue_1_node_t =
    levelized_node_priority_queue<quantify_request<0>,
                                  request_data_first_lt<quantify_request<0>>,
                                  LookAhead,
                                  MemoryMode,
                                  1,
                                  0>;

  /// \brief Type of the primary priority queue for arc-based inputs.
  ///
  /// \details This is used during repeated (partial) quantification.
  template <size_t LookAhead, memory_mode MemoryMode>
  using quantify_priority_queue_1_arc_t =
    levelized_node_arc_priority_queue<quantify_request<0>,
                                      request_data_first_lt<quantify_request<0>>,
                                      LookAhead,
                                      MemoryMode,
                                      1,
                                      0>;

  /// \brief Type of the secondary priority queue to further forward requests across a level.
  template <memory_mode MemoryMode>
  using quantify_priority_queue_2_t =
    priority_queue<MemoryMode, quantify_request<1>, request_data_second_lt<quantify_request<1>>>;

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common i-level cut computations

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Derives an upper bound on the output's maximum i-level cut given its maximum i-level
  ///        cut.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
  size_t
  __quantify_ilevel_upper_bound(const In& in, const bool_op& op)
  {
    const typename Cut::type ct_internal  = cut::type::Internal;
    const typename Cut::type ct_terminals = Policy::cut_with_terminals(op);

    const safe_size_t max_cut_internal  = Cut::get(in, ct_internal);
    const safe_size_t max_cut_terminals = Cut::get(in, ct_terminals);

    return to_size(max_cut_internal * max_cut_terminals + ConstSizeInc);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its size.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename In>
  size_t
  __quantify_ilevel_upper_bound(const In& in)
  {
    const safe_size_t in_size = in.size();
    return to_size(in_size * in_size + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common logic for sweeps

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Places a request from `source` to `target` in the output if resolved to a terminal.
  ///        Otherwise, it is placed in the priority queue to be resolved later.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue>
  inline void
  __quantify_forward_request(PriorityQueue& pq,
                             arc_writer& aw,
                             const typename Policy::pointer_type source,
                             const quantify_request<0>::target_t& target)
  {
    adiar_assert(!target.first().is_nil(),
                 "pointer_type::nil() should only ever end up being placed in target[1]");

    if (target.first().is_terminal()) {
      adiar_assert(target.second().is_nil(), "Operator should already be resolved at this point");
      aw.push({ source, target.first() });
    } else {
      pq.push({ target, {}, { source } });
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the `source` to the next in-going arc and returns `true`. Returns `false` if no
  ///        such source exists.
  ///
  /// \details This it to be used as a do-while loop conditional to resolve all ingoing arcs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename PriorityQueue_1, typename PriorityQueue_2>
  inline bool
  __quantify_update_source_or_break(PriorityQueue_1& pq_1,
                                    PriorityQueue_2& pq_2,
                                    ptr_uint64& source,
                                    const quantify_request<0>::target_t& target)
  {
    if (pq_1.can_pull() && pq_1.top().target == target) {
      source = pq_1.pull().data.source;
    } else if (!pq_2.empty() && pq_2.top().target == target) {
      source = pq_2.top().data.source;
      pq_2.pop();
    } else {
      return false;
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduces a request with (up to) `Targets` many values into its canonical form.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, size_t Targets>
  inline tuple<typename Policy::pointer_type, Targets, true>
  __quantify_resolve_request(const bool_op& op,
                             std::array<typename Policy::pointer_type, Targets> ts)
  {
    { // Sort array with pointers
      // TODO (optimisation): Abuse array is tiny with single-swap (2) / insertion-sort (3+)
      std::sort(ts.begin(), ts.end(), std::less<>());
    }

    // Remove duplicate pointers from array (abusing sorting). The policy may
    // also prune some terminals.
    size_t ts_max_idx = 0;
    for (size_t i = ts_max_idx + 1; i < ts.size(); ++i) {
      adiar_assert(ts_max_idx < i, "i is always ahead of 'ts_max_idx'");

      // Stop early at maximum value of 'nil'
      if (ts[i] == Policy::pointer_type::nil()) { break; }

      // Move new unique element at next spot for 'ts_max_idx'
      if (ts[ts_max_idx] != ts[i]
          && (!ts[i].is_terminal() || Policy::keep_terminal(op, ts[i]))) {
        ts[++ts_max_idx] = ts[i];
      }
    }
    for (size_t i = ts_max_idx + 1; i < ts.size(); ++i) {
      ts[i] = Policy::pointer_type::nil();
    }

    // Is the final element a collapsing terminal?
    const bool max_shortcuts =
      ts[ts_max_idx].is_terminal() && Policy::collapse_to_terminal(op, ts[ts_max_idx]);

    // Are there only terminals left (should be combined)
    const bool only_terminals = ts[0].is_terminal() /* sorted => ts[1].is_terminal() */;

    // If there are more than two targets but one of the two apply, then prune
    // it all the way down to a single target.
    if (1 <= ts_max_idx && (max_shortcuts || only_terminals)) {
      adiar_assert(!ts[1].is_nil(), "Cannot be nil at i <= ts_max_elem");
      ts[0] = max_shortcuts ? ts[ts_max_idx] : op(ts[0], ts[1]);
      for (size_t i = 1u; i <= ts_max_idx; ++i) { ts[i] = Policy::pointer_type::nil(); }
    }

    // Return final (sorted and pruned) set of targets.
    return ts;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Sweep logic with random access

  // TODO

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Sweep logic with a secondary priority queue

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Execution of a single quantification sweep with two priority queues.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            typename PriorityQueue_1,
            typename PriorityQueue_2,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __quantify_pq(const exec_policy& ep,
                const In& in,
                Policy& policy,
                const bool_op& op,
                PriorityQueue_1& pq_1,
                PriorityQueue_2& pq_2)
  {
    // TODO (optimisation):
    //   Merge 'op' into 'policy'.

    // Set up input

    // TODO (optimisation):
    //   Use '.seek(...)' with 'in_nodes' instead such that it only needs to be
    //   opened once in the caller of this function.
    NodeStream in_nodes(in);
    typename Policy::node_type v = in_nodes.pull();

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    // Process requests in topological order of both BDDs
    while (!pq_1.empty()) {
      adiar_assert(pq_2.empty(),
                   "Secondary priority queue is only non-empty while processing each level");

      // Set up level
      pq_1.setup_next_level();
      const typename Policy::label_type out_label = pq_1.current_level();
      typename Policy::id_type out_id             = 0;

      const bool should_quantify = policy.should_quantify(out_label);

      while (!pq_1.empty_level() || !pq_2.empty()) {
        // Merge requests from pq_1 and pq_2
        quantify_request<1> req;

        if (pq_1.can_pull()
            && (pq_2.empty() || pq_1.top().target.first() < pq_2.top().target.second())) {
          req = { pq_1.top().target,
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } },
                  pq_1.top().data };
        } else {
          req = pq_2.top();
        }

        // Seek element from request in stream
        const ptr_uint64 t_seek = req.empty_carry() ? req.target.first() : req.target.second();

        while (v.uid() < t_seek) { v = in_nodes.pull(); }

        // Forward information of node t1 across the level if needed
        if (req.empty_carry() && req.target.second().is_node()
            && req.target.first().label() == req.target.second().label()) {
          do {
            pq_2.push({ req.target, { v.children() }, pq_1.pull().data });
          } while (pq_1.can_pull() && pq_1.top().target == req.target);
          continue;
        }

        adiar_assert(req.target.first().label() == out_label,
                     "Level of requests always ought to match the one currently processed");

#ifdef ADIAR_STATS
        const int arity_idx = req.targets() - 1;
        stats_quantify.requests_unique[arity_idx] += 1;
#endif

        // -----------------------------------------------------------------------------------------
        // CASE: Quantification of Singleton f into (f[0], f[1]).
        if (should_quantify
            && (!Policy::partial_quantification || req.target.second().is_nil())) {
          quantify_request<0>::target_t rec =
            __quantify_resolve_request<Policy, 2>(op, v.children().data());

          while (__quantify_update_source_or_break(pq_1, pq_2, req.data.source, req.target)) {
#ifdef ADIAR_STATS
            stats_quantify.requests[arity_idx] += 1;
#endif
            __quantify_forward_request<Policy>(pq_1, aw, req.data.source, rec);
          }

          continue;
        }

        // Recreate children of the two targeted nodes (or possibly the
        // suppressed node for target.second()).
        const node::children_type children0 = req.empty_carry() ? v.children() : req.node_carry[0];

        const node::children_type children1 = req.target.second().level() == out_label
          ? v.children()
          : Policy::reduction_rule_inv(req.target.second());

        adiar_assert(out_id < Policy::max_id, "Has run out of ids");

        // -----------------------------------------------------------------------------------------
        // CASE: Partial Quantification of Tuple (f,g).
        if constexpr (Policy::partial_quantification) {
          if (should_quantify) {
            const tuple<typename Policy::pointer_type, 4, true> rec_all =
              __quantify_resolve_request<Policy, 4>(
                op, { children0[false], children0[true], children1[false], children1[true] });

            if (rec_all[2] == Policy::pointer_type::nil()) {
              // Collapsed to a terminal?
              if (req.data.source.is_nil() && rec_all[0].is_terminal()) {
                adiar_assert(rec_all[1] == Policy::pointer_type::nil(),
                             "Operator should already be applied");

                return typename Policy::dd_type(rec_all[0].value());
              }

              // No need to output a node as everything fits within a 2-tuple.
              quantify_request<0>::target_t rec(rec_all[0], rec_all[1]);

              while (__quantify_update_source_or_break(pq_1, pq_2, req.data.source, req.target)) {
#ifdef ADIAR_STATS
                stats_quantify.requests[arity_idx] += 1;
#endif
                __quantify_forward_request<Policy>(pq_1, aw, req.data.source, rec);
              }
            } else {
              // Store for later, that a node is yet to be done.
              policy.remaining_nodes++;

              // Output an intermediate node at this level to be quantified later.
              const node::uid_type out_uid(out_label, out_id++);

              quantify_request<0>::target_t rec0(rec_all[0], rec_all[1]);

              __quantify_forward_request<Policy>(
                pq_1, aw, out_uid.as_ptr(false), rec0);

              quantify_request<0>::target_t rec1(rec_all[2], rec_all[3]);

              __quantify_forward_request<Policy>(
                pq_1, aw, out_uid.as_ptr(true), rec1);

              while (__quantify_update_source_or_break(pq_1, pq_2, req.data.source, req.target)) {
#ifdef ADIAR_STATS
                stats_quantify.requests[arity_idx] += 1;
#endif
                if (!req.data.source.is_nil()) {
                  aw.push_internal(arc(req.data.source, out_uid));
                }
              }
            }
            continue;
          }
        }

        // -----------------------------------------------------------------------------------------
        // CASE: Regular Level
        //   The variable should stay: proceed as in the Product Construction by simulating both
        //   possibilities in parallel.

        // TODO (optimisation):
        //   Skip node creation due to Reduction Rule 1

        const node::uid_type out_uid(out_label, out_id++);

        quantify_request<0>::target_t rec0 = __quantify_resolve_request<Policy, 2>(
          op, { children0[false], children1[false] });

        __quantify_forward_request<Policy>(pq_1, aw, out_uid.as_ptr(false), rec0);

        quantify_request<0>::target_t rec1 =
          __quantify_resolve_request<Policy, 2>(op, { children0[true], children1[true] });
        __quantify_forward_request<Policy>(pq_1, aw, out_uid.as_ptr(true), rec1);

        while (__quantify_update_source_or_break(pq_1, pq_2, req.data.source, req.target)) {
#ifdef ADIAR_STATS
          stats_quantify.requests[arity_idx] += 1;
#endif
          if (!req.data.source.is_nil()) {
            aw.push_internal(arc(req.data.source, out_uid));
          }
        }
      }

      // Update meta information
      if (out_id > 0) { aw.push(level_info(out_label, out_id)); }

      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq_1.size());
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                          - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Set up of priority queues for a single top-down quantification sweep with two priority
  ///        queues.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            typename PriorityQueue_1,
            typename PriorityQueue_2,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __quantify_pq(const exec_policy& ep,
                const In& in,
                Policy& policy,
                const bool_op& op,
                const size_t pq_1_memory,
                const size_t max_pq_1_size,
                const size_t pq_2_memory,
                const size_t max_pq_2_size)
  {
    // Check for trivial terminal-only return on shortcutting the root
    typename Policy::node_type root;

    { // Detach and garbage collect node_stream<>
      NodeStream in_nodes(in);
      root = in_nodes.pull();

      if (policy.should_quantify(root.label())
          && (root.low().is_terminal() || root.high().is_terminal())) {
        typename Policy::pointer_type result = Policy::resolve_root(root, op);

        if (result != root.uid() && result.is_terminal()) {
          return typename Policy::dd_type(result.value());
        }
      }
    }

    // TODO: use 'result' above rather than 'root.uid()' for root request

    // Set up cross-level priority queue
    PriorityQueue_1 pq_1({ in }, pq_1_memory, max_pq_1_size, stats_quantify.lpq);
    pq_1.push({ { root.uid(), ptr_uint64::nil() }, {}, { ptr_uint64::nil() } });

    // Set up per-level priority queue
    PriorityQueue_2 pq_2(pq_2_memory, max_pq_2_size);

    return __quantify_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
      ep, in, policy, op, pq_1, pq_2);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common logic for a full single Quantificaiton sweep.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Memory computations to decide number and types of priority queues for a single
  ///        quantification sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            template <size_t, memory_mode> typename PriorityQueue_1_Template,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __quantify(const exec_policy& ep, const In& in, Policy& policy, const bool_op& op)
  {
    adiar_assert(is_commutative(op), "A commutative operator must be used");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory_available()
      // Input stream
      - NodeStream::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      quantify_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
      * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      quantify_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_1_bound =
      std::min({ __quantify_ilevel_upper_bound<Policy, get_2level_cut, 2u>(in, op),
                 __quantify_ilevel_upper_bound(in) });

    const size_t max_pq_1_size =
      internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound =
      __quantify_ilevel_upper_bound<Policy, get_1level_cut, 0u>(in, op);

    const size_t max_pq_2_size =
      internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.unbucketed += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<0, memory_mode::Internal>;
      using PriorityQueue_2 = quantify_priority_queue_2_t<memory_mode::Internal>;

      return __quantify_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
               && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.internal += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
      using PriorityQueue_2 = quantify_priority_queue_2_t<memory_mode::Internal>;

      return __quantify_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_quantify.lpq.external += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
      using PriorityQueue_2 = quantify_priority_queue_2_t<memory_mode::External>;

      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __quantify_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Single-top quantification sweep on node-based inputs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  __quantify(const exec_policy& ep,
             const typename Policy::dd_type& in,
             Policy& policy,
             const bool_op& op)
  {
    return __quantify<node_stream<>, quantify_priority_queue_1_node_t>(ep, in, policy, op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Single-top quantification sweep on arc-based inputs.
  ///
  /// \details This is used with repeated (partial) quantification.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  __quantify(const exec_policy& ep,
             const typename Policy::__dd_type& in,
             Policy& policy,
             const bool_op& op)
  {
    return __quantify<node_arc_stream<>, quantify_priority_queue_1_arc_t>(ep, in, policy, op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Single-variable Quantification

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for quantifying a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class single_quantify_policy : public Policy
  {
  private:
    const typename Policy::label_type _level;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    single_quantify_policy(typename Policy::label_type level)
      : _level(level)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Start product construction at the desired level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    should_quantify(typename Policy::label_type level) const
    {
      return this->_level == level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Disable logic for partial quantification during sweep.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Entry Point for Quantification algorithm for a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           const typename Policy::dd_type& in,
           const typename Policy::label_type label,
           const bool_op& op)
  {
    // Trivial cases, where there is no need to do any computation
    if (dd_isterminal(in) || !has_level(in, label)) {
#ifdef ADIAR_STATS
      stats_quantify.skipped += 1u;
#endif
      return in;
    }

#ifdef ADIAR_STATS
    stats_quantify.singleton_sweeps += 1u;
#endif

    // Set up policy and run sweep
    single_quantify_policy<Policy> policy(label);
    return __quantify(ep, in, policy, op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (common)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator to lift it for Nested Sweeping.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy : public Policy
  {
  private:
    const bool_op& _op;

  public:
    using request_t      = quantify_request<0>;
    using request_pred_t = request_data_first_lt<request_t>;

    template <size_t LookAhead, memory_mode MemoryMode>
    using pq_t = quantify_priority_queue_1_node_t<LookAhead, MemoryMode>;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    stream_memory()
    {
      return node_stream<>::memory_usage() + arc_writer::memory_usage();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    pq_memory(const size_t inner_memory)
    {
      constexpr size_t data_structures_in_pq_1 =
        quantify_priority_queue_1_node_t<ADIAR_LPQ_LOOKAHEAD,
                                         memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pq_2 =
        quantify_priority_queue_2_t<memory_mode::Internal>::data_structures;

      return (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
        * data_structures_in_pq_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    pq_bound(const typename Policy::shared_node_file_type& outer_file,
             const size_t /*outer_roots*/) const
    {
      const typename Policy::dd_type outer_wrapper(outer_file);
      return std::min(
        __quantify_ilevel_upper_bound<Policy, get_2level_cut, 2u>(outer_wrapper, _op),
        __quantify_ilevel_upper_bound(outer_wrapper));
    }

  public:
    multi_quantify_policy(const bool_op& op)
      : _op(op)
    {}

  public:
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Whether an inner nested sweep needs to quantify a given level.
    ///
    /// \details As an invariant, the Inner Sweep only ever touches levels beneath the deepest yet
    ///          to-be quantified level. Hence, we can provide an always-false predicate that can be
    ///          optimized by the compiler.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    constexpr bool should_quantify(typename Policy::label_type /*level*/) const
    {
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The partial quantification logic of the top-down sweep can be disabled.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = false;

    // bool has_sweep(typename Policy::label_type) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Entry Point for Nested Sweeping framework to start an Inner Sweep.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename PriorityQueue_1>
    typename Policy::__dd_type
    sweep_pq(const exec_policy& ep,
             const shared_levelized_file<node>& outer_file,
             PriorityQueue_1& pq_1,
             const size_t inner_remaining_memory) const
    {
      const size_t pq_2_memory_fits =
        quantify_priority_queue_2_t<memory_mode::Internal>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =
        // Obtain 1-level cut from subset
        __quantify_ilevel_upper_bound<Policy, get_1level_cut, 0u>(
          typename Policy::dd_type(outer_file), _op)
        // Add crossing arcs
        + (pq_1.size());

      const size_t max_pq_2_size =
        ep.template get<exec_policy::memory>() == exec_policy::memory::Internal
        ? std::min(pq_2_memory_fits, pq_2_bound)
        : pq_2_bound;

      if (ep.template get<exec_policy::memory>() != exec_policy::memory::External
          && max_pq_2_size <= pq_2_memory_fits) {
        using PriorityQueue_2 = quantify_priority_queue_2_t<memory_mode::Internal>;
        PriorityQueue_2 pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify_pq<node_stream<>, PriorityQueue_1, PriorityQueue_2>(
          ep, outer_file, *this, _op, pq_1, pq_2);
      } else {
        using PriorityQueue_2 = quantify_priority_queue_2_t<memory_mode::External>;
        PriorityQueue_2 pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify_pq<node_stream<>, PriorityQueue_1, PriorityQueue_2>(
          ep, outer_file, *this, _op, pq_1, pq_2);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Pick the type of a Priority Queue and algorithm.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename OuterRoots>
    typename Policy::__dd_type
    sweep(const exec_policy& ep,
          const shared_levelized_file<node>& outer_file,
          OuterRoots& outer_roots,
          const size_t inner_memory) const
    {
      return nested_sweeping::inner::down__sweep_switch(
        ep, *this, outer_file, outer_roots, inner_memory, stats_quantify.lpq);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Convert a node from the outer sweep on a to-be quantified level into a request.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    inline request_t
    request_from_node(const typename Policy::node_type& n,
                      const typename Policy::pointer_type& parent) const
    {
      // Shortcutting or Irrelevant terminal?
      const typename Policy::pointer_type result = Policy::resolve_root(n, _op);

      const bool shortcut = result != n.uid();

      typename request_t::target_t tgt = shortcut
        // If able to shortcut, preserve result.
        ? request_t::target_t{ result, Policy::pointer_type::nil() }
        // Otherwise, create product of children
        : request_t::target_t{ first(n.low(), n.high()), second(n.low(), n.high()) };

#ifdef ADIAR_STATS
      stats_quantify.nested_policy.shortcut_terminal +=
        static_cast<int>(shortcut && result.is_terminal());
      stats_quantify.nested_policy.shortcut_node += static_cast<int>(shortcut && result.is_node());
      stats_quantify.nested_policy.products += static_cast<int>(!shortcut);
#endif

      return request_t(tgt, {}, { parent });
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The final result from Nested Sweeping should be canonical.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool final_canonical = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (predicate)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Partial Quantification.
  ///
  /// \details This is to-be used as part of Repeated Quantification (during multi-variable
  ///          quantification before starting Nested Sweeping).
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class partial_quantify_policy : public Policy
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const pred_t& _pred;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of nodes left to-be quantified.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t remaining_nodes = 0;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    partial_quantify_policy(const pred_t& pred)
      : _pred(pred)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    should_quantify(typename Policy::label_type level) const
    {
      return _pred(level) == Policy::quantify_onset;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    reset()
    {
      remaining_nodes = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Enable partial quantification logic.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Nested Sweeping with a Predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy__pred : public multi_quantify_policy<Policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const pred_t& _pred;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__pred(const bool_op& op, const pred_t& pred)
      : multi_quantify_policy<Policy>(op)
      , _pred(pred)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_sweep(node::pointer_type::label_type l)
    {
      return _pred(l) == Policy::quantify_onset;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the deepest lvel that satisfies (or not) the requested level.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  template <typename Policy>
  inline typename Policy::label_type
  quantify__get_deepest(const typename Policy::dd_type& dd,
                        const predicate<typename Policy::label_type>& pred)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename Policy::label_type l = lis.pull().label();
      if (pred(l) == Policy::quantify_onset) { return l; }
    }
    return Policy::max_label + 1;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Entry Point for Multi-variable Quantification with a Predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::dd_type dd,
           const predicate<typename Policy::label_type>& pred,
           const bool_op& op)
  {
    using unreduced_t = typename Policy::__dd_type;
    // TODO: check for missing std::move(...)

    typename Policy::label_type label = quantify__get_deepest<Policy>(dd, pred);

    if (Policy::max_label < label) {
#ifdef ADIAR_STATS
      stats_quantify.skipped += 1u;
#endif
      return dd;
    }

    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // ---------------------------------------------------------------------
      // Case: Repeated single variable quantification
#ifdef ADIAR_STATS
      stats_quantify.singleton_sweeps += 1u;
#endif
      while (label <= Policy::max_label) {
        dd = quantify<Policy>(ep, dd, label, op);
        if (dd_isterminal(dd)) { return dd; }

        label = quantify__get_deepest<Policy>(dd, pred);
      }
      return dd;
    }

    case exec_policy::quantify::Nested: {
      // ---------------------------------------------------------------------
      // Case: Nested Sweeping
      const size_t dd_size = dd.size();

      // Do Partial Quantification as long as...
      //   1. ... it stays smaller than 1+epsilon of the input size.
      const size_t transposition__size_threshold = (std::min(
        static_cast<double>(std::numeric_limits<size_t>::max() / 2u),
        static_cast<double>(ep.template get<exec_policy::quantify::transposition_growth>())
          * static_cast<double>(dd_size)));

      //   2. ... it has not run more than the maximum number of iterations.
      const size_t transposition__max_iterations =
        ep.template get<exec_policy::quantify::transposition_max>();

      unreduced_t transposed;

      // If transposition__max_iterations is 0, then only quantify the lowest level.
      if (transposition__max_iterations == 0) {
        // Singleton Quantification of bottom-most level
#ifdef ADIAR_STATS
        stats_quantify.singleton_sweeps += 1u;
#endif
        transposed = quantify<Policy>(ep, std::move(dd), label, op);
      } else {
        // Partial Quantification
#ifdef ADIAR_STATS
        stats_quantify.partial_sweeps += 1u;
#endif
        partial_quantify_policy<Policy> partial_impl(pred);
        transposed = __quantify(ep, std::move(dd), partial_impl, op);

        if (partial_impl.remaining_nodes == 0) {
#ifdef ADIAR_STATS
          stats_quantify.partial_termination += 1u;
#endif
          return transposed;
        }

        for (size_t i = 1; i < transposition__max_iterations; ++i) {
          if (transposition__size_threshold < transposed.size()) { break; }

          // Reset policy and rerun partial quantification
          partial_impl.reset();

#ifdef ADIAR_STATS
          stats_quantify.partial_sweeps += 1u;
#endif
          transposed = __quantify(ep, transposed, partial_impl, op);

          // Reduce result, if no work is left to be done.
          if (partial_impl.remaining_nodes == 0) {
#ifdef ADIAR_STATS
            stats_quantify.partial_termination += 1u;
#endif
            return transposed;
          }
        }
      }
      { // Nested Sweeping
#ifdef ADIAR_STATS
        stats_quantify.nested_sweeps += 1u;
#endif
        multi_quantify_policy__pred<Policy> inner_impl(op, pred);
        return nested_sweep<>(ep, std::move(transposed), inner_impl);
      }
    }

      // LCOV_EXCL_START
    default:
      // ---------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (descending generator)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Nested Sweeping with a Generator
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy__generator : public multi_quantify_policy<Policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of the levels to sweep on (or not to sweep on) in descending order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using generator_t = generator<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of levels to sweep on (or not to sweep on) in descending order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const generator_t& _lvls;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer for to hold onto the generated next level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    optional<typename Policy::label_type> _next_level;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__generator(const bool_op& op, const generator_t& g)
      : multi_quantify_policy<Policy>(op)
      , _lvls(g)
    {
      _next_level = _lvls();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to do a Nested Sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_sweep(const typename Policy::label_type l)
    {
      return l == next_level(l) ? Policy::quantify_onset
                                : !Policy::quantify_onset;
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are more levels for the Nested Sweping framework.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_level() const
    {
      return _next_level.has_value();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The next level to start a Nested Sweep.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    typename Policy::label_type
    next_level(const typename Policy::label_type l)
    {
      while (_next_level.has_value() && l < _next_level.value()) { _next_level = _lvls(); }
      return _next_level.value_or(Policy::max_label + 1);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the deepest variable between `bot_level` and `top_level`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  //       clean up
  //       - Make return type 'optional' rather than larger than 'max_label'
  template <typename Policy>
  inline typename Policy::label_type
  quantify__get_deepest(const typename Policy::dd_type& dd,
                        const typename Policy::label_type bot_level,
                        const optional<typename Policy::label_type> top_level)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename Policy::label_type l = lis.pull().label();
      if ((!top_level || top_level.value() < l) && l < bot_level) { return l; }
    }
    return Policy::max_label + 1;
  }

  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::dd_type dd,
           const typename multi_quantify_policy__generator<Policy>::generator_t& lvls,
           const bool_op& op)
  {
    adiar_assert(is_commutative(op), "Operator must be commutative");

    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // ---------------------------------------------------------------------------------------
      // Case: Repeated single variable quantification
      // TODO: correctly handle Policy::quantify_onset
      optional<typename Policy::label_type> on_level = lvls();

      if (Policy::quantify_onset) {
        if (!on_level) {
#ifdef ADIAR_STATS
          stats_quantify.skipped += 1u;
#endif
          return dd;
        }

        // Quantify all but the last 'on_level'. Hence, look one ahead with
        // 'next_on_level' to see whether it is the last one.
        optional<typename Policy::label_type> next_on_level = lvls();
        while (next_on_level) {
#ifdef ADIAR_STATS
          stats_quantify.singleton_sweeps += 1u;
#endif
          dd = quantify<Policy>(ep, dd, on_level.value(), op);
          if (dd_isterminal(dd)) { return dd; }

          on_level      = next_on_level;
          next_on_level = lvls();
        }
        return quantify<Policy>(ep, dd, on_level.value(), op);
      } else { // !Policy::quantify_onset
        // TODO: only designed for 'OR' at this point in time
        if (!on_level) {
          return typename Policy::dd_type(dd->number_of_terminals[true] > 0);
        }

        // Quantify everything below 'label'
        for (;;) {
          const typename Policy::label_type off_level =
            quantify__get_deepest<Policy>(
              dd, Policy::max_label, on_level.value());

          if (Policy::max_label < off_level) { break; }

#ifdef ADIAR_STATS
          stats_quantify.singleton_sweeps += 1u;
#endif
          dd = quantify<Policy>(ep, dd, off_level, op);
          if (dd_isterminal(dd)) { return dd; }
        }

        // Quantify everything strictly in between 'bot_level' and 'top_level'
        optional<typename Policy::label_type> bot_level = on_level;
        optional<typename Policy::label_type> top_level = lvls();

        while (bot_level) {
          for (;;) {
            const typename Policy::label_type off_level =
              quantify__get_deepest<Policy>(dd, bot_level.value(), top_level);

            if (Policy::max_label < off_level) { break; }

#ifdef ADIAR_STATS
            stats_quantify.singleton_sweeps += 1u;
#endif
            dd = quantify<Policy>(ep, dd, off_level, op);
            if (dd_isterminal(dd)) { return dd; }
          }

          bot_level = top_level;
          top_level = lvls();
        }
        return dd;
      }
    }

    case exec_policy::quantify::Nested: {
      // -----------------------------------------------------------------------------------------
      // Case: Nested Sweeping
      //
      // NOTE: read-once access with 'gen' makes repeated transposition not possible. Yet, despite
      //       of this, we can (assuming we have to quantify the on-set) still use the bottom-most
      //       level to transpose the DAG.
      if constexpr (Policy::quantify_onset) {
        // Obtain the bottom-most onset level that exists in the diagram.
        // TODO: Move into helper function.

        optional<typename Policy::label_type> transposition_level = lvls();
        if (!transposition_level) {
#ifdef ADIAR_STATS
          stats_quantify.skipped += 1u;
#endif
          return dd;
        }

        {
          level_info_stream<true> in_meta(dd);
          typename Policy::label_type dd_level = in_meta.pull().level();

          for (;;) {
            // Go forward in the diagram's levels, until we are at or above
            // the current candidate
            while (in_meta.can_pull() && transposition_level.value() < dd_level) {
              dd_level = in_meta.pull().level();
            }
            // There is no onset level in the diagram? If so, then nothing is
            // going to change and we may just return the input.
            if (!in_meta.can_pull() && transposition_level.value() < dd_level) {
#ifdef ADIAR_STATS
              stats_quantify.skipped += 1u;
#endif
              return dd;
            }

            adiar_assert(dd_level <= transposition_level.value(),
                         "Must be at or above candidate level");

            // Did we find the current candidate or skipped past it?
            if (dd_level == transposition_level.value()) {
              break;
            } else { // dd_level < transposition_level
              transposition_level = lvls();

              // Did we run out of 'onset' levels?
              if (!transposition_level) {
#ifdef ADIAR_STATS
                stats_quantify.skipped += 1u;
#endif
                return dd;
              }
            }
          }
        }
        adiar_assert(transposition_level.has_value());

        // Quantify the 'transposition_level' as part of the initial transposition step
#ifdef ADIAR_STATS
        stats_quantify.singleton_sweeps += 1u;
#endif
        typename Policy::__dd_type transposed =
          quantify<Policy>(ep, dd, transposition_level.value(), op);

#ifdef ADIAR_STATS
        stats_quantify.nested_sweeps += 1u;
#endif
        multi_quantify_policy__generator<Policy> inner_impl(op, lvls);
        return nested_sweep<>(ep, std::move(transposed), inner_impl);
      } else { // !Policy::quantify_onset
#ifdef ADIAR_STATS
        stats_quantify.nested_sweeps += 1u;
#endif
        multi_quantify_policy__generator<Policy> inner_impl(op, lvls);
        return nested_sweep<>(ep, dd, inner_impl);
      }
    }

      // LCOV_EXCL_START
    default:
      // -----------------------------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
