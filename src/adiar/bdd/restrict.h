/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

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
