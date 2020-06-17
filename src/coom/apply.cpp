#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "apply.h"

namespace coom
{
  void apply(tpie::file_stream<node>* nodes_1_ptr,
             tpie::file_stream<node>* nodes_2_ptr,
             bool_op op,
             tpie::file_stream<arc>* reduce_node_arcs_ptr,
             tpie::file_stream<arc>* reduce_sink_arcs_ptr)
  {
    auto nodes_1 = *nodes_1_ptr;
    auto nodes_2 = *nodes_2_ptr;
    auto reduce_node_arcs = *reduce_node_arcs_ptr;
    auto reduce_sink_arcs = *reduce_sink_arcs_ptr;

    return;
  }
}

#endif // COOM_APPLY_CPP
