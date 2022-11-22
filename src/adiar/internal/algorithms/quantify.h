#ifndef ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
#define ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H

#include <variant>

#include <adiar/bool_op.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/cnl.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>

#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/request.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template<uint8_t nodes_carried>
  using quantify_request = request_data<2, true, nodes_carried, with_parent>;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  //
  // As such we could just reuse the comparators in tuple.h, but unlike in Apply
  // and Homomorphism we only maintain request for two nodes in the same
  // Decision Diagram. Assuming that the operator is commutative, we can in fact
  // due to this optimise everything by having t1 and t2 in the quantify_tuple
  // sorted.
  //
  // This improves the speed of the comparators.

  template<size_t LOOK_AHEAD, memory::memory_mode mem_mode>
  using quantify_priority_queue_1_t =
    levelized_node_priority_queue<quantify_request<0>,
                                  request_label<quantify_request<0>>,
                                  request_data_fst_lt<quantify_request<0>>,
                                  LOOK_AHEAD, mem_mode>;

  template<memory::memory_mode mem_mode>
  using quantify_priority_queue_2_t =
    priority_queue<mem_mode, quantify_request<1>, request_data_snd_lt<quantify_request<1>>>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename quantify_policy, typename pq_1_t>
  inline void __quantify_resolve_request(pq_1_t &quantify_pq_1,
                                         arc_writer &aw,
                                         const bool_op &op,
                                         const ptr_uint64 source, ptr_uint64 r1, ptr_uint64 r2)
  {
    adiar_debug(!r1.is_nil(), "ptr_uint64::NIL() should only ever end up being placed in r2");

    // Collapse requests to the same node back into one
    if (r2.is_node() && r1 == r2) { r2 = ptr_uint64::NIL(); }

    if (r2.is_nil()) {
      if (r1.is_terminal()) {
        aw.unsafe_push_terminal({ source, r1 });
      } else {
        quantify_pq_1.push({ {r1, r2}, {}, {source} });
      }
    } else {
      // sort the tuple of requests
      quantify_request<0>::target_t rec = quantify_policy::resolve_request(op, r1, r2);

      if (rec[0].is_terminal() /* && rec[1].is_terminal() */) {
        arc out_arc = { source, op(rec[0], rec[1]) };
        aw.unsafe_push_terminal(out_arc);
      } else {
        quantify_pq_1.push({ rec, {}, {source} });
      }
    }
  }

  template<typename pq_1_t, typename pq_2_t>
  inline bool __quantify_update_source_or_break(pq_1_t &quantify_pq_1,
                                                pq_2_t &quantify_pq_2,
                                                ptr_uint64 &source,
                                                const ptr_uint64 t1, const ptr_uint64 t2)
  {
    if (quantify_pq_1.can_pull() && quantify_pq_1.top().target[0] == t1 && quantify_pq_1.top().target[1] == t2) {
      source = quantify_pq_1.pull().data.source;
    } else if (!quantify_pq_2.empty() && quantify_pq_2.top().target[0] == t1 && quantify_pq_2.top().target[1] == t2) {
      source = quantify_pq_2.top().data.source;
      quantify_pq_2.pop();
    } else {
      return true;
    }
    return false;
  }

  inline bool quantify_has_label(const dd::label_t label, const dd &in)
  {
    level_info_stream<node> in_meta(in);
    while(in_meta.can_pull()) {
      level_info_t m = in_meta.pull();

      // Are we already past where it should be?
      if (label < label_of(m)) { return false; }

      // Did we find it?
      if (label_of(m) == label) { return true; }
    }
    return false;
  }

  template<typename quantify_policy, typename pq_1_t, typename pq_2_t>
  typename quantify_policy::unreduced_t __quantify(const typename quantify_policy::reduced_t &in,
                                                   const typename quantify_policy::label_t &label,
                                                   const bool_op &op,
                                                   const size_t pq_1_memory, const size_t max_pq_1_size,
                                                   const size_t pq_2_memory, const size_t max_pq_2_size)
  {
    // Check for trivial terminal-only return on shortcutting the root
    node_stream<> in_nodes(in);
    node v = in_nodes.pull();

    if (v.label() == label && (v.low().is_terminal() || v.high().is_terminal())) {
      typename quantify_policy::unreduced_t maybe_resolved = quantify_policy::resolve_terminal_root(v, op);

      if (!maybe_resolved.empty()) {
        return maybe_resolved;
      }
    }

    // Set-up for arc_file output
    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_1_t quantify_pq_1({in}, pq_1_memory, max_pq_1_size, stats_quantify.lpq);
    pq_2_t quantify_pq_2(pq_2_memory, max_pq_2_size);

    typename quantify_policy::label_t out_label = v.uid().label();
    typename quantify_policy::id_t out_id = 0;

    if (v.uid().label() == label) {
      // Precondition: The input is reduced and will not collapse to a terminal
      quantify_pq_1.push({ {fst(v.low(), v.high()), snd(v.low(), v.high())}, {}, {ptr_uint64::NIL()} });
    } else {
      const uid_t out_uid(out_label, out_id++);

      if (v.low().is_terminal()) {
        aw.unsafe_push_terminal({ out_uid, v.low() });
      } else {
        quantify_pq_1.push({ {v.low(), ptr_uint64::NIL()}, {}, {out_uid} });
      }
      if (v.high().is_terminal()) {
        aw.unsafe_push_terminal({ flag(out_uid), v.high() });
      } else {
        quantify_pq_1.push({ {v.high(), ptr_uint64::NIL()}, {}, {flag(out_uid)} });
      }
    }

    size_t max_1level_cut = 0;

    while(!quantify_pq_1.empty() || !quantify_pq_2.empty()) {
      if (quantify_pq_1.empty_level() && quantify_pq_2.empty()) {
        if (out_label != label) {
          aw.unsafe_push(create_level_info(out_label, out_id));
        }

        quantify_pq_1.setup_next_level();
        out_label = quantify_pq_1.current_level();
        out_id = 0;

        max_1level_cut = std::max(max_1level_cut, quantify_pq_1.size());
      }

      quantify_request<1> req;
      bool with_data = false;

      // Merge requests from quantify_pq_1 and quantify_pq_2 (pretty much just as for Apply)
      if (quantify_pq_1.can_pull()
          && (quantify_pq_2.empty() || quantify_pq_1.top().target.fst() < quantify_pq_2.top().target.snd())) {
        with_data = false;

        req = { quantify_pq_1.top().target,
                {{ node::ptr_t::NIL(), node::ptr_t::NIL() }},
                quantify_pq_1.top().data };
        quantify_pq_1.pop();
      } else {
        with_data = true;

        req = quantify_pq_2.top();
        quantify_pq_2.pop();
      }

      // Seek element from request in stream
      ptr_uint64 t_seek = with_data ? req.target.snd() : req.target.fst();

      while (v.uid() < t_seek) {
        v = in_nodes.pull();
      }

      // Forward information of node t1 across the level if needed
      if (!with_data
          && !req.target.snd().is_nil()
          && req.target.snd().is_node()
          && req.target.fst().label() == req.target.snd().label()) {
        quantify_pq_2.push({ req.target, {v.children()}, req.data });

        while (quantify_pq_1.can_pull() && (quantify_pq_1.top().target == req.target)) {
          quantify_pq_2.push({ req.target, {v.children()}, quantify_pq_1.pull().data });
        }

        continue;
      }

      if (req.target.fst().label() == label) {
        // The variable should be quantified: proceed somewhat as for the BDD
        // Restrict algorithm by forwarding the request of source further to the
        // children, though here we keep track of both possibilities.
        adiar_debug(req.target.snd().is_nil(),
                    "Ended in pairing case on request that already is a pair");

        do {
          __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op,
                                                      req.data.source,
                                                      v.low(), v.high());
        } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                    req.data.source,
                                                    req.target[0], req.target[1]));
      } else {
        // The variable should stay: proceed as in the Product Construction by
        // simulating both possibilities in parallel.

        // Resolve current node and recurse.
        ptr_uint64 low1  = with_data ? req.node_carry[0][false]  : v.low();
        ptr_uint64 high1 = with_data ? req.node_carry[0][true]   : v.high();
        ptr_uint64 low2  = with_data ? v.low()     : req.target.snd();
        ptr_uint64 high2 = with_data ? v.high()    : req.target.snd();

        quantify_policy::compute_cofactor(true, low1, high1);
        quantify_policy::compute_cofactor(req.target.snd().on_level(out_label), low2, high2);

        adiar_debug(out_id < quantify_policy::MAX_ID, "Has run out of ids");
        const uid_t out_uid(out_label, out_id++);

        __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op, out_uid, low1, low2);
        __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op, flag(out_uid), high1, high2);

        if (!req.data.source.is_nil()) {
          do {
            arc out_arc = { req.data.source, out_uid };
            aw.unsafe_push_node(out_arc);
          } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2,
                                                      req.data.source,
                                                      req.target[0], req.target[1]));
        }
      }
    }

    // Push the level of the very last iteration
    if (out_label != label) {
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    out_arcs->max_1level_cut = max_1level_cut;

    return out_arcs;
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
    adiar_debug(is_commutative(op), "Noncommutative operator used");

    // Check if there is no need to do any computation
    if (is_terminal(in) || !quantify_has_label(label, in)) {
      return in;
    }

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory::available()
      // Input stream
      - node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::DATA_STRUCTURES;

    constexpr size_t data_structures_in_pq_2 =
      quantify_priority_queue_2_t<memory::INTERNAL>::DATA_STRUCTURES;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_internal_memory =
      aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      quantify_priority_queue_2_t<memory::INTERNAL>::memory_fits(pq_2_internal_memory);

    const bool internal_only = memory::mode == memory::INTERNAL;
    const bool external_only = memory::mode == memory::EXTERNAL;

    const size_t pq_1_bound = std::min({__quantify_ilevel_upper_bound<quantify_policy, get_2level_cut, 2u>(in,op),
                                        __quantify_ilevel_upper_bound<quantify_policy>(in)});

    const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __quantify_ilevel_upper_bound<quantify_policy, get_1level_cut, 0u>(in,op);

    const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if(!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.unbucketed++;
#endif
      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<0, memory::INTERNAL>,
                        quantify_priority_queue_2_t<memory::INTERNAL>>
        (in, label, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if(!external_only && max_pq_1_size <= pq_1_memory_fits
                             && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_quantify.lpq.internal++;
#endif
      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>,
                        quantify_priority_queue_2_t<memory::INTERNAL>>
        (in, label, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_quantify.lpq.external++;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::EXTERNAL>,
                        quantify_priority_queue_2_t<memory::EXTERNAL>>
        (in, label, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
