#ifndef ADIAR_BDD_BDD_POLICY_H
#define ADIAR_BDD_BDD_POLICY_H

#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logic related to being a 'Binary' Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////
  typedef internal::dd_policy<bdd, __bdd> bdd_policy;

  template<>
  inline bdd::ptr_t
  bdd_policy::reduction_rule(const bdd::node_t &n)
  {
    // If adding attributed edges, i.e. complement edges:
    //    remove the 'unflag' below. Currently, it removes any forwarding of
    //    applying Reduction Rule.
    if (unflag(n.low()) == unflag(n.high())) { return n.low(); }
    return n.uid();
  }

  template<>
  inline bdd::node_t::children_t
  bdd_policy::reduction_rule_inv(const bdd::ptr_t &child)
  {
    return { child, child };
  }

  template<>
  inline void
  bdd_policy::compute_cofactor(bool,
                               /*const*/ bdd::ptr_t &,
                               /*const*/ bdd::ptr_t &)
  { /* do nothing */ }
}

#endif // ADIAR_BDD_BDD_POLICY_H
