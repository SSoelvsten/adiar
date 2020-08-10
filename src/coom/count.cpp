#ifndef COOM_COUNT_CPP
#define COOM_COUNT_CPP

#include <tpie/file_stream.h>

#include "data.h"
#include "util.cpp"

#include "count.h"

namespace coom
{
  inline uint64_t count(tpie::file_stream<node> &nodes,
                        const sink_pred &sink_pred,
                        const bool count_skipped_layers)
  {
    tpie::file_stream<arc> node_arcs;
    node_arcs.open();

    tpie::file_stream<arc> sink_arcs;
    sink_arcs.open();

    transpose_obdd(nodes, node_arcs, sink_arcs);

    // TODO: Do something on the transposed graph...

    return 0;
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred)
  {
    return count(nodes, sink_pred, false);
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes)
  {
    return count_paths(nodes, is_any);
  }

  uint64_t count_assignments(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred)
  {
    return count(nodes, sink_pred, true);
  }
}

#endif // COOM_COUNT_CPP
