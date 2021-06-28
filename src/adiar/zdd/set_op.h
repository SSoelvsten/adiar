#ifndef ADIAR_ZDD_SET_OP_H
#define ADIAR_ZDD_SET_OP_H

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
  __zdd zdd_set_op(const zdd &zdd_1, const zdd &zdd_2, const bool_op &op);

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

#endif // ADIAR_ZDD_SET_OP_H
