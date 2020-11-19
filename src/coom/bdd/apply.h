#ifndef COOM_APPLY_H
#define COOM_APPLY_H

#include <coom/data.h>
#include <coom/bdd/bdd.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two OBDDs creates one as per an operator.
  ///
  /// Creates the product construction of the two given OBDDs.
  ///
  /// \param bdd_i     BDD to apply with the other.
  ///
  /// \param op        Binary boolean operator to be applied.
  ///
  /// \return A node_file if the operator shortcuts the result to a sink, and
  ///         otherwise an arc_file of the OBDD representing the operator
  ///         applied on both inputs.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_nand(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_or(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_nor(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_xor(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_imp(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_invimp(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_equiv(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_diff(const bdd &bdd_1, const bdd &bdd_2);
  __bdd bdd_less(const bdd &bdd_1, const bdd &bdd_2);
}

#endif // COOM_APPLY_H
