#ifndef COOM_EVALUATE_H
#define COOM_EVALUATE_H

#include <coom/data.h>
#include <coom/file.h>

#include <coom/bdd/bdd.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate an OBDD according to an assignment
  ///
  /// \param nodes The node-based OBDD graph in reverse topological order.
  /// \return Sink-value after traversal according to the assignment.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &bdd, const assignment_file &assignment);
}

#endif // COOM_EVALUATE_H
