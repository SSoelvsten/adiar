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

#ifndef ADIAR_ZDD_COUNT_H
#define ADIAR_ZDD_COUNT_H

#include <adiar/data.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of internal nodes used to represent the given ZDD.
  //////////////////////////////////////////////////////////////////////////////
  size_t zdd_nodecount(const zdd &zdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of levels, i.e. variables, occuring in the ZDD.
  //////////////////////////////////////////////////////////////////////////////
  label_t zdd_varcount(const zdd &zdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of sets in the family of sets described by the ZDD.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t zdd_size(const zdd &zdd);
}

#endif // ADIAR_ZDD_COUNT_H
