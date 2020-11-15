#ifndef COOM_QUANTIFY_CPP
#define COOM_QUANTIFY_CPP

#include "quantify.h"

#include <coom/file_stream.h>
#include <coom/file_writer.h>
#include <coom/priority_queue.h>
#include <coom/util.h>

#include <coom/assert.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct tuple
  {
    ptr_t source;
    ptr_t t1;
    ptr_t t2;
  };

  struct tuple_data
  {
    ptr_t source;
    ptr_t t1;
    ptr_t t2;
    ptr_t data_low;
    ptr_t data_high;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  //
  // As such we could just reuse the comparators for Apply, but unlike in Apply
  // we only maintain request for two nodes in the same OBDD. In fact, we can
  // due to this optimise everything by having t1 and t2 in the tuple sorted.
  //
  // This improves the speed of the comparators.
  struct quantify_queue_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return a.t1 < b.t1 || (a.t1 == b.t1 && a.t2 < b.t2);
    }
  };

  struct quantify_queue_label
  {
    label_t label_of(const tuple &t)
    {
      return coom::label_of(t.t1);
    }
  };

  struct quantify_queue_data_lt
  {
    bool operator()(const tuple_data &a, const tuple_data &b)
    {
      return a.t2 < b.t2 || (a.t2 == b.t2 && a.t1 < b.t1);
    }
  };

  typedef node_priority_queue<tuple, quantify_queue_label, quantify_queue_lt> quantify_priority_queue_t;
  typedef tpie::priority_queue<tuple_data, quantify_queue_data_lt> quantify_data_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void quantify_resolve_request(quantify_priority_queue_t &quantD,
                                       arc_writer &aw,
                                       const bool_op &op,
                                       const ptr_t source, const ptr_t r1, const ptr_t r2)
  {
    if (is_nil(r2)) {
      if (is_sink_ptr(r1)) {
        aw.unsafe_push_sink({ source, r1 });
      } else {
        quantD.push({ source, r1, r2 });
      }
    } else if (is_sink_ptr(r1) && can_left_shortcut(op, r1)) {
      arc_t out_arc = { source, op(r1, create_sink_ptr(true)) };
      aw.unsafe_push_sink(out_arc);
    } else if (is_sink_ptr(r2) && can_right_shortcut(op, r2)) {
      arc_t out_arc = { source, op(create_sink_ptr(true), r2) };
      aw.unsafe_push_sink(out_arc);
    } else {
      quantD.push({ source, r1, r2 });
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

  inline bool quantify_has_label(const label_t label, const node_file &bdd)
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
  node_or_arc_file quantify(const node_file &bdd,
                            const label_t label,
                            const bool_op &op)
  {
#if COOM_ASSERT
    assert(is_commutative(op));
#endif

    node_or_arc_file out_union;

    // Check if there is no need to do any computation
    if (is_sink(bdd, is_any) || !quantify_has_label(label, bdd)) {
      return out_union << bdd;
    }

    // Check for trivial sink-only return on shortcutting the root
    node_stream<> in_nodes(bdd);
    node_t v = in_nodes.pull();

    if (label_of(v) == label && (is_sink_ptr(v.low) || (is_sink_ptr(v.high)))) {
      ptr_t res_sink = NIL;

      if (is_sink_ptr(v.low) && (can_left_shortcut(op, v.low) || can_right_shortcut(op, v.low))) {
        res_sink = v.low;
      }

      if (is_sink_ptr(v.high) && (can_left_shortcut(op, v.high) || can_right_shortcut(op, v.high))) {
        res_sink = v.high;
      }

      if (!is_nil(res_sink)) {
        node_file out_nodes;
        node_writer nw(out_nodes);
        nw.unsafe_push(node_t { res_sink, NIL, NIL });
        return out_union << out_nodes;
      }
    }

    // Set-up for arc_file output
    arc_file out_arcs;

    arc_writer aw(out_arcs);

    quantify_data_priority_queue_t quantD_data;

    quantify_priority_queue_t quantD;
    quantD.hook_meta_stream(bdd);

    label_t out_label = label_of(v.uid);
    id_t out_id = 0;

    if (label_of(v.uid) == label) {
      // Precondition: The input is reduced and will not collapse to a sink-only OBDD
      quantD.push({ NIL, std::min(v.low, v.high), std::max(v.low, v.high) });
    } else {
      aw.unsafe_push(meta_t { out_label });

      uid_t out_uid = create_node_uid(out_label, out_id);

      if (is_sink_ptr(v.low)) {
        aw.unsafe_push_sink({ out_uid, v.low });
      } else {
        quantD.push({ out_uid, v.low, NIL });
      }
      if (is_sink_ptr(v.high)) {
        aw.unsafe_push_sink({ flag(out_uid), v.high });
      } else {
        quantD.push({ flag(out_uid), v.high, NIL });
      }
    }

    while(quantD.can_pull() || quantD.has_next_layer() || !quantD_data.empty()) {
      if (!quantD.can_pull() && quantD_data.empty()) {
        quantD.setup_next_layer();
        out_label = quantD.current_layer();
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
      while ((!with_data && v.uid < t1) || (with_data && v.uid < t2)) {
        v = in_nodes.pull();
      }

      // Forward information of v.uid == t1 across the layer if needed
      if (!with_data && !is_nil(t2) && !is_sink_ptr(t2) && label_of(t1) == label_of(t2)) {
        quantD_data.push({ source, t1, t2, v.low, v.high });

        while (quantD.can_pull() && (quantD.top().t1 == t1 && quantD.top().t2 == t2)) {
          source = quantD.pull().source;
          quantD_data.push({ source, t1, t2, v.low, v.high });
        }
        continue;
      }

      // Resolve current node and recurse.
      ptr_t low1  = with_data ? data_low  : v.low;
      ptr_t high1 = with_data ? data_high : v.high;
      ptr_t low2  = with_data ? v.low     : t2;
      ptr_t high2 = with_data ? v.high    : t2;

      // Have two branches collapsed back into one?
      if (low1 == low2) {
        low2 = NIL;
      }
      if (high1 == high2) {
        high2 = NIL;
      }

      if (label_of(std::min(t1, t2)) == label) {
        // The variable should be quantified: proceed as in Restrict by
        // forwarding the request of source further to the children, though here
        // we keep track of both possibilities.
        while (true) {
#if COOM_ASSERT
          assert(is_nil(low2));
          assert(is_nil(high2));
#endif
          quantify_resolve_request(quantD, aw, op,
                                   source,
                                   std::min(low1, high1),
                                   std::max(low1, high1));

          if (quantify_update_source_or_break(quantD, quantD_data, source, t1, t2)) {
            break;
          }
        }
      } else {
        // The variable should stay: proceed as in Apply by simulating both
        // possibilities in parallel.
        uid_t out_uid = create_node_uid(out_label, out_id);
#if COOM_ASSERT
        assert(out_id < MAX_ID);
#endif
        out_id++;

        quantify_resolve_request(quantD, aw, op,
                                 out_uid,
                                 std::min(low1, low2),
                                 std::max(low1, low2));

        quantify_resolve_request(quantD, aw, op,
                                 flag(out_uid),
                                 std::min(high1, high2),
                                 std::max(high1, high2));

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

    return out_union << out_arcs;
  }

  node_file quantify(const node_file &nodes,
                     const label_file &labels,
                     const bool_op &op)
  {
#if COOM_ASSERT
    assert(is_commutative(op));
#endif

    if (labels.size() == 0) {
      return nodes;
    }

    node_file out = nodes;

    // We will quantify the labels in the order they are given.
    label_stream<> ls(labels);
    while(ls.can_pull()) {
      // Did we collapse early to a sink-only OBDD?
      if (is_sink(out, is_any)) {
        break;
      }

      out = quantify(out, ls.pull(), op);
    }
    return out;
  }

  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_exists(const node_file &bdd, const label_t &label)
  {
    return quantify(bdd, label, or_op);
  }

  node_file bdd_exists(const node_file &bdd, const label_file &labels)
  {
    return quantify(bdd, labels, or_op);
  }

  node_or_arc_file bdd_forall(const node_file &bdd, const label_t &label)
  {
    return quantify(bdd, label,  and_op);
  }

  node_file bdd_forall(const node_file &bdd, const label_file &labels)
  {
    return quantify(bdd, labels, and_op);
  }

  node_or_arc_file bdd_unique(const node_file &bdd, const label_t &label)
  {
    return quantify(bdd, label, xor_op);
  }

  node_file bdd_unique(const node_file &bdd, const label_file &labels)
  {
    return quantify(bdd, labels, xor_op);
  }
}

#endif // COOM_QUANTIFY_CPP
