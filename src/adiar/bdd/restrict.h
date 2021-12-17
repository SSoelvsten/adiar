////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
