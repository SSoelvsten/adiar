#ifndef COOM_RESTRICT_H
#define COOM_RESTRICT_H

#include <tpie/file_stream.h>
#include "data.h"

namespace coom
{
  struct assignment {
    uint64_t index;
    bool value;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a given node-based OBDD based on a partial assignment.
  ///
  /// \param node_arcs Edges (source, target) sorted by target in reverse-
  ///                  topological order.
  /// \param sink_arcs Edges (source,sink) sorted by source in reverse-
  ///                  topological order.
  ///
  /// \param reduce_node_arcs The (ptr,ptr) arcs sorted by target in order
  /// \param reduce_sink_arcs The (ptr,sink) arcs sorted by source in order
  //////////////////////////////////////////////////////////////////////////////
  void restrict(tpie::file_stream<node>* nodes,
                tpie::file_stream<assignment>* assignment,
                tpie::file_stream<arc>* reduce_node_arcs,
                tpie::file_stream<arc>* reduce_sink_arcs);
}

#endif // COOM_RESTRICT_H
