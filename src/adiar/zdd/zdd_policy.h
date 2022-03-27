#ifndef ADIAR_ZDD_ZDD_POLICY_H
#define ADIAR_ZDD_ZDD_POLICY_H

#include <adiar/data.h>
#include <adiar/internal/tuple.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logic related to being a 'Zero-suppressed' Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////
  class zdd_policy
  {
  public:
    typedef zdd reduced_t;
    typedef __zdd unreduced_t;

  public:
    static inline ptr_t reduction_rule(const node_t &n)
    {
      if (is_false(n.high)) { return n.low; }
      return n.uid;
    }

    static inline tuple reduction_rule_inv(const ptr_t &child)
    {
      return { child, create_sink_ptr(false) };
    }

  public:
    static inline void compute_cofactor(bool on_curr_level, ptr_t &, ptr_t &high)
    {
      if (!on_curr_level) { high = create_sink_ptr(false); }
    }
  };
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
