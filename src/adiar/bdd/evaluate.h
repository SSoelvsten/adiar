#ifndef ADIAR_BDD_EVALUATE_H
#define ADIAR_BDD_EVALUATE_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate an BDD according to an assignment
  ///
  /// \param nodes The node-based BDD graph in reverse topological order.
  /// \return Sink-value after traversal according to the assignment.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &bdd, const assignment_file &assignment);
}

#endif // ADIAR_BDD_EVALUATE_H
