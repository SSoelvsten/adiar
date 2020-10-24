#ifndef COOM_RESTRICT_H
#define COOM_RESTRICT_H

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict an OBDD based on a partial assignment.
  ///
  /// \param nodes_in   Input file of nodes
  ///
  /// \param assignment Assignment sorted in order of label (and expected
  ///                   unique)
  ///
  /// \return           A node_file if the OBDD is restricted to a sink, and
  ///                   otherwise an arc_file of the restricted OBDD.
  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_restrict(const node_file &bdd, const assignment_file &assignment);
}

#endif // COOM_RESTRICT_H
