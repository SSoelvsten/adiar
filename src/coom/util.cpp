#ifndef COOM_UTIL_CPP
#define COOM_UTIL_CPP

#include <tpie/file_stream.h>

#include "data.h"

namespace coom
{
  const auto transpose_node_arc_lt = [] (const arc& a, const arc& b) -> bool {
    return false;
  };

  const auto transpose_sink_arc_lt = [] (const arc& a, const arc& b) -> bool {
    return false;
  };

  inline void transpose_obdd(tpie::file_stream<node> &in_nodes,
                             tpie::file_stream<arc> &out_node_arcs,
                             tpie::file_stream<arc> &out_sink_arcs)
  {
    /* Minimal example of sorting a file_stream

      tpie::progress_indicator_null pi;
      tpie::sort(xs, [&](const Item & a, const Item & b) -> bool { return a.a < b.a; }, pi);
     */

    // TODO: 'Transpose' the graph ...

    return;
  }

  inline void copy(tpie::file_stream<node> &in_nodes,
                   tpie::file_stream<node> &out_nodes)
  {
    in_nodes.seek(0);
    while(in_nodes.can_read()) {
      out_nodes.write(in_nodes.read());
    }
  }
}

#endif // COOM_UTIL_CPP
