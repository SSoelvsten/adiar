#ifndef COOM_QUANTIFY_CPP
#define COOM_QUANTIFY_CPP

#include "quantify.h"

#include <coom/priority_queue.h>
#include <coom/util.h>

#include <coom/reduce.h>

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

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void quantify_resolve_request(priority_queue<tuple, quantify_queue_label, quantify_queue_lt, 1> &quantD,
                                       tpie::file_stream<arc_t> &reduce_sink_arcs,
                                       bool_op op,
                                       ptr_t source, ptr_t r1, ptr_t r2)
  {
    if (is_nil(r2)) {
      if (is_sink_ptr(r1)) {
        reduce_sink_arcs.write({ source, r1 });
      } else {
        quantD.push({ source, r1, r2 });
      }
    } else if (is_sink_ptr(r1) && is_sink_ptr(r2)) {
      arc_t out_arc = { source, op(r1, r2) };
      reduce_sink_arcs.write(out_arc);
    } else if (is_sink_ptr(r1) && can_left_shortcut(op, r1)) {
      arc_t out_arc = { source, op(r1, create_sink_ptr(true)) };
      reduce_sink_arcs.write(out_arc);
    } else if (is_sink_ptr(r2) && can_right_shortcut(op, r2)) {
      arc_t out_arc = { source, op(create_sink_ptr(true), r2) };
      reduce_sink_arcs.write(out_arc);
    } else {
      quantD.push({ source, r1, r2 });
    }
  }

  inline bool quantify_update_source_or_break(priority_queue<tuple, quantify_queue_label, quantify_queue_lt, 1> &quantD,
                                              tpie::priority_queue<tuple_data, quantify_queue_data_lt> &quantD_data,
                                              ptr_t &source, ptr_t t1, ptr_t t2)
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

  inline bool quantify_has_label(label_t label, tpie::file_stream<meta_t> &in_meta)
  {
    bool has_label = false;

    in_meta.seek(0);
    while(in_meta.can_read() && !has_label) {
      meta_t m = in_meta.read();
      has_label = m.label == label;
    }
    return has_label;
  }

  inline bool quantify_can_shortcut_root(label_t label,
                                         tpie::file_stream<node_t> &in_nodes,
                                         bool_op op,
                                         ptr_t &res_sink)
  {
    res_sink = NIL;

    in_nodes.seek(0, tpie::file_stream_base::end);
    node_t v = in_nodes.read_back();

    if (label_of(v) == label && (is_sink_ptr(v.low) || (is_sink_ptr(v.high)))) {
      if (is_sink_ptr(v.low) && (can_left_shortcut(op, v.low) || can_right_shortcut(op, v.low))) {
        res_sink = v.low;
      }

      if (is_sink_ptr(v.high) && (can_left_shortcut(op, v.high) || can_right_shortcut(op, v.high))) {
        res_sink = v.high;
      }
    }
    return !is_nil(res_sink);
  }

  //////////////////////////////////////////////////////////////////////////////
  void quantify(label_t label,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<arc_t> &reduce_node_arcs,
                tpie::file_stream<arc_t> &reduce_sink_arcs,
                tpie::file_stream<meta_t> &reduce_meta)
  {
    // Set-up
    in_nodes.seek(0, tpie::file_stream_base::end);

    tpie::priority_queue<tuple_data, quantify_queue_data_lt> quantD_data;
    priority_queue<tuple, quantify_queue_label, quantify_queue_lt> quantD;
    quantD.hook_meta_stream(in_meta);

    node_t v = in_nodes.read_back();

    label_t out_label = label_of(v.uid);
    id_t out_id = 0;

    if (label_of(v.uid) == label) {
      // Precondition: The input is reduced and will not collapse to a sink-only OBDD
      quantD.push({ NIL, std::min(v.low, v.high), std::max(v.low, v.high) });
    } else {
      reduce_meta.write({ out_label });

      uid_t out_uid = create_node_uid(out_label, out_id);

      if (is_sink_ptr(v.low)) {
        reduce_sink_arcs.write({ out_uid, v.low });
      } else {
        quantD.push({ out_uid, v.low, NIL });
      }
      if (is_sink_ptr(v.high)) {
        reduce_sink_arcs.write({ flag(out_uid), v.high });
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
          reduce_meta.write({ out_label });
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
        v = in_nodes.read_back();
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
          quantify_resolve_request(quantD, reduce_sink_arcs, op,
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

        quantify_resolve_request(quantD, reduce_sink_arcs, op,
                                 out_uid,
                                 std::min(low1, low2),
                                 std::max(low1, low2));

        quantify_resolve_request(quantD, reduce_sink_arcs, op,
                                 flag(out_uid),
                                 std::min(high1, high2),
                                 std::max(high1, high2));

        if (!is_nil(source)) {
          while (true) {
            arc_t out_arc = { source, out_uid };
            reduce_node_arcs.write(out_arc);

            if (quantify_update_source_or_break(quantD, quantD_data, source, t1, t2)) {
              break;
            }
          }
        }
      }
    }

    tpie::progress_indicator_null pi;
    tpie::sort(reduce_sink_arcs, by_source_lt, pi);
  }

  void quantify(label_t label,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta)
  {
    assert::is_valid_input_stream(in_nodes);
    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

#if COOM_ASSERT
    assert(is_commutative(op));
#endif

    // Check if there is no need to do all the computation
    if (is_sink(in_nodes, is_any) || !quantify_has_label(label, in_meta)) {
      internal::copy(in_nodes, out_nodes);
      internal::copy(in_meta, out_meta);

      return;
    }

    ptr_t res_sink = NIL;
    if (quantify_can_shortcut_root(label, in_nodes, op, res_sink)) {
      out_nodes.write({ res_sink, NIL, NIL });
      return;
    }

    tpie::file_stream<arc_t> reduce_node_arcs;
    tpie::file_stream<arc_t> reduce_sink_arcs;
    tpie::file_stream<meta_t> reduce_meta;

    reduce_node_arcs.open();
    reduce_sink_arcs.open();
    reduce_meta.open();

    quantify(label,
             in_nodes, in_meta,
             op,
             reduce_node_arcs, reduce_sink_arcs, reduce_meta);

    reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta,
           out_nodes, out_meta);
  }

  void quantify(tpie::file_stream<label_t> &labels,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta)
  {
    assert::is_valid_input_stream(in_nodes);
    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

    if (labels.size() == 0) {
      internal::copy(in_nodes, out_nodes);
      internal::copy(in_meta, out_meta);

      return;
    } else if (labels.size() == 1) {
      label_t label = labels.can_read() ? labels.read() : labels.read_back();
      quantify(label, in_nodes, in_meta, op, out_nodes, out_meta);
      return;
    }

#if COOM_ASSERT
    assert(is_commutative(op));
#endif

    // We will quantify the labels in the order they are given.
    labels.seek(0);

    tpie::file_stream<arc_t> reduce_node_arcs;
    tpie::file_stream<arc_t> reduce_sink_arcs;
    tpie::file_stream<meta_t> reduce_meta;

    tpie::file_stream<node_t> temp_nodes;
    tpie::file_stream<meta_t> temp_meta;

    bool first, last;
    do {
      first = !labels.can_read_back();

      // Did we collapse early to a sink-only OBDD?
      if (is_sink(first ? in_nodes : temp_nodes, is_any)) {
        internal::copy(first ? in_nodes : temp_nodes, out_nodes);
        internal::copy(first ? in_meta : temp_meta, out_meta);
        break;
      }

      label_t label = labels.read();

      // Will it trivially collapse to the sink-only OBDD?
      ptr_t res_sink = NIL;
      if (quantify_can_shortcut_root(label, in_nodes, op, res_sink)) {
        out_nodes.write({ res_sink, NIL, NIL });
        return;
      }

      reduce_node_arcs.open();
      reduce_sink_arcs.open();
      reduce_meta.open();

      quantify(label,
               first ? in_nodes : temp_nodes,
               first ? in_meta : temp_meta,
               op,
               reduce_node_arcs, reduce_sink_arcs, reduce_meta);

      last = !labels.can_read();
      if (last) {
        temp_nodes.close();
        temp_meta.close();
      } else {
        // Also closes (and cleans up) the prior temporary stream
        temp_nodes.open();
        temp_meta.open();
      }

      reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta,
             last ? out_nodes : temp_nodes,
             last ? out_meta : temp_meta);

      reduce_node_arcs.close();
      reduce_sink_arcs.close();
      reduce_meta.close();
    } while(!last);
  }

  //////////////////////////////////////////////////////////////////////////////
  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    quantify(label, in_nodes, in_meta, or_op, out_nodes, out_meta);
  }

  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<arc_t> &reduce_node_arcs,
              tpie::file_stream<arc_t> &reduce_sink_arcs,
              tpie::file_stream<meta_t> &reduce_meta)
  {
    quantify(label, in_nodes, in_meta, or_op,
             reduce_node_arcs, reduce_sink_arcs, reduce_meta);
  }

  void exists(tpie::file_stream<label_t> &labels,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    quantify(labels, in_nodes, in_meta, or_op, out_nodes, out_meta);
  }

  void forall(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    quantify(label, in_nodes, in_meta, and_op, out_nodes, out_meta);
  }

  void forall(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<arc_t> &reduce_node_arcs,
              tpie::file_stream<arc_t> &reduce_sink_arcs,
              tpie::file_stream<meta_t> &reduce_meta)
  {
    quantify(label, in_nodes, in_meta, and_op,
             reduce_node_arcs, reduce_sink_arcs, reduce_meta);
  }

  void forall(tpie::file_stream<label_t> &labels,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    quantify(labels, in_nodes, in_meta, and_op, out_nodes, out_meta);
  }
}

#endif // COOM_QUANTIFY_CPP
