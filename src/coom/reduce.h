#ifndef COOM_REDUCE_H
#define COOM_REDUCE_H

#include <tpie/file_stream.h>
#include "data.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based OBDD.
  ///
  /// \param in_node_arcs Edges (source, target) sorted by target in topological
  ///                     order. Flag on `source` is set, if it is the high arc.
  /// \param in_sink_arcs Edges (source,sink) sorted by source in topological
  ///                     order. Flag on `source` is set, if it is the high arc.
  ///
  /// \param out_nodes The reduced node-based OBDD in reverse topological order
  //////////////////////////////////////////////////////////////////////////////
  void reduce(tpie::file_stream<arc_t> &in_node_arcs,
              tpie::file_stream<arc_t> &in_sink_arcs,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta);
}

#endif // COOM_REDUCE_H
