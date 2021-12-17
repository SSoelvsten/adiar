////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
