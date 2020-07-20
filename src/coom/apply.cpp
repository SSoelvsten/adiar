#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include <tpie/file_stream.h>
#include "data.h"
#include "data_pty.h"

#include "apply.h"

namespace coom
{
  bool can_right_shortcut (const bool_op &op, const uint64_t sink)
  {
    return op(create_sink(false), sink) == op(create_sink(true), sink);
  }

  bool can_left_shortcut (const bool_op &op, const uint64_t sink)
  {
    return op(sink, create_sink(false)) == op(sink, create_sink(true));
  }

  void apply(tpie::file_stream<node> &in_nodes_1,
             tpie::file_stream<node> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<arc> &reduce_node_arcs,
             tpie::file_stream<arc> &reduce_sink_arcs)
  {
    in_nodes_1.seek(0, tpie::file_stream_base::end);
    in_nodes_2.seek(0, tpie::file_stream_base::end);

    // TODO: Implement...
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

    if (is_sink_node(root_1) && is_sink_node(root_2)) {
      node res_sink_node = node {
        op(root_1.node_ptr, root_2.node_ptr),
        NIL,
        NIL
      };

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    } else if (is_sink_node(root_1) && can_left_shortcut(op, root_1.node_ptr)) {
      node res_sink_node = node {
        op(root_1.node_ptr, create_sink(false)),
        NIL,
        NIL
      };

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    } else if (is_sink_node(root_2) && can_right_shortcut(op, root_2.node_ptr)) {
      node res_sink_node = node {
        op(create_sink(false), root_2.node_ptr),
        NIL,
        NIL
      };

      out_nodes.write(res_sink_node);

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      println_file_stream(out_nodes);
#endif
    } else {
      tpie::file_stream<arc> reduce_node_arcs;
      reduce_node_arcs.open();

      tpie::file_stream<arc> reduce_sink_arcs;
      reduce_sink_arcs.open();

      apply(in_nodes_1, in_nodes_2, op, reduce_node_arcs, reduce_sink_arcs);
    }
#if COOM_DEBUG
    tpie::log_info() << "\\\\===// APPLY \\\\===//" << std::endl;
#endif
  }
}

#endif // COOM_APPLY_CPP
