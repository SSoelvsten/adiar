#ifndef ADIAR_BDD_RESTRICT_H
#define ADIAR_BDD_RESTRICT_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict an BDD based on a partial assignment.
  ///
  /// \param nodes_in   Input file of nodes
  ///
  /// \param assignment Assignment sorted in order of label (and expected
  ///                   unique)
  ///
  /// \return           A node_file if the BDD is restricted to a sink, and
  ///                   otherwise an arc_file of the restricted BDD.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &bdd, const assignment_file &assignment);
}

#endif // ADIAR_BDD_RESTRICT_H
