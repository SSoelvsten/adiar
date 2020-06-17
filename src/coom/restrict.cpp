#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "restrict.h"

namespace coom
{
  void restrict(tpie::file_stream<node>* nodes_ptr,
                tpie::file_stream<assignment>* assignment_ptr,
                tpie::file_stream<arc>* reduce_node_arcs_ptr,
                tpie::file_stream<arc>* reduce_sink_arcs_ptr)
  {
    auto nodes = *nodes_ptr;
    auto assignment = *assignment_ptr;
    auto reduce_node_arcs = *reduce_node_arcs_ptr;
    auto reduce_sink_arcs = *reduce_sink_arcs_ptr;

    return;
  }
}

#endif // COOM_RESTRICT_CPP
