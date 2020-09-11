#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include <tpie/priority_queue.h>
#include <tpie/file_stream.h>

#include "data.h"
#include "reduce.h"

#include "debug.h"
#include "debug_data.h"

#include "assert.h"

#include "apply.h"

namespace coom
{
  namespace debug {
    inline void println_apply_request([[maybe_unused]] ptr_t t1,
                                      [[maybe_unused]] ptr_t t2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << std::endl << "| request: " << std::endl << "|   | ";
      print_child(t1);
      tpie::log_info() << std::endl << "|   | ";
      print_child(t2);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_apply_position([[maybe_unused]] const node &v1,
                                       [[maybe_unused]] const node &v2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  current: " << std::endl << "|     | ";
      print_node(v1);
      tpie::log_info() << std::endl << "|     | ";
      print_node(v2);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_apply_resolution([[maybe_unused]] ptr_t out_uid,
                                         [[maybe_unused]] ptr_t low1,
                                         [[maybe_unused]] ptr_t low2,
                                         [[maybe_unused]] ptr_t high1,
                                         [[maybe_unused]] ptr_t high2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  resolved to: ";
      print_node_ptr(out_uid);
      tpie::log_info() << std::endl << "|     | ";
      debug::print_child(low1);       tpie::log_info() << ",";
      debug::print_child(low2);       tpie::log_info() << std::endl << "|     | ";
      debug::print_child(high1);      tpie::log_info() << ",";
      debug::print_child(high2);      tpie::log_info() << std::endl;
#endif
    }

    inline void println_apply_ingoing([[maybe_unused]] arc& out_arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  in: ";
      debug::println_arc(out_arc);
#endif
    }

    inline void println_apply_done()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "| done..." << std::endl;
#endif
    }

    inline void println_apply_later()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "| later..." << std::endl;
#endif
    }
  }

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

