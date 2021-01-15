#ifndef ADIAR_REDUCE_H
#define ADIAR_REDUCE_H

#include <adiar/data.h>
#include <adiar/file.h>

namespace adiar
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

#endif // ADIAR_REDUCE_H
