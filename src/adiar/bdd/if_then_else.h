#ifndef ADIAR_IF_THEN_ELSE_H
#define ADIAR_IF_THEN_ELSE_H

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

#endif // ADIAR_IF_THEN_ELSE_H
