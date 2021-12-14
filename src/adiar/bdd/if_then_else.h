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

#ifndef ADIAR_BDD_IF_THEN_ELSE_H
#define ADIAR_BDD_IF_THEN_ELSE_H

#include <adiar/data.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two BDDs creates one as per an operator.
  ///
  /// Creates the product construction of the three given BDDs to resemble their
  /// if-then-else. That is, if the first BDD evaluates to true, then the output
  /// will evaluate to the value of the second. If not, then it evaluates to the
  /// value of the third.
  ///
  /// This is faster than using bdd_apply to compute
  ///
  ///               (bdd_if => bdd_then) && (~bdd_if => bdd_else)
  ///
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_ite(const bdd &bdd_if, const bdd &bdd_then, const bdd &bdd_else);
}

#endif // ADIAR_BDD_IF_THEN_ELSE_H
