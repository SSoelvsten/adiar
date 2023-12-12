#ifndef ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
#define ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H

#include <algorithm>
#include <variant>

#include <adiar/exec_policy.h>
#include <adiar/functional.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/unreachable.h>
#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/io/node_stream.h>
#include <adiar/internal/io/node_arc_stream.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template<uint8_t nodes_carried>
  using quantify_request = request_data<2, with_parent, nodes_carried, 1>;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

  template<size_t look_ahead, memory_mode mem_mode>
  using quantify_priority_queue_1_node_t =
    levelized_node_priority_queue<quantify_request<0>,
                                  request_data_first_lt<quantify_request<0>>,
                                  look_ahead,
                                  mem_mode,
                                  1,
                                  0>;

  template<size_t look_ahead, memory_mode mem_mode>
  using quantify_priority_queue_1_arc_t =
    levelized_node_arc_priority_queue<quantify_request<0>,
                                      request_data_first_lt<quantify_request<0>>,
                                      look_ahead,
                                      mem_mode,
                                      1,
                                      0>;

  template<memory_mode mem_mode>
  using quantify_priority_queue_2_t =
    priority_queue<mem_mode, quantify_request<1>, request_data_second_lt<quantify_request<1>>>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename quantify_policy, typename pq_1_t>
  inline void __quantify_forward_request(pq_1_t &quantify_pq_1,
                                         arc_writer &aw,
                                         const typename quantify_policy::pointer_type source,
                                         const quantify_request<0>::target_t &target)
  {
    adiar_assert(!target.first().is_nil(),
                 "pointer_type::nil() should only ever end up being placed in target[1]");

    if (target.first().is_terminal()) {
      adiar_assert(target.second().is_nil(),
                   "Operator should already be resolved at this point");
      aw.push({source, target.first()});
    } else {
      quantify_pq_1.push({target, {}, {source}});
    }
  }

  template<typename pq_1_t, typename pq_2_t>
  inline bool
  __quantify_update_source_or_break(pq_1_t &quantify_pq_1,
                                    pq_2_t &quantify_pq_2,
                                    ptr_uint64 &source,
                                    const quantify_request<0>::target_t &target)
  {
    if (quantify_pq_1.can_pull()
        && quantify_pq_1.top().target == target) {
      source = quantify_pq_1.pull().data.source;
    } else if (!quantify_pq_2.empty()
               && quantify_pq_2.top().target == target) {
      source = quantify_pq_2.top().data.source;
      quantify_pq_2.pop();
    } else {
      return true;
    }
    return false;
  }

  template<typename quantify_policy, size_t targets>
  inline tuple<typename quantify_policy::pointer_type, targets, true>
  __quantify_resolve_request(const bool_op &op,
                             std::array<typename quantify_policy::pointer_type, targets> ts)
  {
    { // Sort array with pointers
      // TODO (optimisation): Abuse array is tiny with single-swap (2) / insertion-sort (3+)
      std::sort(ts.begin(), ts.end(), std::less<>());
    }

    // Remove duplicate pointers from array (abusing sorting). The policy may
    // also prune some terminals.
    size_t ts_max_idx = 0;
    for (size_t i = ts_max_idx+1; i < ts.size(); ++i) {
      adiar_assert(ts_max_idx < i, "i is always ahead of 'ts_max_idx'");

      // Stop early at maximum value of 'nil'
      if (ts[i] == quantify_policy::pointer_type::nil()) { break; }

      // Move new unique element at next spot for 'ts_max_idx'
      if (ts[ts_max_idx] != ts[i] &&
          (!ts[i].is_terminal() || quantify_policy::keep_terminal(op, ts[i]))) {
        ts[++ts_max_idx] = ts[i];
      }
    }
    for (size_t i = ts_max_idx+1; i < ts.size(); ++i) {
      ts[i] = quantify_policy::pointer_type::nil();
    }

    // Is the final element a collapsing terminal?
    const bool max_shortcuts =
      ts[ts_max_idx].is_terminal() && quantify_policy::collapse_to_terminal(op, ts[ts_max_idx]);

    // Are there only terminals left (should be combined)
    const bool only_terminals =
      ts[0].is_terminal() /* sorted => ts[1].is_terminal() */;

    // If there are more than two targets but one of the two apply, then prune
    // it all the way down to a single target.
    if (1 <= ts_max_idx && (max_shortcuts || only_terminals)) {
      adiar_assert(!ts[1].is_nil(), "Cannot be nil at i <= ts_max_elem");
      ts[0] = max_shortcuts ? ts[ts_max_idx] : op(ts[0], ts[1]);
      for (size_t i = 1u; i <= ts_max_idx; ++i) {
        ts[i] = quantify_policy::pointer_type::nil();
      }
    }

    // Return final (sorted and pruned) set of targets.
    return ts;
  }

  template<typename node_stream_t, typename pq_1_t, typename pq_2_t, typename quantify_policy, typename in_t>
  typename quantify_policy::__dd_type
  __quantify(const exec_policy &ep,
             const in_t &in,
             quantify_policy &policy_impl,
             const bool_op &op,
             pq_1_t &quantify_pq_1,
             pq_2_t &quantify_pq_2)
  {
    // TODO (optimisation):
    //   Merge 'op' into 'policy_impl'.

    // Set up input

    // TODO (optimisation):
    //   Use '.seek(...)' with 'in_nodes' instead such that it only needs to be
    //   opened once in the caller of this function.
    node_stream_t in_nodes(in);
    typename quantify_policy::node_type v = in_nodes.pull();

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    // Process requests in topological order of both BDDs
    while(!quantify_pq_1.empty()) {
      adiar_assert(quantify_pq_2.empty(),
                   "Secondary priority queue is only non-empty while processing each level");

      // Set up level
      quantify_pq_1.setup_next_level();
      const typename quantify_policy::label_type out_label = quantify_pq_1.current_level();
      typename quantify_policy::id_type out_id = 0;

      const bool should_quantify = policy_impl.should_quantify(out_label);

      while (!quantify_pq_1.empty_level() || !quantify_pq_2.empty()) {
        // Merge requests from quantify_pq_1 and quantify_pq_2
        quantify_request<1> req;

        if (quantify_pq_1.can_pull()
            && (quantify_pq_2.empty() || quantify_pq_1.top().target.first() < quantify_pq_2.top().target.second())) {
          req = { quantify_pq_1.top().target,
                  {{ { node::pointer_type::nil(), node::pointer_type::nil() } }},
                  quantify_pq_1.top().data };
          quantify_pq_1.pop();
        } else {
          req = quantify_pq_2.top();
          quantify_pq_2.pop();
        }

        // Seek element from request in stream
        const ptr_uint64 t_seek = req.empty_carry() ? req.target.first() : req.target.second();

        while (v.uid() < t_seek) {
          v = in_nodes.pull();
        }

        // Forward information of node t1 across the level if needed
        if (req.empty_carry()
            && req.target.second().is_node()
            && req.target.first().label() == req.target.second().label()) {
          adiar_assert(!req.target.second().is_nil(),
                       "req.target.second().is_node() ==> !req.target.second().is_nil()");

          quantify_pq_2.push({ req.target, {v.children()}, req.data });

          while (quantify_pq_1.can_pull() && (quantify_pq_1.top().target == req.target)) {
            quantify_pq_2.push({ req.target, {v.children()}, quantify_pq_1.pull().data });
          }

          continue;
        }

        adiar_assert(req.target.first().label() == out_label,
                     "Level of requests always ought to match the one currently processed");

        // ---------------------------------------------------------------------
        // CASE: Quantification of Singleton f into (f[0], f[1]).
        if (should_quantify &&
            (!quantify_policy::partial_quantification || req.target.second().is_nil())) {
          quantify_request<0>::target_t rec =
            __quantify_resolve_request<quantify_policy, 2>(op, v.children().data());

          do {
            __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, req.data.source, rec);
          } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                      req.data.source,
                                                      req.target));

          continue;
        }

        // Recreate children of the two targeted nodes (or possibly the
        // suppressed node for target.second()).
        const node::children_type children0 =
          req.empty_carry() ? v.children() : req.node_carry[0];

        const node::children_type children1 =
          req.target.second().level() == out_label
          ? v.children()
          : quantify_policy::reduction_rule_inv(req.target.second());

        adiar_assert(out_id < quantify_policy::max_id, "Has run out of ids");

        // ---------------------------------------------------------------------
        // CASE: Partial Quantification of Tuple (f,g).
        if constexpr (quantify_policy::partial_quantification) {
          if (should_quantify) {
            const tuple<typename quantify_policy::pointer_type, 4, true> rec_all =
              __quantify_resolve_request<quantify_policy, 4>(op, {children0[false], children0[true],
                                                                  children1[false], children1[true]});

            if (rec_all[2] == quantify_policy::pointer_type::nil()) {
              // Collapsed to a terminal?
              if (req.data.source.is_nil() && rec_all[0].is_terminal()) {
                adiar_assert(rec_all[1] == quantify_policy::pointer_type::nil(),
                             "Operator should already be applied");

                return typename quantify_policy::dd_type(rec_all[0].value());
              }

              // No need to output a node as everything fits within a 2-tuple.
              quantify_request<0>::target_t rec(rec_all[0], rec_all[1]);

              do {
                __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, req.data.source, rec);
              } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                          req.data.source,
                                                          req.target));
            } else {
              // Store for later, that a node is yet to be done.
              policy_impl.remaining_nodes++;

              // Output an intermediate node at this level to be quantified later.
              const node::uid_type out_uid(out_label, out_id++);

              quantify_request<0>::target_t rec0(rec_all[0], rec_all[1]);

              __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, out_uid.as_ptr(false), rec0);

              quantify_request<0>::target_t rec1(rec_all[2], rec_all[3]);

              __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, out_uid.as_ptr(true), rec1);

              if (!req.data.source.is_nil()) {
                do {
                  aw.push_internal(arc(req.data.source, out_uid));
                } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                            req.data.source,
                                                            req.target));
              }
            }
            continue;
          }
        }

        // ---------------------------------------------------------------------
        // CASE: Regular Level
        //   The variable should stay: proceed as in the Product Construction
        //   by simulating both possibilities in parallel.

        // TODO (optimisation):
        //   Skip node creation due to Reduction Rule 1

        const node::uid_type out_uid(out_label, out_id++);

        quantify_request<0>::target_t rec0 =
          __quantify_resolve_request<quantify_policy, 2>(op, {children0[false], children1[false]});

        __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, out_uid.as_ptr(false), rec0);

        quantify_request<0>::target_t rec1 =
          __quantify_resolve_request<quantify_policy, 2>(op, {children0[true], children1[true]});
        __quantify_forward_request<quantify_policy>(quantify_pq_1, aw, out_uid.as_ptr(true), rec1);

        if (!req.data.source.is_nil()) {
          do {
            aw.push_internal(arc(req.data.source, out_uid));
          } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                      req.data.source,
                                                      req.target));
        }
      }

      // Update meta information
      if (out_id > 0) {
        aw.push(level_info(out_label, out_id));
      }

      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, quantify_pq_1.size());
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                                  - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename quantify_policy::__dd_type(out_arcs, ep);
  }


  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its
  /// maximum i-level cut.
  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy, typename cut, size_t const_size_inc, typename in_t>
  size_t __quantify_ilevel_upper_bound(const in_t &in,
                                       const bool_op &op)
  {
    const typename cut::type ct_internal  = cut::type::Internal;
    const typename cut::type ct_terminals = quantify_policy::cut_with_terminals(op);

    const safe_size_t max_cut_internal  = cut::get(in, ct_internal);
    const safe_size_t max_cut_terminals = cut::get(in, ct_terminals);

    return to_size(max_cut_internal * max_cut_terminals + const_size_inc);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its size.
  //////////////////////////////////////////////////////////////////////////////
  template<typename in_t>
  size_t __quantify_ilevel_upper_bound(const in_t &in)
  {
    const safe_size_t in_size = in.size();
    return to_size(in_size * in_size + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename node_stream_t, typename pq_1_t, typename pq_2_t, typename quantify_policy, typename in_t>
  typename quantify_policy::__dd_type
  __quantify(const exec_policy &ep,
             const in_t &in,
             quantify_policy &policy_impl,
             const bool_op &op,
             const size_t pq_1_memory, const size_t max_pq_1_size,
             const size_t pq_2_memory, const size_t max_pq_2_size)
  {
    // Check for trivial terminal-only return on shortcutting the root
    typename quantify_policy::node_type root;

    { // Detach and garbage collect node_stream<>
      node_stream_t in_nodes(in);
      root = in_nodes.pull();

      if (policy_impl.should_quantify(root.label()) && (root.low().is_terminal() || root.high().is_terminal())) {
        typename quantify_policy::pointer_type result = quantify_policy::resolve_root(root, op);

        if (result != root.uid() && result.is_terminal()) {
          return typename quantify_policy::dd_type(result.value());
        }
      }
    }

    // TODO: use 'result' above rather than 'root.uid()' for root request

    // Set up cross-level priority queue
    pq_1_t quantify_pq_1({in}, pq_1_memory, max_pq_1_size, stats_quantify.lpq);
    quantify_pq_1.push({ { root.uid(), ptr_uint64::nil() }, {}, {ptr_uint64::nil()} });

    // Set up per-level priority queue
    pq_2_t quantify_pq_2(pq_2_memory, max_pq_2_size);

    return __quantify<node_stream_t, pq_1_t, pq_2_t>
      (ep, in, policy_impl, op, quantify_pq_1, quantify_pq_2);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename node_stream_t,
           template<size_t, memory_mode> typename pq_1_template,
           typename quantify_policy, typename in_t>
  typename quantify_policy::__dd_type
  __quantify(const exec_policy &ep,
             const in_t &in,
             quantify_policy &policy_impl,
             const bool_op &op)
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
      - node_stream_t::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      pq_1_template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      quantify_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      pq_1_template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_internal_memory =
      aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      quantify_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const bool internal_only = ep.memory_mode() == exec_policy::memory::Internal;
    const bool external_only = ep.memory_mode() == exec_policy::memory::External;

    const size_t pq_1_bound = std::min({__quantify_ilevel_upper_bound<quantify_policy, get_2level_cut, 2u>(in,op),
                                        __quantify_ilevel_upper_bound(in)});

    const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __quantify_ilevel_upper_bound<quantify_policy, get_1level_cut, 0u>(in,op);

    const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if(!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.unbucketed += 1u;
#endif
      return __quantify<node_stream_t,
                        pq_1_template<0, memory_mode::Internal>,
                        quantify_priority_queue_2_t<memory_mode::Internal>>
        (ep, in, policy_impl, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if(!external_only && max_pq_1_size <= pq_1_memory_fits
                             && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.internal += 1u;
#endif
      return __quantify<node_stream_t,
                        pq_1_template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>,
                        quantify_priority_queue_2_t<memory_mode::Internal>>
        (ep, in, policy_impl, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_quantify.lpq.external += 1u;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __quantify<node_stream_t,
                        pq_1_template<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>,
                        quantify_priority_queue_2_t<memory_mode::External>>
        (ep, in, policy_impl, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy>
  typename quantify_policy::__dd_type
  __quantify(const exec_policy &ep,
             const typename quantify_policy::dd_type &in,
             quantify_policy &policy_impl,
             const bool_op &op)
  {
    return __quantify<node_stream<>, quantify_priority_queue_1_node_t>(ep, in, policy_impl, op);
  }

  template<typename quantify_policy>
  typename quantify_policy::__dd_type
  __quantify(const exec_policy &ep,
             const typename quantify_policy::__dd_type &in,
             quantify_policy &policy_impl,
             const bool_op &op)
  {
    return __quantify<node_arc_stream<>, quantify_priority_queue_1_arc_t>(ep, in, policy_impl, op);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Single-variable
  template<typename quantify_policy>
  class single_quantify_policy : public quantify_policy
  {
  private:
    const typename quantify_policy::label_type _level;

  public:
    ////////////////////////////////////////////////////////////////////////////
    single_quantify_policy(typename quantify_policy::label_type level)
      : _level(level)
    { }

    ////////////////////////////////////////////////////////////////////////////
    inline bool
    should_quantify(typename quantify_policy::label_type level) const
    { return _level == level; }

    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = false;
  };

  template<typename quantify_policy>
  typename quantify_policy::__dd_type
  quantify(const exec_policy &ep,
           const typename quantify_policy::dd_type &in,
           const typename quantify_policy::label_type label,
           const bool_op &op)
  {

    // Trivial cases, where there is no need to do any computation
    if (dd_isterminal(in) || !has_level(in, label)) {
      return in;
    }

    // Set up policy and run sweep
    single_quantify_policy<quantify_policy> policy_impl(label);
    return __quantify(ep, in, policy_impl, op);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Multi-variable (common)
  template<typename quantify_policy>
  class multi_quantify_policy : public quantify_policy
  {
  private:
    const bool_op &_op;

  public:
    using request_t = quantify_request<0>;
    using request_pred_t = request_data_first_lt<request_t>;

    template<size_t look_ahead, memory_mode mem_mode>
    using pq_t = quantify_priority_queue_1_node_t<look_ahead, mem_mode>;

  public:
    ////////////////////////////////////////////////////////////////////////////
    static size_t stream_memory()
    { return node_stream<>::memory_usage() + arc_writer::memory_usage(); }

    ////////////////////////////////////////////////////////////////////////////
    static size_t pq_memory(const size_t inner_memory)
    {
      constexpr size_t data_structures_in_pq_1 =
        quantify_priority_queue_1_node_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pq_2 =
        quantify_priority_queue_2_t<memory_mode::Internal>::data_structures;

      return  (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t pq_bound(const typename quantify_policy::shared_node_file_type &outer_file,
                    const size_t /*outer_roots*/) const
    {
      const typename quantify_policy::dd_type outer_wrapper(outer_file);
      return std::min(__quantify_ilevel_upper_bound<quantify_policy, get_2level_cut, 2u>(outer_wrapper, _op),
                      __quantify_ilevel_upper_bound(outer_wrapper));
    }

  public:
    multi_quantify_policy(const bool_op &op)
      : _op(op)
    { }

  public:
    //////////////////////////////////////////////////////////////////////////////
    /// As an invariant, the Inner Sweep only ever touches levels beneath the
    /// deepest yet to-be quantified level. Hence, we can provide an
    /// always-false predicate that is immediate to the compiler.
    //////////////////////////////////////////////////////////////////////////////
    constexpr bool
    should_quantify(typename quantify_policy::label_type /*level*/) const
    { return false; }

    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = false;

    // bool has_sweep(typename quantify_policy::label_type) const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Run Sweep with given priority queue.
    ////////////////////////////////////////////////////////////////////////////
    template<typename inner_pq_1_t>
    typename quantify_policy::__dd_type
    sweep_pq(const exec_policy &ep,
             const shared_levelized_file<node> &outer_file,
             inner_pq_1_t &inner_pq_1,
             const size_t inner_remaining_memory) const
    {
      const size_t pq_2_memory_fits =
        quantify_priority_queue_2_t<memory_mode::Internal>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =
        // Obtain 1-level cut from subset
        __quantify_ilevel_upper_bound<quantify_policy, get_1level_cut, 0u>
          (typename quantify_policy::dd_type(outer_file), _op)
        // Add crossing arcs
        + (inner_pq_1.size());

      const size_t max_pq_2_size = ep.memory_mode() == exec_policy::memory::Internal
        ? std::min(pq_2_memory_fits, pq_2_bound)
        : pq_2_bound;

      if(ep.memory_mode() != exec_policy::memory::External && max_pq_2_size <= pq_2_memory_fits) {
        using inner_pq_2_t = quantify_priority_queue_2_t<memory_mode::Internal>;
        inner_pq_2_t inner_pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify<node_stream<>, inner_pq_1_t, inner_pq_2_t>
          (ep, outer_file, *this, _op, inner_pq_1, inner_pq_2);
      } else {
        using inner_pq_2_t = quantify_priority_queue_2_t<memory_mode::External>;
        inner_pq_2_t inner_pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify<node_stream<>, inner_pq_1_t, inner_pq_2_t>
          (ep, outer_file, *this, _op, inner_pq_1, inner_pq_2);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Pick PQ type and Run Sweep.
    ////////////////////////////////////////////////////////////////////////////
    template<typename outer_roots_t>
    typename quantify_policy::__dd_type
    sweep(const exec_policy &ep,
          const shared_levelized_file<node> &outer_file,
          outer_roots_t &outer_roots,
          const size_t inner_memory) const
    {
      return nested_sweeping::inner::down__sweep_switch
        (ep, *this, outer_file, outer_roots, inner_memory);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Create request
    //////////////////////////////////////////////////////////////////////////////
    inline request_t
    request_from_node(const typename quantify_policy::node_type &n,
                      const typename quantify_policy::pointer_type &parent) const
    {
      // Shortcutting or Irrelevant terminal?
      const typename quantify_policy::pointer_type result =
        quantify_policy::resolve_root(n, _op);

      typename request_t::target_t tgt = result != n.uid()
        // If able to shortcut, preserve result.
        ? request_t::target_t{ result, quantify_policy::pointer_type::nil() }
        // Otherwise, create product of children
        : request_t::target_t{ first(n.low(), n.high()), second(n.low(), n.high()) };

      return request_t(tgt, {}, {parent});
    }

    ////////////////////////////////////////////////////////////////////////////
    static constexpr internal::nested_sweeping::reduce_strategy reduce_strategy =
      internal::nested_sweeping::Auto;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Multi-variable (predicate)
  template<typename quantify_policy>
  class partial_quantify_policy
    : public quantify_policy
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename quantify_policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////
    const pred_t &_pred;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of nodes left to-be quantified.
    ////////////////////////////////////////////////////////////////////////////
    size_t remaining_nodes = 0;

  public:
    ////////////////////////////////////////////////////////////////////////////
    partial_quantify_policy(const pred_t &pred)
      : _pred(pred)
    { }

    ////////////////////////////////////////////////////////////////////////////
    inline bool
    should_quantify(typename quantify_policy::label_type level) const
    { return _pred(level) == quantify_policy::quantify_onset; }

    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      remaining_nodes = 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool partial_quantification = true;
  };

  template<typename quantify_policy>
  class multi_quantify_policy__pred
    : public multi_quantify_policy<quantify_policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename quantify_policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////
    const pred_t &_pred;

  public:
    ////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__pred(const bool_op &op, const pred_t &pred)
      : multi_quantify_policy<quantify_policy>(op), _pred(pred)
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////
    bool has_sweep(node::pointer_type::label_type l)
    {
      return _pred(l) == quantify_policy::quantify_onset;
    }
  };

  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  template<typename quantify_policy>
  inline typename quantify_policy::label_type
  quantify__get_deepest(const typename quantify_policy::dd_type &dd,
                        const predicate<typename quantify_policy::label_type> &pred)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename quantify_policy::label_type l = lis.pull().label();
      if (pred(l) == quantify_policy::quantify_onset) { return l; }
    }
    return quantify_policy::max_label+1;
  }

  template<typename quantify_policy>
  inline typename quantify_policy::label_type
  quantify__count_above_widest(const typename quantify_policy::dd_type &dd,
                               const predicate<typename quantify_policy::label_type> &pred)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    size_t acc = 0u;
    bool start_count = false;
    while (lis.can_pull()) {
      const level_info li = lis.pull();
      start_count |= li.width() == dd->width;
      acc += start_count && pred(li.label()) == quantify_policy::quantify_onset;
    }
    return acc;
  }

  template<typename quantify_policy>
  typename quantify_policy::__dd_type
  quantify(const exec_policy &ep,
           typename quantify_policy::dd_type dd,
           const predicate<typename quantify_policy::label_type> &pred,
           const bool_op &op)
  {
    using unreduced_t = typename quantify_policy::__dd_type;
    // TODO: check for missing std::move(...)

    typename quantify_policy::label_type label = quantify__get_deepest<quantify_policy>(dd, pred);

    if (quantify_policy::max_label < label) {
#ifdef ADIAR_STATS
      // TODO
#endif
      return dd;
    }

    switch (ep.quantify_alg()) {
    case exec_policy::quantify::Partial:
      { // ---------------------------------------------------------------------
        // Case: Repeated partial quantification
#ifdef ADIAR_STATS
        // TODO
#endif
        partial_quantify_policy<quantify_policy> partial_impl(pred);
        unreduced_t res = __quantify(ep, std::move(dd), partial_impl, op);

        while (partial_impl.remaining_nodes > 0) {
#ifdef ADIAR_STATS
          // TODO
#endif
          partial_impl.reset();
          res = __quantify(ep, res, partial_impl, op);
        }
        return res;
      }

    case exec_policy::quantify::Singleton:
      { // ---------------------------------------------------------------------
        // Case: Repeated single variable quantification
#ifdef ADIAR_STATS
        // TODO
#endif
        while (label <= quantify_policy::max_label) {
          dd = quantify<quantify_policy>(ep, dd, label, op);
          if (dd_isterminal(dd)) { return std::move(dd); }

          label = quantify__get_deepest<quantify_policy>(dd, pred);
        }
        return dd;
      }

    case exec_policy::quantify::Nested:
      { // ---------------------------------------------------------------------
        // Case: Nested Sweeping
#ifdef ADIAR_STATS
        // TODO
#endif
        multi_quantify_policy__pred<quantify_policy> inner_impl(op, pred);
        return nested_sweep<>(ep,
                              quantify<quantify_policy>(ep, std::move(dd), label, op),
                              inner_impl);
      }
    case exec_policy::quantify::Auto:
      { // ---------------------------------------------------------------------
        // Case: Partial/Singleton Quantification + Nested Sweeping
        const size_t dd_size = dd.size();

        const size_t nodes_per_block =
          get_block_size() / sizeof(typename quantify_policy::node_type);

        // Do Partial Quantification as long as...
        //   1. ... it stays smaller than 150% of the input size.
        const size_t partial__size_threshold = 3u * (dd_size / 2u);

        //   2. ... the number of terminals stays above 2% of the input.
        //      (abusing rounding errors, this only has an effect if the
        //      decision diagram is larger than a single block).
        const size_t partial__terminal_threshold =
          (nodes_per_block / (100u / 2u)) * (dd_size / nodes_per_block);

        //   3. ... there (most likely) are still shallow to-be quantified
        //      variables left for this iteration ('shallow' is in this context
        //      variables above the deepest level with maximum width).
        //
        //      TODO: compute in the same iteration as 'label' above.
        const size_t partial__max_iterations =
          quantify__count_above_widest<quantify_policy>(dd, pred);

        unreduced_t transposed;

        // If 2. does not apply to the input, then partial quantification
        // probably will explode the diagram to something quadratic.
        if (dd.number_of_terminals() < partial__terminal_threshold) {
          // Singleton Quantification of bottom-most level
#ifdef ADIAR_STATS
          // TODO
#endif
          transposed = quantify<quantify_policy>(ep, std::move(dd), label, op);
        } else {
          // Partial Quantification
#ifdef ADIAR_STATS
          // TODO
#endif
          partial_quantify_policy<quantify_policy> partial_impl(pred);
          transposed = __quantify(ep, std::move(dd), partial_impl, op);

          if (partial_impl.remaining_nodes == 0) {
#ifdef ADIAR_STATS
            // TODO
#endif
            return transposed;
          }

          for (size_t i = 1; i < partial__max_iterations; ++i) {
            if (partial__size_threshold < transposed.size()) {
              break;
            }
            if (transposed.number_of_terminals() < partial__terminal_threshold) {
              break;
            }
#ifdef ADIAR_STATS
            // TODO
#endif
            // Reset policy and rerun partial quantification
            partial_impl.reset();
            transposed = __quantify(ep, transposed, partial_impl, op);

            // Reduce result, if no work is left to be done.
            if (partial_impl.remaining_nodes == 0) {
#ifdef ADIAR_STATS
              // TODO
#endif
              return transposed;
            }
          }
        }
#ifdef ADIAR_STATS
        // TODO
#endif
        { // Nested Sweeping
          multi_quantify_policy__pred<quantify_policy> inner_impl(op, pred);
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

  //////////////////////////////////////////////////////////////////////////////
  // Multi-variable (descending generator)
  template<typename quantify_policy>
  class multi_quantify_policy__generator
    : public multi_quantify_policy<quantify_policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of the levels to sweep on (or not to sweep on) in
    ///        descending order.
    ////////////////////////////////////////////////////////////////////////////
    using generator_t = generator<typename quantify_policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of levels to sweep on (or not to sweep on) in
    ///        descending order.
    ////////////////////////////////////////////////////////////////////////////
    const generator_t &_lvls;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer for to hold onto the generated next level.
    ////////////////////////////////////////////////////////////////////////////
    optional<typename quantify_policy::label_type> _next_level;

  public:
    ////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__generator(const bool_op &op, const generator_t &g)
      : multi_quantify_policy<quantify_policy>(op), _lvls(g)
    {
      _next_level = _lvls();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////
    bool has_sweep(const typename quantify_policy::label_type l)
    {
      return l == next_level(l)
        ? quantify_policy::quantify_onset
        : !quantify_policy::quantify_onset;
    }

  private:
    bool
    has_next_level() const
    {
      return _next_level.has_value();
    }

    typename quantify_policy::label_type
    next_level(const typename quantify_policy::label_type l)
    {
      while (_next_level.has_value() && l < _next_level.value()) {
        _next_level = _lvls();
      }
      return _next_level.value_or(quantify_policy::max_label+1);
    }
  };

  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  //       clean up
  //       - Make return type 'optional' rather than larger than 'max_label'
  template<typename quantify_policy>
  inline typename quantify_policy::label_type
  quantify__get_deepest(const typename quantify_policy::dd_type &dd,
                        const typename quantify_policy::label_type bot_level,
                        const optional<typename quantify_policy::label_type> top_level)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename quantify_policy::label_type l = lis.pull().label();
      if ((!top_level || top_level.value() < l) && l < bot_level) {
        return l;
      }
    }
    return quantify_policy::max_label+1;
  }

  template<typename quantify_policy>
  typename quantify_policy::__dd_type
  quantify(const exec_policy &ep,
           typename quantify_policy::dd_type dd,
           const typename multi_quantify_policy__generator<quantify_policy>::generator_t &lvls,
           const bool_op &op)
  {
    adiar_assert(is_commutative(op), "Operator must be commutative");

    // NOTE: read-once access with 'gen' makes partial quantification not
    //       possible.
    switch (ep.quantify_alg()) {
    case exec_policy::quantify::Partial:
    case exec_policy::quantify::Singleton:
      { // -------------------------------------------------------------------
        // Case: Repeated single variable quantification
        // TODO: correctly handle quantify_policy::quantify_onset
        optional<typename quantify_policy::label_type> on_level = lvls();

        if (quantify_policy::quantify_onset) {
          if (!on_level) { return dd; }

          // Quantify all but the last 'on_level'. Hence, look one ahead with
          // 'next_on_level' to see whether it is the last one.
          optional<typename quantify_policy::label_type> next_on_level = lvls();
          while (next_on_level) {
            dd = quantify<quantify_policy>(ep, dd, on_level.value(), op);
            if (dd_isterminal(dd)) { return dd; }

            on_level = next_on_level;
            next_on_level = lvls();
          }
          return quantify<quantify_policy>(ep, dd, on_level.value(), op);
        } else { // !quantify_policy::quantify_onset
          // TODO: only designed for 'OR' at this point in time
          if (!on_level) {
            return typename quantify_policy::dd_type(dd->number_of_terminals[true] > 0);
          }

          // Quantify everything below 'label'
          for (;;) {
            const typename quantify_policy::label_type off_level =
              quantify__get_deepest<quantify_policy>(dd, quantify_policy::max_label, on_level.value());

            if (quantify_policy::max_label < off_level) { break; }

            dd = quantify<quantify_policy>(ep, dd, off_level, op);
            if (dd_isterminal(dd)) { return dd; }
          }

          // Quantify everything strictly in between 'bot_level' and 'top_level'
          optional<typename quantify_policy::label_type> bot_level = on_level;
          optional<typename quantify_policy::label_type> top_level = lvls();

          while (bot_level) {
            for (;;) {
              const typename quantify_policy::label_type off_level =
                quantify__get_deepest<quantify_policy>(dd, bot_level.value(), top_level);

              if (quantify_policy::max_label < off_level) { break; }

              dd = quantify<quantify_policy>(ep, dd, off_level, op);
              if (dd_isterminal(dd)) { return dd; }
            }

            bot_level = top_level;
            top_level = lvls();
          }
          return dd;
        }
      }

    case exec_policy::quantify::Auto:
    case exec_policy::quantify::Nested:
      { // ---------------------------------------------------------------------
        // Case: Nested Sweeping
        //
        // NOTE: Despite partial quantification is not possible, we can
        //       (assuming we have to quantify the on-set) still use the
        //       bottom-most level to transpose the DAG.
        if constexpr (quantify_policy::quantify_onset) {
          // Obtain the bottom-most onset level that exists in the diagram.
          optional<typename quantify_policy::label_type> transposition_level = lvls();
          if (!transposition_level) { return dd; }

          {
            level_info_stream<true> in_meta(dd);
            typename quantify_policy::label_type dd_level = in_meta.pull().level();

            for (;;) {
              // Go forward in the diagram's levels, until we are at or above
              // the current candidate
              while (in_meta.can_pull() && transposition_level.value() < dd_level) {
                dd_level = in_meta.pull().level();
              }
              // There is no onset level in the diagram? If so, then nothing is
              // going to change and we may just return the input.
              if (!in_meta.can_pull() && transposition_level.value() < dd_level) {
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
                if (!transposition_level) { return dd; }
              }
            }
          }
          adiar_assert(transposition_level.has_value());

          // Quantify the 'transposition_level' as part of the initial transposition step
          multi_quantify_policy__generator<quantify_policy> inner_impl(op, lvls);
          return nested_sweep<>(ep, quantify<quantify_policy>(ep, dd, transposition_level.value(), op), inner_impl);
        } else { // !quantify_policy::quantify_onset
          multi_quantify_policy__generator<quantify_policy> inner_impl(op, lvls);
          return nested_sweep<>(ep, dd, inner_impl);
        }
      }

      // LCOV_EXCL_START
    default:
      // ---------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
