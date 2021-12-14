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

#ifndef ADIAR_BDD_QUANTIFY_H
#define ADIAR_BDD_QUANTIFY_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &bdd, const label_t &label);

  __bdd bdd_exists(const bdd &bdd, const label_file &labels);
  __bdd bdd_exists(bdd &&bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &bdd, const label_t &label);

  __bdd bdd_forall(const bdd &bdd, const label_file &labels);
  __bdd bdd_forall(bdd &&bdd, const label_file &labels);
}

#endif // ADIAR_BDD_QUANTIFY_H
