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
  class bdd_policy
  {
  public:
    typedef bdd reduced_t;
    typedef __bdd unreduced_t;

  public:
    static inline ptr_t reduction_rule(const node_t &n)
    {
      // If adding attributed edges, i.e. complement edges:
      //    remove the 'unflag' below. Currently, it removes any forwarding of
      //    applying Reduction Rule.
      if (unflag(n.low) == unflag(n.high)) { return n.low; }
      return n.uid;
    }

    static inline tuple reduction_rule_inv(const ptr_t &child)
    {
      return { child, child };
    }

  public:
    static inline void compute_cofactor(bool /* on_curr_level */,
                                        ptr_t & /* low */,
                                        ptr_t & /* high */)
    { /* do nothing */ }
  };
}

#endif // ADIAR_BDD_BDD_POLICY_H
