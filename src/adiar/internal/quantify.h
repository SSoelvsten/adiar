#ifndef ADIAR_INTERNAL_QUANTIFY_H
#define ADIAR_INTERNAL_QUANTIFY_H

#include <variant>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/tuple.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct quantify_tuple : tuple
  {
    ptr_t source;
  };

  struct quantify_tuple_data : tuple_data
  {
    ptr_t source;
  };

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
  struct quantify_1_lt
  {
    bool operator()(const quantify_tuple &a, const quantify_tuple &b)
    {
      return a.t1 < b.t1 || (a.t1 == b.t1 && a.t2 < b.t2)
#ifndef NDEBUG
        || (a.t1 == b.t1 && a.t2 == b.t2 && a.source < b.source)
#endif
        ;
    }
  };

  struct quantify_2_lt
  {
    bool operator()(const quantify_tuple_data &a, const quantify_tuple_data &b)
    {
      return a.t2 < b.t2 || (a.t2 == b.t2 && a.t1 < b.t1)
#ifndef NDEBUG
        || (a.t1 == b.t1 && a.t2 == b.t2 && a.source < b.source)
#endif
        ;
    }
  };

  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using quantify_priority_queue_1_t =
    levelized_node_priority_queue<quantify_tuple, tuple_label, quantify_1_lt,
                                  sorter_template, priority_queue_template>;

  template<template<typename, typename> typename priority_queue_template>
  using quantify_priority_queue_2_t = priority_queue_template<quantify_tuple_data, quantify_2_lt>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename quantify_policy, typename pq_1_t>
  inline void __quantify_resolve_request(pq_1_t &quantify_pq_1,
                                         arc_writer &aw,
                                         const bool_op &op,
                                         const ptr_t source, ptr_t r1, ptr_t r2)
  {
    adiar_debug(!is_nil(r1), "NIL should only ever end up being placed in r2");

    // Collapse requests to the same node back into one
    if (is_node(r2) && r1 == r2) { r2 = NIL; }

    if (is_nil(r2)) {
      if (is_sink(r1)) {
        aw.unsafe_push_sink({ source, r1 });
      } else {
        quantify_pq_1.push({ r1, r2, source });
      }
    } else {
      // sort the tuple of requests
      tuple rec = quantify_policy::resolve_request(op, r1, r2);
      adiar_debug(fst(rec) == rec.t1 && snd(rec) == rec.t2, "Request recursion should be created in-order");

      if (is_sink(rec.t1) /* && is_sink(rec.t2) */) {
        arc_t out_arc = { source, op(rec.t1, rec.t2) };
        aw.unsafe_push_sink(out_arc);
      } else {
        quantify_pq_1.push({ rec.t1, rec.t2, source });
      }
    }
  }

  template<typename pq_1_t, typename pq_2_t>
  inline bool __quantify_update_source_or_break(pq_1_t &quantify_pq_1,
                                                pq_2_t &quantify_pq_2,
                                                ptr_t &source,
                                                const ptr_t t1, const ptr_t t2)
  {
    if (quantify_pq_1.can_pull() && quantify_pq_1.top().t1 == t1 && quantify_pq_1.top().t2 == t2) {
      source = quantify_pq_1.pull().source;
    } else if (!quantify_pq_2.empty() && quantify_pq_2.top().t1 == t1 && quantify_pq_2.top().t2 == t2) {
      source = quantify_pq_2.top().source;
      quantify_pq_2.pop();
    } else {
      return true;
    }
    return false;
  }

  inline bool quantify_has_label(const label_t label, const decision_diagram &in)
  {
    level_info_stream<node_t> in_meta(in);
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
                                                   const label_t &label,
                                                   const bool_op &op,
                                                   const size_t pq_1_memory,
                                                   const size_t pq_2_memory,
                                                   const size_t max_pq_size)
  {
    // Check for trivial sink-only return on shortcutting the root
    node_stream<> in_nodes(in);
    node_t v = in_nodes.pull();

    if (label_of(v) == label && (is_sink(v.low) || is_sink(v.high))) {
      typename quantify_policy::unreduced_t maybe_resolved = quantify_policy::resolve_sink_root(v, op);

      if (!maybe_resolved.empty()) {
        return maybe_resolved;
      }
    }

    // Set-up for arc_file output
    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_1_t quantify_pq_1({in}, pq_1_memory, max_pq_size);
    pq_2_t quantify_pq_2(pq_2_memory, max_pq_size);

    label_t out_label = label_of(v.uid);
    id_t out_id = 0;

    if (label_of(v.uid) == label) {
      // Precondition: The input is reduced and will not collapse to a sink
      quantify_pq_1.push({ fst(v.low, v.high), snd(v.low, v.high), NIL });
    } else {
      uid_t out_uid = create_node_uid(out_label, out_id++);

      if (is_sink(v.low)) {
        aw.unsafe_push_sink({ out_uid, v.low });
      } else {
        quantify_pq_1.push({ v.low, NIL, out_uid });
      }
      if (is_sink(v.high)) {
        aw.unsafe_push_sink({ flag(out_uid), v.high });
      } else {
        quantify_pq_1.push({ v.high, NIL, flag(out_uid) });
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

      ptr_t source, t1, t2;
      bool with_data = false;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from quantify_pq_1 and quantify_pq_2 (pretty much just as for Apply)
      if (quantify_pq_1.can_pull() && (quantify_pq_2.empty() || quantify_pq_1.top().t1 < quantify_pq_2.top().t2)) {
        with_data = false;
        source = quantify_pq_1.top().source;
        t1 = quantify_pq_1.top().t1;
        t2 = quantify_pq_1.top().t2;

        quantify_pq_1.pop();
      } else {
        with_data = true;
        source = quantify_pq_2.top().source;
        t1 = quantify_pq_2.top().t1;
        t2 = quantify_pq_2.top().t2;

        data_low = quantify_pq_2.top().data_low;
        data_high = quantify_pq_2.top().data_high;

        quantify_pq_2.pop();
      }

      // Seek element from request in stream
      ptr_t t_seek = with_data ? t2 : t1;

      while (v.uid < t_seek) {
        v = in_nodes.pull();
      }

      // Forward information of v.uid == t1 across the level if needed
      if (!with_data && !is_nil(t2) && !is_sink(t2) && label_of(t1) == label_of(t2)) {
        quantify_pq_2.push({ t1, t2, v.low, v.high, source });

        while (quantify_pq_1.can_pull() && (quantify_pq_1.top().t1 == t1 && quantify_pq_1.top().t2 == t2)) {
          source = quantify_pq_1.pull().source;
          quantify_pq_2.push({ t1, t2, v.low, v.high, source });
        }

        continue;
      }

      if (label_of(fst(t1, t2)) == label) {
        // The variable should be quantified: proceed somewhat as for the BDD
        // Restrict algorithm by forwarding the request of source further to the
        // children, though here we keep track of both possibilities.
        adiar_debug(is_nil(t2), "Ended in pairing case on request that already is a pair");

        do {
          __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op, source, v.low, v.high);
        } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2, source, t1, t2));
      } else {
        // The variable should stay: proceed as in the Product Construction by
        // simulating both possibilities in parallel.

        // Resolve current node and recurse.
        ptr_t low1  = with_data ? data_low  : v.low;
        ptr_t high1 = with_data ? data_high : v.high;
        ptr_t low2  = with_data ? v.low     : t2;
        ptr_t high2 = with_data ? v.high    : t2;

        quantify_policy::compute_cofactor(true, low1, high1);
        quantify_policy::compute_cofactor(on_level(out_label, t2), low2, high2);

        adiar_debug(out_id < MAX_ID, "Has run out of ids");
        uid_t out_uid = create_node_uid(out_label, out_id++);

        __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op, out_uid, low1, low2);
        __quantify_resolve_request<quantify_policy>(quantify_pq_1, aw, op, flag(out_uid), high1, high2);

        if (!is_nil(source)) {
          do {
            arc_t out_arc = { source, out_uid };
            aw.unsafe_push_node(out_arc);
          } while (!__quantify_update_source_or_break(quantify_pq_1, quantify_pq_2, source, t1, t2));
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

  template<typename quantify_policy>
  size_t __quantify_max_cut_upper_bound(const typename quantify_policy::reduced_t &in)
  {
    const size_t number_of_nodes = in->size();
    const bits_approximation input_bits(number_of_nodes);

    const bits_approximation bound_bits = input_bits * input_bits + 2;

    if(bound_bits.may_overflow()) {
      return std::numeric_limits<size_t>::max();
    } else {
      return (number_of_nodes * number_of_nodes) + 2;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename quantify_policy>
  typename quantify_policy::unreduced_t quantify(const typename quantify_policy::reduced_t &in,
                                                 const label_t label,
                                                 const bool_op &op)
  {
    adiar_debug(is_commutative(op), "Noncommutative operator used");

    // Check if there is no need to do any computation
    if (is_sink(in) || !quantify_has_label(label, in)) {
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

    const size_t max_pq_size = __quantify_max_cut_upper_bound<quantify_policy>(in);

    constexpr size_t data_structures_in_pq_1 =
      quantify_priority_queue_1_t<internal_sorter, internal_priority_queue>::DATA_STRUCTURES;

    constexpr size_t data_structures_in_pq_2 =
      quantify_priority_queue_2_t<internal_priority_queue>::DATA_STRUCTURES;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      quantify_priority_queue_1_t<internal_sorter, internal_priority_queue>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_internal_memory =
      aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      quantify_priority_queue_2_t<internal_priority_queue>::memory_fits(pq_2_internal_memory);

    // TODO: maximum 1-level cut is sufficient for pq_2!
    if(max_pq_size <= pq_1_memory_fits && max_pq_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_quantify.lpq_internal++;
#endif
      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<internal_sorter, internal_priority_queue>,
                        quantify_priority_queue_2_t<internal_priority_queue>>
        (in, label, op, pq_1_internal_memory, pq_2_internal_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_quantify.lpq_external++;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __quantify<quantify_policy,
                        quantify_priority_queue_1_t<external_sorter, external_priority_queue>,
                        quantify_priority_queue_2_t<external_priority_queue>>
        (in, label, op, pq_1_memory, pq_2_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_QUANTIFY_H
