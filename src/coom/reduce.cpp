#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "reduce.h"

namespace coom
{
  void reduce(tpie::file_stream<arc> &in_node_arcs,
              tpie::file_stream<arc> &in_sink_arcs,
              tpie::file_stream<node> &out_nodes)
  {
    return;
  }
}

#endif // COOM_REDUCE_CPP
