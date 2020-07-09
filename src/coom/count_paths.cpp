#ifndef COOM_COUNT_PATHS_CPP
#define COOM_COUNT_PATHS_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "count_paths.h"

namespace coom
{
  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred)
  {
    return 0;
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes)
  {
    return count_paths(nodes, is_any);
  }
}

#endif // COOM_COUNT_PATHS_CPP