  struct apply_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::min(a.t1, a.t2) < std::min(b.t1, b.t2) ||
            (std::min(a.t1, a.t2) == std::min(b.t1, b.t2) && std::max(a.t1, a.t2) < std::max(b.t1, b.t2));
    }
  };

  struct apply_lt_data
  {
    bool operator()(const tuple_data &a, const tuple_data &b)
    {
      return std::max(a.t1, a.t2) < std::max(b.t1, b.t2) ||
            (std::max(a.t1, a.t2) == std::max(b.t1, b.t2) && std::min(a.t1, a.t2) < std::min(b.t1, b.t2));
    }
  };

  bool can_right_shortcut(const bool_op &op, const ptr_t sink)
  {
    return op(create_sink_ptr(false), sink) == op(create_sink_ptr(true), sink);
  }

  bool can_left_shortcut(const bool_op &op, const ptr_t sink)
  {
    return op(sink, create_sink_ptr(false)) == op(sink, create_sink_ptr(true));
  }

  void apply(tpie::file_stream<node_t> &in_nodes_1,
             tpie::file_stream<node_t> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<arc_t> &reduce_node_arcs,
             tpie::file_stream<arc_t> &reduce_sink_arcs)
  {
    // Set-up
    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    tpie::priority_queue<tuple, apply_lt> appD;
    tpie::priority_queue<tuple_data, apply_lt_data> appD_data;

    node_t v1 = in_nodes_1.read_back();
    node_t v2 = in_nodes_2.read_back();
    id_t out_id = 0;

    // Process root and create initial recursion requests
    ptr_t low1, low2;
    ptr_t high1, high2;

    label_t prior_label = label_of(std::min(v1.uid, v2.uid));
    uid_t root_uid = create_node_uid(prior_label, out_id);

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
    if (is_sink_ptr(low1) && is_sink_ptr(low2)) {
      arc_t new_arc = { root_uid, op(low1, low2) };
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink_ptr(low1) && can_left_shortcut(op, low1)) {
      arc_t new_arc = { root_uid, op(low1, create_sink_ptr(true)) };
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink_ptr(low2) && can_right_shortcut(op, low2)) {
      arc_t new_arc = { root_uid, op(create_sink_ptr(true), low2) };
      reduce_sink_arcs.write(new_arc);
    } else {
      appD.push({ root_uid, low1, low2 });
    }

    if (is_sink_ptr(high1) && is_sink_ptr(high2)) {
      arc_t new_arc = { flag(root_uid), op(high1, high2) };
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink_ptr(high1) && can_left_shortcut(op, high1)) {
      arc_t new_arc = { flag(root_uid), op(high1, create_sink_ptr(true)) };
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink_ptr(high2) && can_right_shortcut(op, high2)) {
      arc_t new_arc = { flag(root_uid), op(create_sink_ptr(true), high2) };
      reduce_sink_arcs.write(new_arc);
    } else {
      appD.push({ flag(root_uid), high1, high2 });
    }

    // Process all nodes in topological order of both OBDDs
    ptr_t source, t1, t2;

    bool with_data, from_1 = false;
    ptr_t data_low = NIL, data_high = NIL;

    while (!appD.empty() || !appD_data.empty()) {
      // Merge requests from  appD or appD_data
      if (!appD.empty() && (appD_data.empty() ||
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

      debug::println_apply_request(t1,t2);

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

      debug::println_apply_position(v1,v2);

      // Forward information across the layer
      if (!with_data
          && !is_sink_ptr(t1) && !is_sink_ptr(t2) && label_of(t1) == label_of(t2)
          && (v1.uid != t1 || v2.uid != t2)) {
        bool from_1 = v1.uid == t1;
        node_t v0 = from_1 ? v1 : v2;

        appD_data.push({ source, t1, t2, from_1, v0.low, v0.high });

        while (!appD.empty() && (appD.top().t1 == t1 && appD.top().t2 == t2)) {
          source = appD.top().source;
          appD.pop();
          appD_data.push({ source, t1, t2, from_1, v0.low, v0.high });
        }
        debug::println_apply_later();
        continue;
      }

      // Resolve current node and recurse
      ptr_t low1;
      ptr_t low2;
      ptr_t high1;
      ptr_t high2;

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

      // Create new node
      label_t out_label = label_of(std::min(t1, t2));
      out_id = prior_label != out_label ? 0 : out_id;
      prior_label = out_label;

      uid_t out_uid = create_node_uid(out_label, out_id);
      debug::println_apply_resolution(out_uid, low1, low2, high1, high2);

      out_id++;

      // Output outgoing sink arcs or recurse
      if (is_sink_ptr(low1) && is_sink_ptr(low2)) {
        arc_t out_arc = { out_uid, op(low1, low2) };
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink_ptr(low1) && can_left_shortcut(op, low1)) {
        arc_t out_arc = { out_uid, op(low1, create_sink_ptr(true)) };
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink_ptr(low2) && can_right_shortcut(op, low2)) {
        arc_t out_arc = { out_uid, op(create_sink_ptr(true), low2) };
        reduce_sink_arcs.write(out_arc);
      } else {
        appD.push({out_uid, low1, low2});
      }

      if (is_sink_ptr(high1) && is_sink_ptr(high2)) {
        arc_t out_arc = { flag(out_uid), op(high1, high2) };
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink_ptr(high1) && can_left_shortcut(op, high1)) {
        arc_t out_arc = { flag(out_uid), op(high1, create_sink_ptr(true)) };
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink_ptr(high2) && can_right_shortcut(op, high2)) {
        arc_t out_arc = { flag(out_uid), op(create_sink_ptr(true), high2) };
        reduce_sink_arcs.write(out_arc);
      } else {
        appD.push({ flag(out_uid), high1, high2 });
      }

      // Output ingoing arcs
      while (true) {
        arc_t out_arc = { source, out_uid };
        reduce_node_arcs.write(out_arc);

        debug::println_apply_ingoing(out_arc);

        if (!appD.empty() && appD.top().t1 == t1 && appD.top().t2 == t2) {
          source = appD.top().source;
          appD.pop();
        } else if (!appD_data.empty() && appD_data.top().t1 == t1 && appD_data.top().t2 == t2) {
          source = appD_data.top().source;
          appD_data.pop();
        } else {
          break;
        }
      }
      debug::println_apply_done();
    }
  }

  void apply(tpie::file_stream<node_t> &in_nodes_1,
             tpie::file_stream<node_t> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<node_t> &out_nodes)
  {
    debug::println_algorithm_start("APPLY");

    assert::is_valid_input_stream(in_nodes_1);
    debug::println_file_stream(in_nodes_1, "in_nodes 1");

    assert::is_valid_input_stream(in_nodes_2);
    debug::println_file_stream(in_nodes_2, "in_nodes 2");

    assert::is_valid_output_stream(out_nodes);

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
      debug::println_file_stream(out_nodes, "out_nodes");
    } else if (is_sink(root_1) && can_left_shortcut(op, root_1.uid)) {
      node_t res_sink_node = {
          op(root_1.uid, create_sink_ptr(false)),
          NIL,
          NIL
      };

      out_nodes.write(res_sink_node);
      debug::println_file_stream(out_nodes, "out_nodes");
    } else if (is_sink(root_2) && can_right_shortcut(op, root_2.uid)) {
      node_t res_sink_node = {
          op(create_sink_ptr(false), root_2.uid),
          NIL,
          NIL
      };

      out_nodes.write(res_sink_node);
      debug::println_file_stream(out_nodes, "out_nodes");
    } else {
      tpie::file_stream<arc_t> reduce_node_arcs;
      reduce_node_arcs.open();

      tpie::file_stream<arc_t> reduce_sink_arcs;
      reduce_sink_arcs.open();

      apply(in_nodes_1, in_nodes_2, op, reduce_node_arcs, reduce_sink_arcs);
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    }
    debug::println_algorithm_end("APPLY");
  }
} // namespace coom

#endif // COOM_APPLY_CPP
