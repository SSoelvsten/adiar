#ifndef ADIAR_QUANTIFY_CPP
#define ADIAR_QUANTIFY_CPP

#include "quantify.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/priority_queue.h>
#include <adiar/tuple.h>
#include <adiar/util.h>

#include <adiar/bdd/build.h>

#include <adiar/assert.h>

namespace adiar
{
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
  // and Homomorphism we only maintain request for two nodes in the same BDD. We
  // can in fact due to this optimise everything by having t1 and t2 in the
  // quantify_tuple sorted.
  //
  // This improves the speed of the comparators.
  struct quantify_1_lt
  {
    bool operator()(const quantify_tuple &a, const quantify_tuple &b)
    {
      return a.t1 < b.t1 || (a.t1 == b.t1 && a.t2 < b.t2);
    }
  };

  struct quantify_2_lt
  {
    bool operator()(const quantify_tuple_data &a, const quantify_tuple_data &b)
    {
      return a.t2 < b.t2 || (a.t2 == b.t2 && a.t1 < b.t1);
    }
  };

  typedef node_priority_queue<quantify_tuple, tuple_label, quantify_1_lt> quantify_priority_queue_t;
  typedef tpie::priority_queue<quantify_tuple_data, quantify_2_lt> quantify_data_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void quantify_resolve_request(quantify_priority_queue_t &quantD,
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
        quantD.push({ r1, r2, source });
      }
    } else {
      // sort the tuple of requests
      ptr_t r_fst = fst(r1,r2);
      ptr_t r_snd = snd(r1,r2);

      if (is_sink(r_fst) /* && is_sink(r_snd) */) {
        arc_t out_arc = { source, op(r_fst, r_snd) };
        aw.unsafe_push_sink(out_arc);
      } else if (is_sink(r_snd) && can_right_shortcut(op, r_snd)) {
        arc_t out_arc = { source, op(create_sink_ptr(true), r_snd) };
        aw.unsafe_push_sink(out_arc);
      } else {
        quantD.push({ r_fst, r_snd, source });
      }
    }
  }

  inline bool quantify_update_source_or_break(quantify_priority_queue_t &quantD,
                                              quantify_data_priority_queue_t &quantD_data,
                                              ptr_t &source,
                                              const ptr_t t1, const ptr_t t2)
  {
    if (quantD.can_pull() && quantD.top().t1 == t1 && quantD.top().t2 == t2) {
      source = quantD.pull().source;
    } else if (!quantD_data.empty() && quantD_data.top().t1 == t1 && quantD_data.top().t2 == t2) {
      source = quantD_data.top().source;
      quantD_data.pop();
    } else {
      return true;
    }
    return false;
  }

  inline bool quantify_has_label(const label_t label, const bdd &bdd)
  {
    meta_stream<node_t, 1> in_meta(bdd);
    while(in_meta.can_pull()) {
      meta_t m = in_meta.pull();

      // Are we already past where it should be?
      if (label < m.label) { return false; }

      // Did we find it?
      if (m.label == label) { return true; }
    }
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd quantify(const bdd &bdd,
                 const label_t label,
                 const bool_op &op)
  {
    adiar_debug(is_commutative(op), "Noncommutative operator used");

    // Check if there is no need to do any computation
    if (is_sink(bdd, is_any) || !quantify_has_label(label, bdd)) {
      return bdd;
    }

    // Check for trivial sink-only return on shortcutting the root
    node_stream<> in_nodes(bdd);
    node_t v = in_nodes.pull();

    if (label_of(v) == label && (is_sink(v.low) || (is_sink(v.high)))) {
      ptr_t res_sink = NIL;

      if (is_sink(v.low) && can_left_shortcut(op, v.low)) {
        res_sink = v.low;
      }

      if (is_sink(v.high) && can_right_shortcut(op, v.high)) {
        res_sink = v.high;
      }

      if (!is_nil(res_sink)) {
        node_file out_nodes;
        node_writer nw(out_nodes);
        nw.unsafe_push(node_t { res_sink, NIL, NIL });
        return out_nodes;
      }
    }

    // Set-up for arc_file output
    arc_file out_arcs;

    arc_writer aw(out_arcs);

    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();
    quantify_priority_queue_t quantD({bdd}, available_memory / 2);
    quantify_data_priority_queue_t quantD_data(calc_tpie_pq_factor(available_memory / 2));

    label_t out_label = label_of(v.uid);
    id_t out_id = 0;

    if (label_of(v.uid) == label) {
      // Precondition: The input is reduced and will not collapse to a sink-only BDD
      quantD.push({ fst(v.low, v.high), snd(v.low, v.high), NIL });
    } else {
      aw.unsafe_push(meta_t { out_label });

      uid_t out_uid = create_node_uid(out_label, out_id);

      if (is_sink(v.low)) {
        aw.unsafe_push_sink({ out_uid, v.low });
      } else {
        quantD.push({ v.low, NIL, out_uid });
      }
      if (is_sink(v.high)) {
        aw.unsafe_push_sink({ flag(out_uid), v.high });
      } else {
        quantD.push({ v.high, NIL, flag(out_uid) });
      }
    }

    while(quantD.can_pull() || quantD.has_next_level() || !quantD_data.empty()) {
      if (!quantD.can_pull() && quantD_data.empty()) {
        quantD.setup_next_level();
        out_label = quantD.current_level();
        out_id = 0;

        if (out_label != label) {
          aw.unsafe_push(meta_t { out_label });
        }
      }

      ptr_t source, t1, t2;
      bool with_data = false;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from quantD and quantD_data (pretty much just as for Apply)
      if (quantD.can_pull() && (quantD_data.empty() || quantD.top().t1 < quantD_data.top().t2)) {
        with_data = false;
        source = quantD.top().source;
        t1 = quantD.top().t1;
        t2 = quantD.top().t2;

        quantD.pop();
      } else {
        with_data = true;
        source = quantD_data.top().source;
        t1 = quantD_data.top().t1;
        t2 = quantD_data.top().t2;

        data_low = quantD_data.top().data_low;
        data_high = quantD_data.top().data_high;

        quantD_data.pop();
      }

      // Seek element from request in stream
      ptr_t t_seek = with_data ? t2 : t1;

      while (v.uid < t_seek) {
        v = in_nodes.pull();
      }

      // Forward information of v.uid == t1 across the level if needed
      if (!with_data && !is_nil(t2) && !is_sink(t2) && label_of(t1) == label_of(t2)) {
        quantD_data.push({ t1, t2, v.low, v.high, source });

        while (quantD.can_pull() && (quantD.top().t1 == t1 && quantD.top().t2 == t2)) {
          source = quantD.pull().source;
          quantD_data.push({ t1, t2, v.low, v.high, source });
        }
        continue;
      }

      if (label_of(fst(t1, t2)) == label) {
        // The variable should be quantified: proceed as in Restrict by
        // forwarding the request of source further to the children, though here
        // we keep track of both possibilities.
        adiar_debug(is_nil(t2), "Ended in pairing case on request that already is a pair");

        do {
          quantify_resolve_request(quantD, aw, op, source, v.low, v.high);
        } while (!quantify_update_source_or_break(quantD, quantD_data, source, t1, t2));
      } else {
        // The variable should stay: proceed as in Apply by simulating both
        // possibilities in parallel.

        // Resolve current node and recurse.
        ptr_t low1  = with_data ? data_low  : v.low;
        ptr_t high1 = with_data ? data_high : v.high;
        ptr_t low2  = with_data ? v.low     : t2;
        ptr_t high2 = with_data ? v.high    : t2;

        adiar_debug(out_id < MAX_ID, "Has run out of ids");
        uid_t out_uid = create_node_uid(out_label, out_id++);

        quantify_resolve_request(quantD, aw, op, out_uid, low1, low2);
        quantify_resolve_request(quantD, aw, op, flag(out_uid), high1, high2);

        if (!is_nil(source)) {
          do {
            arc_t out_arc = { source, out_uid };
            aw.unsafe_push_node(out_arc);
          } while (!quantify_update_source_or_break(quantD, quantD_data, source, t1, t2));
        }
      }
    }

    // TODO: Add bool variable to check whether we really do need to sort.
    aw.sort_sinks();

    return out_arcs;
  }

# define multi_quantify_macro(bdd_var, labels, op)            \
  if (labels.size() == 0) { return bdd_var; }                 \
  label_stream<> ls(labels);                                  \
  while(true) {                                               \
    if (is_sink(bdd_var, is_any)) { return bdd_var; }         \
                                                              \
    label_t label = ls.pull();                                \
    if (!ls.can_pull()) {                                     \
      return quantify(bdd_var, label, op);                    \
    } else {                                                  \
      bdd_var = quantify(bdd_var, label, op);                 \
    }                                                         \
 }                                                            \

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, const label_t &label)
  {
    return quantify(in_bdd, label, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, or_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_t &label)
  {
    return quantify(in_bdd, label, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, and_op);
  }
}

#endif // ADIAR_QUANTIFY_CPP
