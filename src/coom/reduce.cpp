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


#if COOM_DEBUG
    tpie::log_info() << "out_nodes: ";
    coom::println_file_stream(out_nodes);
    tpie::log_info() << "\\\\===// REDUCE \\\\===//\n";
#endif
  }
}

#endif // COOM_REDUCE_CPP
