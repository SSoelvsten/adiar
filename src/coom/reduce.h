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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a possibly yet to be reduced file.
  ///
  /// If the given maybe-reduced set of files is unreduced, then it will be
  /// reduced and the given file_union will also be pointing to the content of
  /// the output node_file returned.
  ///
  /// \return The reduced obdd in a node-based representation.
  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const node_or_arc_file &maybe_reduced_file);
}

#endif // COOM_REDUCE_H
