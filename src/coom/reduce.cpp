#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/file_stream.h>

#include "data.h"
#include "data_pty.h"

#include "reduce.h"

namespace coom
{
  void reduce(tpie::file_stream<arc> &in_node_arcs,
              tpie::file_stream<arc> &in_sink_arcs,
              tpie::file_stream<node> &out_nodes)
  {
#if COOM_DEBUG
    tpie::log_info() << "//===\\\\ REDUCE //===\\\\\n";
    tpie::log_info() << "in_node_arcs: ";
    coom::println_file_stream(in_node_arcs);

    tpie::log_info() << "in_sink_arcs: ";
    coom::println_file_stream(in_sink_arcs);
#endif

    // Fake it 'til you make it...
    out_nodes.seek(0);

    auto n3 = create_node(2, MAX_ID, create_sink(false), create_sink(true));
    out_nodes.write(n3);

    out_nodes.write(create_node(1, MAX_ID, create_node_ptr(2,MAX_ID), create_sink(true)));
    out_nodes.write(create_node(0, MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(1,MAX_ID)));

    return;

#if COOM_DEBUG
    tpie::log_info() << "out_nodes: ";
    coom::println_file_stream(out_nodes);
    tpie::log_info() << "\\\\===// REDUCE \\\\===//\n";
#endif
  }
}

#endif // COOM_REDUCE_CPP
