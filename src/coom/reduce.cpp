#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "reduce.h"

namespace coom
{
  void reduce(tpie::file_stream<arc>* in_node_arcs_ptr,
              tpie::file_stream<arc>* in_sink_arcs_ptr,
              tpie::file_stream<node>* out_nodes_ptr)
  {
    auto in_node_arcs = *in_node_arcs_ptr;
    auto in_sink_arcs = *in_sink_arcs_ptr;
    auto out_nodes = *out_nodes_ptr;
    return;
  }
}

#endif // COOM_REDUCE_CPP
