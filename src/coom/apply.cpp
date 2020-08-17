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
    inline void println_apply_request([[maybe_unused]] uint64_t t1,
                                      [[maybe_unused]] uint64_t t2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << std::endl << "| request: " << std::endl << "|   | ";
      print_child(t1);
      tpie::log_info() << std::endl << "|   | ";
      print_child(t2);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_apply_position([[maybe_unused]] const node& v1,
                                       [[maybe_unused]] const node& v2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  current: " << std::endl << "|     | ";
      print_node(v1);
      tpie::log_info() << std::endl << "|     | ";
      print_node(v2);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_apply_resolution([[maybe_unused]] uint64_t out_node_ptr,
                                         [[maybe_unused]] uint64_t low1,
                                         [[maybe_unused]] uint64_t low2,
                                         [[maybe_unused]] uint64_t high1,
                                         [[maybe_unused]] uint64_t high2)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  resolved to: ";
      print_node_ptr(out_node_ptr);
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
    uint64_t source;
    uint64_t t1;
    uint64_t t2;
    bool is_high;
  };

  struct tuple_data
  {
    uint64_t source;
    uint64_t t1;
    uint64_t t2;
    bool is_high;
    node data;
    bool from_1;
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

  bool can_right_shortcut(const bool_op &op, const uint64_t sink)
  {
    return op(create_sink(false), sink) == op(create_sink(true), sink);
  }

  bool can_left_shortcut(const bool_op &op, const uint64_t sink)
  {
    return op(sink, create_sink(false)) == op(sink, create_sink(true));
  }

  void apply(tpie::file_stream<node> &in_nodes_1,
             tpie::file_stream<node> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<arc> &reduce_node_arcs,
             tpie::file_stream<arc> &reduce_sink_arcs)
  {
    // Set-up
    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    tpie::priority_queue<tuple, apply_lt> appD;
    tpie::priority_queue<tuple_data, apply_lt_data> appD_data;

    node v1 = in_nodes_1.read_back();
    node v2 = in_nodes_2.read_back();
    uint64_t out_index = 0;

    // Process root and create initial recursion requests
    uint64_t low1, low2;
    uint64_t high1, high2;

    uint64_t prior_label = label_of(std::min(v1.node_ptr, v2.node_ptr));
    uint64_t root = create_node_ptr(prior_label, out_index);

    if (is_sink_node(v1)) {
      low1 = high1 = v1.node_ptr;
      low2 = v2.low;
      high2 = v2.high;

      if (in_nodes_2.can_read_back()) {
        v2 = in_nodes_2.read_back();
      }
    } else if (is_sink_node(v2)) {
      low1 = v1.low;
      high1 = v1.high;
      low2 = high2 = v2.node_ptr;

      if (in_nodes_1.can_read_back()) {
        v1 = in_nodes_1.read_back();
      }
    } else  if (label_of(v1) < label_of(v2)) {
      low1 = v1.low;
      high1 = v1.high;
      low2 = v2.node_ptr;
      high2 = v2.node_ptr;

      if (in_nodes_1.can_read_back()) {
        v1 = in_nodes_1.read_back();
      }
    } else if (label_of(v1) > label_of(v2)) {
      low1 = v1.node_ptr;
      high1 = v1.node_ptr;
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
    if (is_sink(low1) && is_sink(low2)) {
      arc new_arc = create_arc(root, false, op(low1, low2));
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink(low1) && can_left_shortcut(op, low1)) {
      arc new_arc = create_arc(root, false, op(low1, create_sink(true)));
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink(low2) && can_right_shortcut(op, low2)) {
      arc new_arc = create_arc(root, false, op(create_sink(true), low2));
      reduce_sink_arcs.write(new_arc);
    } else {
      appD.push({root, low1, low2, false});
    }

    if (is_sink(high1) && is_sink(high2)) {
      arc new_arc = create_arc(root, true, op(high1, high2));
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink(high1) && can_left_shortcut(op, high1)) {
      arc new_arc = create_arc(root, true, op(high1, create_sink(true)));
      reduce_sink_arcs.write(new_arc);
    } else if (is_sink(high2) && can_right_shortcut(op, high2)) {
      arc new_arc = create_arc(root, true, op(create_sink(true), high2));
      reduce_sink_arcs.write(new_arc);
    } else {
      appD.push({root, high1, high2, true});
    }

    // Process all nodes in topological order of both OBDDs
    uint64_t source;
    uint64_t t1, t2;
    bool is_high;

    bool with_data, from_1 = false;
    node data;

    while (!appD.empty() || !appD_data.empty()) {
      // Merge requests from  appD or appD_data
      if (!appD.empty() && (appD_data.empty() ||
                            std::min(appD.top().t1, appD.top().t2) <
                            std::max(appD_data.top().t1, appD_data.top().t2))) {
        with_data = false;
        source = appD.top().source;
        t1 = appD.top().t1;
        t2 = appD.top().t2;
        is_high = appD.top().is_high;

        appD.pop();
      } else {
        with_data = true;
        source = appD_data.top().source;
        t1 = appD_data.top().t1;
        t2 = appD_data.top().t2;
        is_high = appD_data.top().is_high;
        data = appD_data.top().data;
        from_1 = appD_data.top().from_1;

        appD_data.pop();
      }

      debug::println_apply_request(t1,t2);

      // Seek request partially in stream
      if (label_of(t1) == label_of(t2)) {
        while (label_of(v1.node_ptr) < label_of(t1)) {
          v1 = in_nodes_1.read_back();
        }
        while (label_of(v2.node_ptr) < label_of(t2)) {
          v2 = in_nodes_2.read_back();
        }
      }

      if (t1 == t2) {
        while (v1.node_ptr < t1) {
          v1 = in_nodes_1.read_back();
        }
        while (v2.node_ptr < t2) {
          v2 = in_nodes_2.read_back();
        }
      } else if (with_data) {
        if (from_1) {
          while (v2.node_ptr < t2) {
            v2 = in_nodes_2.read_back();
          }
        } else {
          while (v1.node_ptr < t1) {
            v1 = in_nodes_1.read_back();
          }
        }
      } else {
        if (t1 < t2) {
          while (v1.node_ptr < t1) {
            v1 = in_nodes_1.read_back();
          }
        } else {
          while (v2.node_ptr < t2) {
            v2 = in_nodes_2.read_back();
          }
        }
      }

      debug::println_apply_position(v1,v2);

      // Forward information across the layer
      if (label_of(t1) == label_of(t2)
          && (v1.node_ptr != t1 || v2.node_ptr != t2)
          && !with_data) {
        from_1 = v1.node_ptr == t1;
        node v0 = from_1 ? v1 : v2;

        appD_data.push({source, t1, t2, is_high, v0, from_1});

        while (!appD.empty() && (appD.top().t1 == t1 && appD.top().t2 == t2)) {
          source = appD.top().source;
          is_high = appD.top().is_high;
          appD.pop();
          appD_data.push({source, t1, t2, is_high, v0, from_1});
        }
        debug::println_apply_later();
        continue;
      }

      // Resolve current node and recurse
      uint64_t out_label = std::min(label_of(t1), label_of(t2));
      uint64_t low1;
      uint64_t low2;
      uint64_t high1;
      uint64_t high2;

      if (label_of(t1) != label_of(t2)) {
        if (t1 < t2) { // ==> label_of(t1) < label_of(t2)
          low1 = v1.low;
          high1 = v1.high;
          low2 = t2;
          high2 = t2;
        } else { // ==> label_of(t1) > label_of(t2)
          low1 = t1;
          high1 = t1;
          low2 = v2.low;
          high2 = v2.high;
        }
      } else {
        node v1a = with_data && from_1 ? data : v1;
        node v2a = with_data && !from_1 ? data : v2;

        low1 = v1a.low;
        high1 = v1a.high;
        low2 = v2a.low;
        high2 = v2a.high;
      }

      // Create new node
      out_index = prior_label != out_label ? 0 : out_index;
      prior_label = out_label;

      uint64_t out_node_ptr = create_node_ptr(out_label, out_index);
      debug::println_apply_resolution(out_node_ptr, low1, low2, high1, high2);

      out_index++;

      // Output outgoing sink arcs or recurse
      if (is_sink(low1) && is_sink(low2)) {
        arc out_arc = create_arc(out_node_ptr, false, op(low1, low2));
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink(low1) && can_left_shortcut(op, low1)) {
        arc out_arc = create_arc(out_node_ptr, false, op(low1, create_sink(true)));
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink(low2) && can_right_shortcut(op, low2)) {
        arc out_arc = create_arc(out_node_ptr, false, op(create_sink(true), low2));
        reduce_sink_arcs.write(out_arc);
      } else {
        appD.push({out_node_ptr, low1, low2, false});
      }

      if (is_sink(high1) && is_sink(high2)) {
        arc out_arc = create_arc(out_node_ptr, true, op(high1, high2));
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink(high1) && can_left_shortcut(op, high1)) {
        arc out_arc = create_arc(out_node_ptr, true, op(high1, create_sink(true)));
        reduce_sink_arcs.write(out_arc);
      } else if (is_sink(high2) && can_right_shortcut(op, high2)) {
        arc out_arc = create_arc(out_node_ptr, true, op(create_sink(true), high2));
        reduce_sink_arcs.write(out_arc);
      } else {
        appD.push({out_node_ptr, high1, high2, true});
      }

      // Output ingoing arcs
      while (true) {
        arc out_arc = create_arc(source, is_high, out_node_ptr);
        reduce_node_arcs.write(out_arc);
        debug::println_apply_ingoing(out_arc);

        if (!appD.empty() && appD.top().t1 == t1 && appD.top().t2 == t2) {
          source = appD.top().source;
          t1 = appD.top().t1;
          t2 = appD.top().t2;
          is_high = appD.top().is_high;
          appD.pop();
        } else if (!appD_data.empty() && appD_data.top().t1 == t1 && appD_data.top().t2 == t2) {
          source = appD_data.top().source;
          t1 = appD_data.top().t1;
          t2 = appD_data.top().t2;
          is_high = appD_data.top().is_high;
          appD_data.pop();
        } else {
          break;
        }
      }
      debug::println_apply_done();
    }
  }

  void apply(tpie::file_stream<node> &in_nodes_1,
             tpie::file_stream<node> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<node> &out_nodes)
  {
    debug::println_algorithm_start("APPLY");

    assert::is_valid_input_stream(in_nodes_1);
    debug::println_file_stream(in_nodes_1, "in_nodes 1");

    assert::is_valid_input_stream(in_nodes_2);
    debug::println_file_stream(in_nodes_2, "in_nodes 2");

    assert::is_valid_output_stream(out_nodes);

    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    auto root_1 = in_nodes_1.read_back();
    auto root_2 = in_nodes_2.read_back();

    if (is_sink_node(root_1) && is_sink_node(root_2)) {
      node res_sink_node = node{
          op(root_1.node_ptr, root_2.node_ptr),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);
      debug::println_file_stream(out_nodes, "out_nodes");
    } else if (is_sink_node(root_1) && can_left_shortcut(op, root_1.node_ptr)) {
      node res_sink_node = node{
          op(root_1.node_ptr, create_sink(false)),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);
      debug::println_file_stream(out_nodes, "out_nodes");
    } else if (is_sink_node(root_2) && can_right_shortcut(op, root_2.node_ptr)) {
      node res_sink_node = node{
          op(create_sink(false), root_2.node_ptr),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);
      debug::println_file_stream(out_nodes, "out_nodes");
    } else {
      tpie::file_stream<arc> reduce_node_arcs;
      reduce_node_arcs.open();

      tpie::file_stream<arc> reduce_sink_arcs;
      reduce_sink_arcs.open();

      apply(in_nodes_1, in_nodes_2, op, reduce_node_arcs, reduce_sink_arcs);
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    }
    debug::println_algorithm_end("APPLY");
  }
} // namespace coom

#endif // COOM_APPLY_CPP
