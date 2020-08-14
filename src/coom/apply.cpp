#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include <tpie/priority_queue.h>
#include <tpie/file_stream.h>

#include "data.h"
#include "data_pty.h"

#include "reduce.h"

#include "apply.h"


namespace coom
{
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

  struct lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::min(a.t1, a.t2) < std::min(b.t1, b.t2) ||
            (std::min(a.t1, a.t2) == std::min(b.t1, b.t2) && std::max(a.t1, a.t2) < std::max(b.t1, b.t2));
    }
  };

  struct lt_data
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
    //Set-up
    auto sink_T = create_sink(true);
    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);
    tpie::priority_queue<tuple, lt> appD;
    tpie::priority_queue<tuple_data, lt_data> appD_data;
    node v1 = in_nodes_1.read_back();
    node v2 = in_nodes_2.read_back();
    uint64_t id0 = 0;
    uint64_t current_label = 0;

    //Process root and create initial recursion requests (apply_root)
    uint64_t label_v1 = label_of(v1);
    uint64_t label_v2 = label_of(v2);
    uint64_t low1;
    uint64_t low2;
    uint64_t high1;
    uint64_t high2;
    if (label_v1 < label_v2)
    {
      low1 = v1.low;
      high1 = v1.high;
      low2 = v2.node_ptr;
      high2 = v2.node_ptr;
      if (in_nodes_1.can_read_back())
      {
        v1 = in_nodes_1.read_back();
      }
    }
    else if (label_v1 > label_v2)
    {
      low1 = v1.node_ptr;
      high1 = v1.node_ptr;
      low2 = v2.low;
      high2 = v2.high;
      if (in_nodes_2.can_read_back())
      {
        v2 = in_nodes_2.read_back();
      }
    }
    else
    {
      low1 = v1.low;
      high1 = v1.high;
      low2 = v2.low;
      high2 = v2.high;
      if (in_nodes_1.can_read_back())
      {
        v1 = in_nodes_1.read_back();
      }
      if (in_nodes_2.can_read_back())
      {
        v2 = in_nodes_2.read_back();
      }
    }


    uint64_t root = create_node_ptr(std::min(label_v1, label_v2), 0);
    current_label = std::min(label_v1, label_v2);
    //Shortcut the root
    if (is_sink(low1) && is_sink(low2))
    {
      arc new_arc = create_arc(root, false, op(low1, low2));
      reduce_sink_arcs.write(new_arc);
    }
    else if (is_sink(low1) && can_left_shortcut(op, low1))
    {
      arc new_arc = create_arc(root, false, op(low1, sink_T));
      reduce_sink_arcs.write(new_arc);
    }
    else if (is_sink(low2) && can_right_shortcut(op, low2))
    {
      arc new_arc = create_arc(root, false, op(sink_T, low2));
      reduce_sink_arcs.write(new_arc);
    }
    else
    {
      appD.push({root, low1, low2, false});
    }
    if (is_sink(high1) && is_sink(high2))
    {
      arc new_arc = create_arc(root, true, op(high1, high2));
      reduce_sink_arcs.write(new_arc);
    }
    else if (is_sink(high1) && can_left_shortcut(op, high1))
    {
      arc new_arc = create_arc(root, true, op(high1, sink_T));
      reduce_sink_arcs.write(new_arc);
    }
    else if (is_sink(high2) && can_right_shortcut(op, high2))
    {
      arc new_arc = create_arc(root, true, op(sink_T, high2));
      reduce_sink_arcs.write(new_arc);
    }
    else
    {
      appD.push({root, high1, high2, true});
    }

    label_v1 = label_of(v1);
    label_v2 = label_of(v2);

    //Process all nodes in topological order of both OBDDs
    uint64_t source;
    uint64_t t1;
    uint64_t t2;
    bool is_high;
    node data;
    bool from_1;
    bool with_data;

    while (!appD.empty() || !appD_data.empty())
    {
      //Check if we should read from appD or appD_data
      if(appD_data.empty() ||
         std::min(appD.top().t1, appD.top().t2) < std::max(appD_data.top().t1, appD_data.top().t2))
      {
        tuple current_tuple = appD.top();
        appD.pop();
        with_data = false;
        source = current_tuple.source;
        t1 = current_tuple.t1;
        t2 = current_tuple.t2;
        is_high = current_tuple.is_high;
      }
      else
      {
        tuple_data current_tuple = appD_data.top();
        appD_data.pop();
        with_data = true;
        source = current_tuple.source;
        t1 = current_tuple.t1;
        t2 = current_tuple.t2;
        is_high = current_tuple.is_high;
        data = current_tuple.data;
        from_1 = current_tuple.from_1;

        if(from_1 && data.node_ptr == v1.node_ptr) {
          v2 = in_nodes_2.read_back();
          label_v2 = label_of(v2);
        }
        else if(!from_1 && data.node_ptr == v2.node_ptr) {
          v1 = in_nodes_1.read_back();
        }
      }

      //Forward if none match request (apply_step)
      while (v1.node_ptr != t1 && v2.node_ptr != t2)
      {
        node new_v1 = v1;
        if (v1.node_ptr <= v2.node_ptr)
        {
          new_v1 = in_nodes_1.read_back(); //Ikke overskrive v1!
        }
        if (v1.node_ptr >= v2.node_ptr)
        {
          v2 = in_nodes_2.read_back();
          label_v2 = label_of(v2);
        }
        v1 = new_v1;
      }

      // Forward information across the layer
      if (label_of(t1) == label_of(t2) && (v1.node_ptr != t1 || v2.node_ptr != t2) && !with_data)
      {
        node v0;
        bool from_1;
        if(v1.node_ptr == t1) {
          v0 = v1;
          from_1 = true;
        }
        else {
          v0 = v2;
          from_1 = false;
        }
        appD_data.push({source, t1, t2, is_high, v0, from_1});
        while (!appD.empty() && (appD.top().t1 == t1 && appD.top().t2 == t2))
        {
          tuple current_tuple = appD.top();
          source = current_tuple.source;
          t1 = current_tuple.t1;
          t2 = current_tuple.t2;
          is_high = current_tuple.is_high;
          appD.pop();
          appD_data.push({source, t1, t2, is_high, v0, from_1});
        }
        continue;
      }

      // Resolve current node and recurse
      uint64_t label_t1 = label_of(t1);
      uint64_t label_t2 = label_of(t2);
      uint64_t new_label = std::min(label_t1, label_t2);
      uint64_t low1;
      uint64_t low2;
      uint64_t high1;
      uint64_t high2;

      if (label_t1 != label_t2)
      {
        if (new_label == label_t1)
        {
          low1 = v1.low;
          high1 = v1.high;
          low2 = t2;
          high2 = t2;
        }
        else
        {
          low1 = t1;
          high1 = t1;
          low2 = v2.low;
          high2 = v2.high;
        }
      }
      else
      {
        node v1a = v1;
        node v2a = v2;
        if (with_data)
        {
          if(from_1) {
            v1a = data;
          }
          else {
            v2a = data;
          }
        }
        low1 = v1a.low;
        high1 = v1a.high;
        low2 = v2a.low;
        high2 = v2a.high;
      }

      // Check shortcuts
      if (current_label != new_label)
      {
        id0 = 0;
        current_label = new_label;
      }
      uint64_t new_node_ptr = create_node_ptr(new_label, id0);
      id0 = id0 + 1;

      if (is_sink(low1) && is_sink(low2))
      {
        arc new_arc = create_arc(new_node_ptr, false, op(low1, low2));
        reduce_sink_arcs.write(new_arc);
      }
      else if (is_sink(low1) && can_left_shortcut(op, low1))
      {
        arc new_arc = create_arc(new_node_ptr, false, op(low1, sink_T));
        reduce_sink_arcs.write(new_arc);
      }
      else if (is_sink(low2) && can_right_shortcut(op, low2))
      {
        arc new_arc = create_arc(new_node_ptr, false, op(sink_T, low2));
        reduce_sink_arcs.write(new_arc);
      }
      else
      {
        appD.push({new_node_ptr, low1, low2, false});
      }
      if (is_sink(high1) && is_sink(high2))
      {
        arc new_arc = create_arc(new_node_ptr, true, op(high1, high2));
        reduce_sink_arcs.write(new_arc);
      }
      else if (is_sink(high1) && can_left_shortcut(op, high1))
      {
        arc new_arc = create_arc(new_node_ptr, true, op(high1, sink_T));
        reduce_sink_arcs.write(new_arc);
      }
      else if (is_sink(high2) && can_right_shortcut(op, high2))
      {
        arc new_arc = create_arc(new_node_ptr, true, op(sink_T, high2));
        reduce_sink_arcs.write(new_arc);
      }
      else
      {
        appD.push({new_node_ptr, high1, high2, true});
      }

      while (true)
      {
        arc new_arc = create_arc(source, is_high, new_node_ptr);
        reduce_node_arcs.write(new_arc);

        if (!appD.empty() && (appD.top().t1 == t1 && appD.top().t2 == t2))
        {
          tuple current_tuple = appD.top();
          appD.pop();
          source = current_tuple.source;
          t1 = current_tuple.t1;
          t2 = current_tuple.t2;
          is_high = current_tuple.is_high;
        }
        else if (!appD_data.empty() && (appD_data.top().t1 == t1 && appD_data.top().t2 == t2))
        {
          tuple_data current_tuple = appD_data.top();
          appD_data.pop();
          source = current_tuple.source;
          t1 = current_tuple.t1;
          t2 = current_tuple.t2;
          is_high = current_tuple.is_high;
          data = current_tuple.data;
        }
        else
        {
          break;
        }
      }
    }
  }

  void apply(tpie::file_stream<node> &in_nodes_1,
             tpie::file_stream<node> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<node> &out_nodes)
  {
#if COOM_ASSERT
    assert(in_nodes_1.size() > 0);
    assert(in_nodes_2.size() > 0);
    assert(out_nodes.size() == 0);
#endif
#if COOM_DEBUG
    tpie::log_info() << "//===\\\\ APPLY //===\\\\" << std::endl;
    tpie::log_info() << "in_nodes_1: ";
    println_file_stream(in_nodes_1);
    tpie::log_info() << "in_nodes_2: ";
    println_file_stream(in_nodes_2);
#endif

    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    auto root_1 = in_nodes_1.read_back();
    auto root_2 = in_nodes_2.read_back();

    if (is_sink_node(root_1) && is_sink_node(root_2))
    {
      node res_sink_node = node{
          op(root_1.node_ptr, root_2.node_ptr),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    }
    else if (is_sink_node(root_1) && can_left_shortcut(op, root_1.node_ptr))
    {
      node res_sink_node = node{
          op(root_1.node_ptr, create_sink(false)),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    }
    else if (is_sink_node(root_2) && can_right_shortcut(op, root_2.node_ptr))
    {
      node res_sink_node = node{
          op(create_sink(false), root_2.node_ptr),
          NIL,
          NIL};

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    }
    else
    {
      tpie::file_stream<arc> reduce_node_arcs;
      reduce_node_arcs.open();

      tpie::file_stream<arc> reduce_sink_arcs;
      reduce_sink_arcs.open();

      apply(in_nodes_1, in_nodes_2, op, reduce_node_arcs, reduce_sink_arcs);
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    }
#if COOM_DEBUG
    tpie::log_info() << "\\\\===// APPLY \\\\===//" << std::endl;
#endif
  }
} // namespace coom

#endif // COOM_APPLY_CPP
