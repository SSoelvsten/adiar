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

#ifndef ADIAR_ZDD_BINOP_H
#define ADIAR_ZDD_BINOP_H

#include <adiar/data.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two ZDDs creates one as per an operator.
  ///
  /// \param zdd_i     ZDD to apply with the other.
  ///
  /// \param op        Binary boolean operator to be applied.
  ///
  /// \return Product construction of the two that represents the boolean
  ///         operator applied to the two family of sets.
  //////////////////////////////////////////////////////////////////////////////
  // TODO: Should this be exposed to the end user?
  __zdd zdd_binop(const zdd &zdd_1, const zdd &zdd_2, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two ZDDs creates a new ZDD that represents the union of their
  /// family of sets.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_union(const zdd &zdd_1, const zdd &zdd_2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two ZDDs creates a new ZDD that represents the intersection
  /// of their family of sets.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_intsec(const zdd &zdd_1, const zdd &zdd_2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two ZDDs creates a new ZDD that represents the difference
  /// between their family of sets.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_diff(const zdd &zdd_1, const zdd &zdd_2);
}

#endif // ADIAR_ZDD_BINOP_H
