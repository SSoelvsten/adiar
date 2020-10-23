#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include "apply.h"

#include <coom/priority_queue.h>
#include <coom/reduce.h>

#include <assert.h>
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
    bool from_1;
    ptr_t data_low;
    ptr_t data_high;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct apply_queue_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::min(a.t1, a.t2) < std::min(b.t1, b.t2) ||
            (std::min(a.t1, a.t2) == std::min(b.t1, b.t2) && std::max(a.t1, a.t2) < std::max(b.t1, b.t2));
    }
  };

  struct apply_queue_label
  {
    label_t label_of(const tuple &t)
    {
      return coom::label_of(std::min(t.t1, t.t2));
    }
  };

  struct apply_queue_data_lt
  {
    bool operator()(const tuple_data &a, const tuple_data &b)
    {
      return std::max(a.t1, a.t2) < std::max(b.t1, b.t2) ||
            (std::max(a.t1, a.t2) == std::max(b.t1, b.t2) && std::min(a.t1, a.t2) < std::min(b.t1, b.t2));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void apply_resolve_request(priority_queue<tuple, apply_queue_label, apply_queue_lt, 2> &appD,
                                    tpie::file_stream<arc_t> &reduce_sink_arcs,
                                    const bool_op &op,
                                    ptr_t source, ptr_t r1, ptr_t r2)
  {
    if (is_sink_ptr(r1) && is_sink_ptr(r2)) {
      arc_t out_arc = { source, op(r1, r2) };
      reduce_sink_arcs.write(out_arc);
    } else if (is_sink_ptr(r1) && can_left_shortcut(op, r1)) {
      arc_t out_arc = { source, op(r1, create_sink_ptr(true)) };
      reduce_sink_arcs.write(out_arc);
    } else if (is_sink_ptr(r2) && can_right_shortcut(op, r2)) {
      arc_t out_arc = { source, op(create_sink_ptr(true), r2) };
      reduce_sink_arcs.write(out_arc);
    } else {
      appD.push({ source, r1, r2 });
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  void apply(tpie::file_stream<node_t> &in_nodes_1,
             tpie::file_stream<meta_t> &in_meta_1,
             tpie::file_stream<node_t> &in_nodes_2,
             tpie::file_stream<meta_t> &in_meta_2,
             const bool_op &op,
             tpie::file_stream<arc_t> &reduce_node_arcs,
             tpie::file_stream<arc_t> &reduce_sink_arcs,
             tpie::file_stream<meta_t> &reduce_meta)
  {
    // Set-up
    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    tpie::priority_queue<tuple_data, apply_queue_data_lt> appD_data;
    priority_queue<tuple, apply_queue_label, apply_queue_lt, 2> appD;
    appD.hook_meta_stream(in_meta_1);
    appD.hook_meta_stream(in_meta_2);

    node_t v1 = in_nodes_1.read_back();
    node_t v2 = in_nodes_2.read_back();

    label_t out_label = label_of(std::min(v1.uid, v2.uid));
    id_t out_id = 0;

    // Process root and create initial recursion requests
    ptr_t low1, low2;
    ptr_t high1, high2;

    uid_t root_uid = create_node_uid(out_label, out_id);
    reduce_meta.write({ out_label });

    if (is_sink(v1)) {
      low1 = high1 = v1.uid;
      low2 = v2.low;
      high2 = v2.high;

      if (in_nodes_2.can_read_back()) {
        v2 = in_nodes_2.read_back();
      }
    } else if (is_sink(v2)) {
      low1 = v1.low;
      high1 = v1.high;
      low2 = high2 = v2.uid;

      if (in_nodes_1.can_read_back()) {
        v1 = in_nodes_1.read_back();
      }
    } else  if (label_of(v1) < label_of(v2)) {
      low1 = v1.low;
      high1 = v1.high;
      low2 = v2.uid;
      high2 = v2.uid;

      if (in_nodes_1.can_read_back()) {
        v1 = in_nodes_1.read_back();
      }
    } else if (label_of(v1) > label_of(v2)) {
      low1 = v1.uid;
      high1 = v1.uid;
      low2 = v2.low;
      high2 = v2.high;

      if (in_nodes_2.can_read_back()) {
        v2 = in_nodes_2.read_back();
      }
    } else {
      low1 = v1.low;
      high1 = v1.high;
      low2 = v2.low;
      high2 = v2.high;

      if (in_nodes_1.can_read_back()) {
        v1 = in_nodes_1.read_back();
      }
      if (in_nodes_2.can_read_back()) {
        v2 = in_nodes_2.read_back();
      }
    }

    // Shortcut the root
    apply_resolve_request(appD, reduce_sink_arcs, op, root_uid, low1, low2);
    apply_resolve_request(appD, reduce_sink_arcs, op, flag(root_uid), high1, high2);

    // Process all nodes in topological order of both OBDDs
    while (appD.can_pull() || appD.has_next_layer() || !appD_data.empty()) {
      if (!appD.can_pull() && appD_data.empty()) {
        appD.setup_next_layer();
        out_label = appD.current_layer();
        reduce_meta.write({ out_label });
        out_id = 0;
      }

      ptr_t source, t1, t2;
      bool with_data, from_1 = false;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from  appD or appD_data
      if (appD.can_pull() && (appD_data.empty() ||
                              std::min(appD.top().t1, appD.top().t2) <
                              std::max(appD_data.top().t1, appD_data.top().t2))) {
        with_data = false;
        source = appD.top().source;
        t1 = appD.top().t1;
        t2 = appD.top().t2;

        appD.pop();
      } else {
        with_data = true;
        source = appD_data.top().source;
        t1 = appD_data.top().t1;
        t2 = appD_data.top().t2;

        from_1 = appD_data.top().from_1;
        data_low = appD_data.top().data_low;
        data_high = appD_data.top().data_high;

        appD_data.pop();
      }

      // Seek request partially in stream
      if (with_data) {
        if (from_1) {
          while (v2.uid < t2) {
            v2 = in_nodes_2.read_back();
          }
        } else {
          while (v1.uid < t1) {
            v1 = in_nodes_1.read_back();
          }
        }
      } else {
        if (label_of(t1) == label_of(t2)) {
          while (label_of(v1.uid) < label_of(t1)) {
            v1 = in_nodes_1.read_back();
          }
          while (label_of(v2.uid) < label_of(t2)) {
            v2 = in_nodes_2.read_back();
          }
        }

        if (is_sink_ptr(t1) || (t1 == v1.uid && std::min(t1,t2) == t2)) {
          while (v2.uid < t2) {
            v2 = in_nodes_2.read_back();
          }
        } else if (is_sink_ptr(t2) || (t2 == v2.uid && std::min(t1,t2) == t1)) {
          while (v1.uid < t1) {
            v1 = in_nodes_1.read_back();
          }
        } else if (t1 == t2) {
          while (v1.uid < t1) {
            v1 = in_nodes_1.read_back();
          }
          while (v2.uid < t2) {
            v2 = in_nodes_2.read_back();
          }
        } else if (t1 < t2) {
          while (v1.uid < t1) {
            v1 = in_nodes_1.read_back();
          }
        } else {
          while (v2.uid < t2) {
            v2 = in_nodes_2.read_back();
          }
        }
      }

      // Forward information across the layer
      if (!with_data
          && !is_sink_ptr(t1) && !is_sink_ptr(t2) && label_of(t1) == label_of(t2)
          && (v1.uid != t1 || v2.uid != t2)) {
        bool from_1 = v1.uid == t1;
        node_t v0 = from_1 ? v1 : v2;

        appD_data.push({ source, t1, t2, from_1, v0.low, v0.high });

        while (appD.can_pull() && (appD.top().t1 == t1 && appD.top().t2 == t2)) {
          source = appD.pull().source;
          appD_data.push({ source, t1, t2, from_1, v0.low, v0.high });
        }
        continue;
      }

      // Resolve current node and recurse
      ptr_t low1, low2, high1, high2;

      if (is_sink_ptr(t1) || is_sink_ptr(t2) || label_of(t1) != label_of(t2)) {
        if (t1 < t2) { // ==> label_of(t1) < label_of(t2) || is_sink_ptr(t2)
          low1 = v1.low;
          high1 = v1.high;
          low2 = t2;
          high2 = t2;
        } else { // ==> label_of(t1) > label_of(t2) || is_sink_ptr(t1)
          low1 = t1;
          high1 = t1;
          low2 = v2.low;
          high2 = v2.high;
        }
      } else {
        low1 = with_data && from_1 ? data_low : v1.low;
        high1 = with_data && from_1 ? data_high : v1.high;
        low2 = with_data && !from_1 ? data_low : v2.low;
        high2 = with_data && !from_1 ? data_high : v2.high;
      }

      // Resolve request
      uid_t out_uid = create_node_uid(out_label, out_id);

      out_id++;

      apply_resolve_request(appD, reduce_sink_arcs, op, out_uid, low1, low2);
      apply_resolve_request(appD, reduce_sink_arcs, op, flag(out_uid), high1, high2);

      // Output ingoing arcs
      while (true) {
        arc_t out_arc = { source, out_uid };
        reduce_node_arcs.write(out_arc);

        if (appD.can_pull() && appD.top().t1 == t1 && appD.top().t2 == t2) {
          source = appD.pull().source;
        } else if (!appD_data.empty() && appD_data.top().t1 == t1 && appD_data.top().t2 == t2) {
          source = appD_data.top().source;
          appD_data.pop();
        } else {
          break;
        }
      }
    }
  }

  void apply(tpie::file_stream<node_t> &in_nodes_1,
             tpie::file_stream<meta_t> &in_meta_1,
             tpie::file_stream<node_t> &in_nodes_2,
             tpie::file_stream<meta_t> &in_meta_2,
             const bool_op &op,
             tpie::file_stream<node_t> &out_nodes,
             tpie::file_stream<meta_t> &out_meta)
  {
    assert::is_valid_input_stream(in_nodes_1);
    assert::is_valid_input_stream(in_nodes_2);
    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    node_t root_1 = in_nodes_1.read_back();
    node_t root_2 = in_nodes_2.read_back();

    if (is_sink(root_1) && is_sink(root_2)) {
      node_t res_sink_node = node{
          op(root_1.uid, root_2.uid),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);
    } else if (is_sink(root_1) && can_left_shortcut(op, root_1.uid)) {
      node_t res_sink_node = {
          op(root_1.uid, create_sink_ptr(false)),
          NIL,
          NIL
      };

      out_nodes.write(res_sink_node);
    } else if (is_sink(root_2) && can_right_shortcut(op, root_2.uid)) {
      node_t res_sink_node = {
          op(create_sink_ptr(false), root_2.uid),
          NIL,
          NIL
      };

      out_nodes.write(res_sink_node);
    } else {
      tpie::file_stream<arc_t> reduce_node_arcs;
      reduce_node_arcs.open();

      tpie::file_stream<arc_t> reduce_sink_arcs;
      reduce_sink_arcs.open();

      tpie::file_stream<meta_t> reduce_meta;
      reduce_meta.open();

      apply(in_nodes_1, in_meta_1, in_nodes_2, in_meta_2, op, reduce_node_arcs, reduce_sink_arcs, reduce_meta);
      reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta, out_nodes, out_meta);
    }
  }
} // namespace coom

#endif // COOM_APPLY_CPP
