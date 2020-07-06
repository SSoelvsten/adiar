#ifndef COOM_APPLY_CPP
#define COOM_APPLY_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "apply.h"

namespace coom
{
  void apply(tpie::file_stream<node> &nodes_1,
             tpie::file_stream<node> &nodes_2,
             const bool_op &op,
             tpie::file_stream<arc> &reduce_node_arcs,
             tpie::file_stream<arc> &reduce_sink_arcs)
  {
    return;
  }
}

#endif // COOM_APPLY_CPP
