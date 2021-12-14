/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADIAR_BDD_ASSIGNMENT_H
#define ADIAR_BDD_ASSIGNMENT_H

#include <adiar/file.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically smallest x such that f(x) = 1.
  ///
  /// \param f A non-sink BDD.
  ///
  /// \return An assignment_file containing the lexicographically smallest
  /// assignment for the variables with a populated level in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmin(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically largest x such that f(x) = 1.
  ///
  /// The returned assignment file will have a value for every populated level
  /// in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmax(const bdd &f);
}

#endif // ADIAR_BDD_ASSIGNMENT_H
