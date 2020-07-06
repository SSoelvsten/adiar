#ifndef COOM_COUNT_PATHS_CPP
#define COOM_COUNT_PATHS_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "count_paths.h"

namespace coom
{
  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const node_pred &node_pred,
                       const sink_pred &sink_pred)
  {
    return 0;
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const node_pred &node_pred)
  {
    return count_paths(nodes, node_pred, is_any_sink);
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred)
  {
    return count_paths(nodes, is_any_node, sink_pred);
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes)
  {
    return count_paths(nodes, is_any_node, is_any_sink);
  }
}

#endif // COOM_COUNT_PATHS_CPP
