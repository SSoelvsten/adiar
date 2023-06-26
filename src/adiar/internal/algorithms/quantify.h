#ifndef ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
#define ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H

#include <variant>
#include <functional>

#include <adiar/bool_op.h>
#include <adiar/quantify_mode.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/cnl.h>
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
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template<uint8_t nodes_carried>
  using quantify_request = request_data<2, with_parent, nodes_carried, 1>;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

  template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
  using quantify_priority_queue_1_t =
    levelized_node_priority_queue<quantify_request<0>,
                                  request_data_fst_lt<quantify_request<0>>,
                                  LOOK_AHEAD,
                                  mem_mode,
                                  1,
                                  0>;

  template<memory_mode_t mem_mode>
  using quantify_priority_queue_2_t =
    priority_queue<mem_mode, quantify_request<1>, request_data_snd_lt<quantify_request<1>>>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename quantify_policy, typename pq_1_t>
  inline void __quantify_resolve_request(pq_1_t &quantify_pq_1,
                                         arc_writer &aw,
                                         const bool_op &op,
                                         const typename quantify_policy::ptr_t source,
                                         const tuple<typename quantify_policy::ptr_t, 2> &target)
  {
    adiar_debug(!target[0].is_nil(),
                "ptr_t::NIL() should only ever end up being placed in target[1]");

    // Collapse requests to the same node back into one
    if (target.snd().is_node() && target.fst() == target.snd()) {
      return __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op,
                                                         source,
                                                         { target.fst(), node::ptr_t::NIL() });
    }

    if (target[1].is_nil()) {
      if (target[0].is_terminal()) {
        aw.push_terminal({ source, target[0] });
      } else { // target[0].is_node()
        quantify_pq_1.push({ {target[0], target[1]}, {}, {source} });
      }
    } else {
      quantify_request<0>::target_t rec = quantify_policy::resolve_request(op, {target.fst(), target.snd()});

      if (rec[0].is_terminal() /* sorted ==> rec[1].is_terminal() */) {
        aw.push_terminal({ source, op(rec[0], rec[1]) });
      } else {
        quantify_pq_1.push({ rec, {}, {source} });
      }
    }
  }

  template<typename pq_1_t, typename pq_2_t>
  inline bool __quantify_update_source_or_break(pq_1_t &quantify_pq_1,
                                                pq_2_t &quantify_pq_2,
                                                ptr_uint64 &source,
                                                const quantify_request<0>::target_t &target)
  {
    if (quantify_pq_1.can_pull()
        && quantify_pq_1.top().target[0] == target[0]
        && quantify_pq_1.top().target[1] == target[1]) {
      source = quantify_pq_1.pull().data.source;
    } else if (!quantify_pq_2.empty()
               && quantify_pq_2.top().target[0] == target[0]
               && quantify_pq_2.top().target[1] == target[1]) {
      source = quantify_pq_2.top().data.source;
      quantify_pq_2.pop();
    } else {
      return true;
    }
    return false;
  }

  template<typename quantify_policy, typename pq_1_t, typename pq_2_t>
  typename quantify_policy::shared_arcs_t
  __quantify(const typename quantify_policy::reduced_t &in,
             const typename quantify_policy::label_t &label,
             const bool_op &op,
             pq_1_t &quantify_pq_1,
             pq_2_t &quantify_pq_2)
  {
    // TODO (partial quantification) / (optimisation for nested sweeping):
    //   Replace label with a 'quantify_impl' that provides a predicate whether
    //   to quantify a certain level.
    //
    //   This should then also expose the operator instead.

    // Set up input
    // TODO: use '.seek(...)' with 'in_nodes' instead such that it only needs to
    //       be opened once in the caller of this function.
    node_stream<> in_nodes(in);
    typename quantify_policy::node_t v = in_nodes.pull();

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    // Process requests in topological order of both BDDs
    while(!quantify_pq_1.empty()) {
      adiar_invariant(quantify_pq_2.empty(),
                      "Secondary priority queue is only non-empty while processing each level");

      // Set up level
      quantify_pq_1.setup_next_level();
      const typename quantify_policy::label_t out_label = quantify_pq_1.current_level();
      typename quantify_policy::id_t out_id = 0;

      // TODO: move quantification test out here instead

      while (!quantify_pq_1.empty_level() || !quantify_pq_2.empty()) {
        // Merge requests from quantify_pq_1 and quantify_pq_2
        quantify_request<1> req;

        if (quantify_pq_1.can_pull()
            && (quantify_pq_2.empty() || quantify_pq_1.top().target.fst() < quantify_pq_2.top().target.snd())) {
          req = { quantify_pq_1.top().target,
                  {{ { node::ptr_t::NIL(), node::ptr_t::NIL() } }},
                  quantify_pq_1.top().data };
          quantify_pq_1.pop();
        } else {
          req = quantify_pq_2.top();
          quantify_pq_2.pop();
        }

        // Seek element from request in stream
        const ptr_uint64 t_seek = req.empty_carry() ? req.target.fst() : req.target.snd();

        while (v.uid() < t_seek) {
          v = in_nodes.pull();
        }

        // Forward information of node t1 across the level if needed
        if (req.empty_carry()
            && req.target.snd().is_node()
            && req.target.fst().label() == req.target.snd().label()) {
          adiar_debug(!req.target.snd().is_nil(),
                      "req.target.snd().is_node ==> !req.target.snd().is_nil()");

          quantify_pq_2.push({ req.target, {v.children()}, req.data });

          while (quantify_pq_1.can_pull() && (quantify_pq_1.top().target == req.target)) {
            quantify_pq_2.push({ req.target, {v.children()}, quantify_pq_1.pull().data });
          }

          continue;
        }

        if (req.target.fst().label() == label) {
          // TODO:
          // (1) Use policy impl instead to check whether to quantify
          // (2) Add logic to output partially quantified nodes

          // The variable should be quantified: proceed somewhat as for the BDD
          // Restrict algorithm by forwarding the request of source further to the
          // children, though here we keep track of both possibilities.
          adiar_debug(req.target.snd().is_nil(),
                      "Ended in pairing case on request that already is a pair");

          do {
            __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op,
                                                        req.data.source,
                                                        v.children());
          } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                      req.data.source,
                                                      req.target));
        } else {
          // The variable should stay: proceed as in the Product Construction by
          // simulating both possibilities in parallel.

          // Resolve current node and recurse.
          const node::children_t children0 =
            req.empty_carry() ? v.children() : req.node_carry[0];

          const node::children_t children1 =
            req.target.snd().on_level(out_label)
            ? v.children()
            : quantify_policy::reduction_rule_inv(req.target.snd());

          adiar_debug(out_id < quantify_policy::MAX_ID, "Has run out of ids");
          const node::uid_t out_uid(out_label, out_id++);

          __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op,
                                                      out_uid.with(false),
                                                      { children0[false], children1[false] });

          __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op,
                                                      out_uid.with(true),
                                                      { children0[true], children1[true] });

          if (!req.data.source.is_nil()) {
            do {
              arc out_arc = { req.data.source, out_uid };
              aw.push_internal(out_arc);
            } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                        req.data.source,
                                                        req.target));
          }
        }
      }

      if (out_id > 0) {
        aw.push(level_info(out_label, out_id));
      }

      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, quantify_pq_1.size());
    }

    // Ensure the edge case, where the in-going edge from NIL to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                                  - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return out_arcs;
  }

  template<typename quantify_policy, typename pq_1_t, typename pq_2_t>
  typename quantify_policy::unreduced_t __quantify(const typename quantify_policy::reduced_t &in,
                                                   const typename quantify_policy::label_t &label,
                                                   const bool_op &op,
                                                   const size_t pq_1_memory, const size_t max_pq_1_size,
                                                   const size_t pq_2_memory, const size_t max_pq_2_size)
  {
    // Check for trivial terminal-only return on shortcutting the root
    typename quantify_policy::node_t root;

    { // Detach and garbage collect node_stream<>
      node_stream<> in_nodes(in);
      root = in_nodes.pull();

      if (root.label() == label && (root.low().is_terminal() || root.high().is_terminal())) {
        typename quantify_policy::ptr_t result = quantify_policy::resolve_root(root, op);

        if (result != root.uid()) {
          adiar_debug(result.is_terminal(), "Should shortcut to a terminal");
          return typename quantify_policy::reduced_t(result.value());
        }
      }
    }

    // Set up cross-level priority queue
    pq_1_t quantify_pq_1({in}, pq_1_memory, max_pq_1_size, stats_quantify.lpq);
    quantify_pq_1.push({ { root.uid(), ptr_uint64::NIL() }, {}, {ptr_uint64::NIL()} });

    // Set up per-level priority queue
    pq_2_t quantify_pq_2(pq_2_memory, max_pq_2_size);

    return __quantify<quantify_policy, pq_1_t, pq_2_t>
      (in, label, op, quantify_pq_1, quantify_pq_2);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its
  /// maximum i-level cut.
  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy, typename cut, size_t const_size_inc>
  size_t __quantify_ilevel_upper_bound(const typename quantify_policy::reduced_t &in,
                                       const bool_op &op)
  {
    const cut_type ct_internal = cut_type::INTERNAL;
    const cut_type ct_terminals = quantify_policy::cut_with_terminals(op);

    const safe_size_t max_cut_internal = cut::get(in, ct_internal);
    const safe_size_t max_cut_terminals = cut::get(in, ct_terminals);

    return to_size(max_cut_internal * max_cut_terminals + const_size_inc);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its size.
  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy>
  size_t __quantify_ilevel_upper_bound(const typename quantify_policy::reduced_t &in)
  {
    const safe_size_t in_size = in->size();
    return to_size(in_size * in_size + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy>
  typename quantify_policy::unreduced_t quantify(const typename quantify_policy::reduced_t &in,
                                                 const typename quantify_policy::label_t label,
                                                 const bool_op &op)
  {
    adiar_debug(is_commutative(op), "A commutative operator must be used");

    // Check if there is no need to do any computation
    if (is_terminal(in) || !has_level(in, label)) {
      return in;
    }

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory_available()
      // Input stream
      - node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::DATA_STRUCTURES;

    constexpr size_t data_structures_in_pq_2 =
      quantify_priority_queue_2_t<memory_mode_t::INTERNAL>::DATA_STRUCTURES;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_internal_memory =
      aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      quantify_priority_queue_2_t<memory_mode_t::INTERNAL>::memory_fits(pq_2_internal_memory);

    const bool internal_only = memory_mode == memory_mode_t::INTERNAL;
    const bool external_only = memory_mode == memory_mode_t::EXTERNAL;

    const size_t pq_1_bound = std::min({__quantify_ilevel_upper_bound<quantify_policy, get_2level_cut, 2u>(in,op),
                                        __quantify_ilevel_upper_bound<quantify_policy>(in)});

    const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __quantify_ilevel_upper_bound<quantify_policy, get_1level_cut, 0u>(in,op);

    const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if(!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.unbucketed += 1u;
#endif
      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<0, memory_mode_t::INTERNAL>,
                        quantify_priority_queue_2_t<memory_mode_t::INTERNAL>>
        (in, label, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if(!external_only && max_pq_1_size <= pq_1_memory_fits
                             && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.internal += 1u;
#endif
      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>,
                        quantify_priority_queue_2_t<memory_mode_t::INTERNAL>>
        (in, label, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_quantify.lpq.external += 1u;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::EXTERNAL>,
                        quantify_priority_queue_2_t<memory_mode_t::EXTERNAL>>
        (in, label, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy>
  class multi_quantify_policy : public quantify_policy
  {
  private:
    const bool_op &_op;

  public:
    using request_t = quantify_request<0>;
    using request_pred_t = request_data_fst_lt<request_t>;

    template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
    using pq_t = quantify_priority_queue_1_t<LOOK_AHEAD, mem_mode>;

  public:
    ////////////////////////////////////////////////////////////////////////////
    static size_t stream_memory()
    { return node_stream<>::memory_usage() + arc_writer::memory_usage(); }

    ////////////////////////////////////////////////////////////////////////////
    static size_t pq_memory(const size_t inner_memory)
    {
      constexpr size_t data_structures_in_pq_1 =
        quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::DATA_STRUCTURES;

      constexpr size_t data_structures_in_pq_2 =
        quantify_priority_queue_2_t<memory_mode_t::INTERNAL>::DATA_STRUCTURES;

      return  (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t pq_bound(const typename quantify_policy::shared_nodes_t &outer_file,
                    const size_t /*outer_roots*/)
    {
      return std::min(__quantify_ilevel_upper_bound<quantify_policy, get_2level_cut, 2u>(outer_file, _op),
                      __quantify_ilevel_upper_bound<quantify_policy>(outer_file));
    }

  public:
    multi_quantify_policy(const bool_op &op)
      : _op(op)
    { }

  public:
    // bool has_sweep(typename quantify_policy::label_t) const;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Run Sweep with given priority queue.
    ////////////////////////////////////////////////////////////////////////////
    template<typename inner_pq_1_t>
    typename quantify_policy::unreduced_t
    sweep_pq(const shared_levelized_file<node> &outer_file,
             inner_pq_1_t &inner_pq_1,
             const size_t inner_remaining_memory)
    {
      const size_t pq_2_memory_fits =
        quantify_priority_queue_2_t<memory_mode_t::INTERNAL>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =
        __quantify_ilevel_upper_bound<quantify_policy, get_1level_cut, 0u>(outer_file, _op);

      const size_t max_pq_2_size = memory_mode == memory_mode_t::INTERNAL
        ? std::min(pq_2_memory_fits, pq_2_bound)
        : pq_2_bound;

      if(memory_mode != memory_mode_t::EXTERNAL && max_pq_2_size <= pq_2_memory_fits) {
        using inner_pq_2_t = quantify_priority_queue_2_t<memory_mode_t::INTERNAL>;
        inner_pq_2_t inner_pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify<quantify_policy, inner_pq_1_t, inner_pq_2_t>
          (outer_file, 0 /*dummy*/, _op, inner_pq_1, inner_pq_2);
      } else {
        using inner_pq_2_t = quantify_priority_queue_2_t<memory_mode_t::EXTERNAL>;
        inner_pq_2_t inner_pq_2(inner_remaining_memory, max_pq_2_size);

        return __quantify<quantify_policy, inner_pq_1_t, inner_pq_2_t>
          (outer_file, 0 /*dummy*/, _op, inner_pq_1, inner_pq_2);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Pick PQ type and Run Sweep.
    ////////////////////////////////////////////////////////////////////////////
    template<typename outer_roots_t>
    typename quantify_policy::unreduced_t
    sweep(const shared_levelized_file<node> &outer_file,
          outer_roots_t &outer_roots,
          const size_t inner_memory)
    {
      return nested_sweeping::inner::down__sweep_switch(*this, outer_file, outer_roots, inner_memory);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Create request
    //////////////////////////////////////////////////////////////////////////////
    inline request_t
    request_from_node(const typename quantify_policy::node_t &n,
                      const typename quantify_policy::ptr_t &parent)
    {
      // Shortcutting terminal?
      const typename quantify_policy::ptr_t result =
        quantify_policy::resolve_root(n, _op);

      // Always pick high child
      typename request_t::target_t tgt = result != n.uid()
        ? request_t::target_t{ result, quantify_policy::ptr_t::NIL() }
        : request_t::target_t{ fst(n.low(), n.high()), snd(n.low(), n.high()) };

      return request_t(tgt, {}, {parent});
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Multi-variable (file)
  // TODO (deprecated): Remove

  // LCOV_EXCL_START
  template<typename quantify_policy>
  typename quantify_policy::unreduced_t
  quantify(typename quantify_policy::reduced_t dd,
           const shared_file<typename quantify_policy::label_t> labels,
           const bool_op &op)
  {
    const size_t labels_size = labels->size();
    if (labels_size == 0) { return dd; }

    file_stream<typename quantify_policy::label_t> label_stream(labels);

    for (size_t label_idx = 0u; label_idx < labels_size - 1; label_idx++) {
      if (is_terminal(dd)) { return dd; }

      adiar_debug(label_stream.can_pull(), "Should not exceed 'labels' size");
      dd = quantify<quantify_policy>(dd, label_stream.pull(), op);
    }

    adiar_debug(label_stream.can_pull(), "Should not exceed 'labels' size");
    const typename quantify_policy::label_t label = label_stream.pull();
    adiar_debug(!label_stream.can_pull(), "Should pull final label");
    return quantify<quantify_policy>(dd, label, op);
  }
  // LCOV_EXCL_STOP

  //////////////////////////////////////////////////////////////////////////////
  // Multi-variable (predicate)
  template<typename quantify_policy>
  class multi_quantify_policy__pred
    : public multi_quantify_policy<quantify_policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////
    using pred_t = std::function<bool(typename quantify_policy::label_t)>;

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
    bool has_sweep(node::ptr_t::label_t l)
    {
      return _pred(l) == quantify_policy::quantify_onset;
    }
  };

  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_level' should not terminate early but
  //         determine whether any variable may "survive".
  template<typename quantify_policy>
  inline typename quantify_policy::label_t
  quantify__get_level(const typename quantify_policy::reduced_t &dd,
                      const std::function<bool(typename quantify_policy::label_t)> &pred)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename quantify_policy::label_t l = lis.pull().label();
      if (pred(l) == quantify_policy::quantify_onset) { return l; }
    }
    return quantify_policy::MAX_LABEL+1;
  }

  template<typename quantify_policy>
  typename quantify_policy::unreduced_t
  quantify(typename quantify_policy::reduced_t dd,
           const std::function<bool(typename quantify_policy::label_t)> &pred,
           const bool_op &op)
  {
    typename quantify_policy::label_t label = quantify__get_level<quantify_policy>(dd, pred);
    if (quantify_policy::MAX_LABEL < label) { return dd; }

    switch (quantify_mode) {
    case quantify_mode_t::PARTIAL:
      { // ---------------------------------------------------------------------
        // Case: Repeated partial quantification

        // TODO: implement partial sweeping (and move case below singleton)
      }

    case quantify_mode_t::SINGLETON:
      { // -------------------------------------------------------------------
        // Case: Repeated single variable quantification
        while (label <= quantify_policy::MAX_LABEL) {
          dd = quantify<quantify_policy>(dd, label, op);
          if (is_terminal(dd)) { return dd; }

          label = quantify__get_level<quantify_policy>(dd, pred);
        }
        return dd;
      }

    case quantify_mode_t::NESTED:
      { // ---------------------------------------------------------------------
        // Case: Nested Sweeping

        // TODO: Copy the AUTO case up here, when we add the more complex
        //       preprocessing with partial quantification.
      }
    case quantify_mode_t::AUTO:
      { // ---------------------------------------------------------------------
        // Case: Nested Sweeping
        using outer_up_sweep = nested_sweeping::outer::up__policy_t<quantify_policy>;
        multi_quantify_policy__pred<quantify_policy> inner_impl(op, pred);

        // TODO: If AUTO, apply partial quantification until result is larger
        //       than some 1+epsilon factor.

        return nested_sweep<outer_up_sweep>(quantify<quantify_policy>(dd, label, op), inner_impl);
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
  class multi_quantify_policy__gen
    : public multi_quantify_policy<quantify_policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of levels to sweep on (or not to sweep on) in
    ///        descending order.
    ////////////////////////////////////////////////////////////////////////////
    using gen_t = std::function<typename quantify_policy::label_t()>;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of levels to sweep on (or not to sweep on) in
    ///        descending order.
    ////////////////////////////////////////////////////////////////////////////
    const gen_t &_gen;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer for to hold onto the generated next level.
    ////////////////////////////////////////////////////////////////////////////
    typename quantify_policy::label_t _next_level;

  public:
    ////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__gen(const bool_op &op, const gen_t &gen)
      : multi_quantify_policy<quantify_policy>(op), _gen(gen)
    {
      _next_level = _gen();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////
    bool has_sweep(const node::ptr_t::label_t l)
    {
      return l == next_level(l)
        ? quantify_policy::quantify_onset
        : !quantify_policy::quantify_onset;
    }

  private:
    bool
    has_next_level() const
    { return _next_level <= quantify_policy::MAX_LABEL; }

    const node::ptr_t::label_t&
    next_level(const node::ptr_t::label_t l)
    {
      while (has_next_level() && l < _next_level) {
        _next_level = _gen();
      }
      return _next_level;
    }
  };

  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial 'quantify__get_level' should not terminate early but
  //         determine whether any variable may "survive".
  template<typename quantify_policy>
  inline typename quantify_policy::label_t
  quantify__get_level(const typename quantify_policy::reduced_t &dd,
                      const typename quantify_policy::label_t bot_level,
                      const typename quantify_policy::label_t top_level)
  {
    level_info_stream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename quantify_policy::label_t l = lis.pull().label();
      if ((top_level < l || quantify_policy::MAX_LABEL < top_level) && l < bot_level) {
        return l;
      }
    }
    return quantify_policy::MAX_LABEL+1;
  }

  template<typename quantify_policy>
  typename quantify_policy::unreduced_t
  quantify(typename quantify_policy::reduced_t dd,
           const typename multi_quantify_policy__gen<quantify_policy>::gen_t &gen,
           const bool_op &op)
  {
    adiar_debug(is_commutative(op), "Operator must be commutative");

    // NOTE: read-once access with 'gen' makes partial quantification not
    //       possible.
    switch (quantify_mode) {
    case quantify_mode_t::PARTIAL:
    case quantify_mode_t::SINGLETON:
      { // -------------------------------------------------------------------
        // Case: Repeated single variable quantification
        // TODO: correctly handle quantify_policy::quantify_onset
        typename quantify_policy::label_t on_level = gen();

        if (quantify_policy::quantify_onset) {
          if (quantify_policy::MAX_LABEL < on_level) { return dd; }

          typename quantify_policy::label_t next_on_level = gen();
          while (next_on_level <= quantify_policy::MAX_LABEL) {
            dd = quantify<quantify_policy>(dd, on_level, op);
            if (is_terminal(dd)) { return dd; }

            on_level = next_on_level;
            next_on_level = gen();
          }
          return quantify<quantify_policy>(dd, on_level, op);
        } else { // !quantify_policy::quantify_onset
          // TODO: only designed for 'OR' at this point in time
          if (quantify_policy::MAX_LABEL < on_level) {
            return typename quantify_policy::reduced_t(dd->number_of_terminals[true] > 0);
          }

          // Quantify everything below 'label'
          for (;;) {
            const typename quantify_policy::label_t off_level =
              quantify__get_level<quantify_policy>(dd, quantify_policy::MAX_LABEL, on_level);

            if (quantify_policy::MAX_LABEL < off_level) { break; }

            dd = quantify<quantify_policy>(dd, off_level, op);
            if (is_terminal(dd)) { return dd; }
          }

          // Quantify everything strictly in between 'bot_level' and 'top_level'
          typename quantify_policy::label_t bot_level = on_level;
          typename quantify_policy::label_t top_level = gen();

          while (bot_level <= quantify_policy::MAX_LABEL) {
            for (;;) {
              const typename quantify_policy::label_t off_level =
                quantify__get_level<quantify_policy>(dd, bot_level, top_level);

              if (quantify_policy::MAX_LABEL < off_level) { break; }

              dd = quantify<quantify_policy>(dd, off_level, op);
              if (is_terminal(dd)) { return dd; }
            }

            bot_level = top_level;
            top_level = gen();
          }
          return dd;
        }
      }

    case quantify_mode_t::AUTO:
    case quantify_mode_t::NESTED:
      { // ---------------------------------------------------------------------
        // Case: Nested Sweeping
        //
        // NOTE: Despite partial quantification is not possible, we can
        //       (assuming we have to quantify the on-set) still use the
        //       bottom-most level to transpose the DAG.
        using outer_up_sweep = nested_sweeping::outer::up__policy_t<quantify_policy>;

        if constexpr (quantify_policy::quantify_onset) {
          typename quantify_policy::label_t label = gen();
          if (quantify_policy::MAX_LABEL < label) { return dd; }

          // TODO: get bottom-most level that actually exists in DAG.

          multi_quantify_policy__gen<quantify_policy> inner_impl(op, gen);
          return nested_sweep<outer_up_sweep>(quantify<quantify_policy>(dd, label, op), inner_impl);
        } else { // !quantify_policy::quantify_onset
          multi_quantify_policy__gen<quantify_policy> inner_impl(op, gen);
          return nested_sweep<outer_up_sweep>(dd, inner_impl);
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
