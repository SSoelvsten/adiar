#ifndef COOM_REDUCE_H
#define COOM_REDUCE_H

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based OBDD.
  ///
  /// \param arc_file The unreduced obdd in its arc-based representation
  ///
  /// \return The reduced obdd in a node-based representation
  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const arc_file &arc_file);
}

#endif // COOM_REDUCE_H
