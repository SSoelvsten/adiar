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

#ifndef ADIAR_ZDD_PRED_H
#define ADIAR_ZDD_PRED_H

#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determines whether the two given ZDDs represent the same family.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_equal(const zdd &s1, const zdd &s2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determines whether the two given ZDDs represent different families.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_unequal(const zdd &s1, const zdd &s2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determines whether one family is a subset or equal to the other.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_subseteq(const zdd &s1, const zdd &s2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determines whether one family is a subset of the other.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_subset(const zdd &s1, const zdd &s2);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determines whether the two families are disjoint.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_disjoint(const zdd &s1, const zdd &s2);
}

#endif // ADIAR_ZDD_PRED_H
