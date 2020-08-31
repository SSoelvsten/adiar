#ifndef COOM_UTIL_CPP
#define COOM_UTIL_CPP

#include <tpie/file_stream.h>

#include "data.h"

namespace coom
{
  inline void copy(tpie::file_stream<node_t> &in_nodes,
                   tpie::file_stream<node_t> &out_nodes)
  {
    in_nodes.seek(0);
    while(in_nodes.can_read()) {
      out_nodes.write(in_nodes.read());
    }
  }
}

#endif // COOM_UTIL_CPP
