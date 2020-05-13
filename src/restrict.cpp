#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "restrict.h"

namespace coom
{
  void restrict(tpie::file_stream<node> nodes,
                tpie::file_stream<assignment> assignment,
                tpie::file_stream<arc> reduce_node_arcs,
                tpie::file_stream<arc> reduce_sink_arcs)
  {
    return;
  }
}

#endif // COOM_RESTRICT_CPP
