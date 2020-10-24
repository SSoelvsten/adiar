#ifndef COOM_APPLY_H
#define COOM_APPLY_H

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two OBDDs creates one as per an operator.
  ///
  /// Creates the product construction of the two given OBDDs.
  ///
  /// \param in_file_i File containing an OBDD in question.
  ///
  /// \param op        Binary boolean operator to be applied.
  ///
  /// \return A node_file if the operator shortcuts the result to a sink, and
  ///         otherwise an arc_file of the OBDD representing the operator
  ///         applied on both inputs.
  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_apply(const node_file &file_1,
                             const node_file &file_2,
                             const bool_op &op);
}

#endif // COOM_APPLY_H
