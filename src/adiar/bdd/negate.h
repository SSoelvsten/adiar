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

#ifndef ADIAR_BDD_NEGATE_H
#define ADIAR_BDD_NEGATE_H

#include <adiar/data.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate a given node-based BDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_not(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate a given node-based BDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_not(bdd &&bdd);
}

#endif // ADIAR_BDD_NEGATE_H
