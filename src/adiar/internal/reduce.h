#ifndef ADIAR_REDUCE_H
#define ADIAR_REDUCE_H

#include <adiar/file.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based BDD.
  ///
  /// \param arc_file The unreduced bdd in its arc-based representation
  ///
  /// \return The reduced bdd in a node-based representation
  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const arc_file &arc_file);
}

#endif // ADIAR_REDUCE_H
