#ifndef ADIAR_BDD_EVALUATE_H
#define ADIAR_BDD_EVALUATE_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  typedef std::function<bool(label_t)> assignment_func;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate a BDD according to an assignment
  ///
  /// The function `af` may assume/abuse that it is only called with the labels
  /// in a strictly increasing order.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &bdd, const assignment_func &af);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate a BDD according to an assignment
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &bdd, const assignment_file &as);
}

#endif // ADIAR_BDD_EVALUATE_H
