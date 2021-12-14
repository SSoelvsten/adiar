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

#ifndef ADIAR_BDD_APPLY_H
#define ADIAR_BDD_APPLY_H

#include <adiar/data.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two BDDs creates one as per an operator.
  ///
  /// \param bdd_i     BDD to apply with the other.
  ///
  /// \param op        Binary boolean operator to be applied.
  ///
  /// \return The product construction of the two that represents the boolean
  ///         operator applied to the output of both functions.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_nand(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_or(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_nor(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_xor(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_xnor(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_imp(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_invimp(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_equiv(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_diff(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_less(const bdd &bdd_1, const bdd &bdd_2);
}

#endif // ADIAR_BDD_APPLY_H
