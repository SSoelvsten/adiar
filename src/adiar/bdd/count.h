/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADIAR_BDD_COUNT_H
#define ADIAR_BDD_COUNT_H

#include <adiar/data.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of internal nodes used to represent the given BDD.
  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of levels, i.e. variables, occuring in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  label_t bdd_varcount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to a T sink.
  ///
  /// \param bdd       The node-based BDD to count in.
  ///
  /// \return The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all assignments to variables that lead to a T sink. We don't
  /// need to know the exact variables in question, just the number of variables
  /// involved.
  ///
  /// \param bdd       The node-based BDD to count in.
  /// \param varcount  The total number of variables, that may occur in the bdd.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd, label_t varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Same as `bdd_satcount(bdd, varcount)`, with varcount set to be
  /// `varcount(bdd)`.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd);
}

#endif // ADIAR_BDD_COUNT_H
