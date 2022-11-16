#ifndef ADIAR_ZDD_ZDD_POLICY_H
#define ADIAR_ZDD_ZDD_POLICY_H

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/uid.h>

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
    static inline ptr_uint64 reduction_rule(const node &n)
    {
      if (n.high().is_false()) { return n.low(); }
      return n.uid();
    }

    static inline tuple reduction_rule_inv(const ptr_uint64 &child)
    {
      return { child, ptr_uint64(false) };
    }

  public:
    static inline void compute_cofactor(bool on_curr_level, ptr_uint64 &, ptr_uint64 &high)
    {
      if (!on_curr_level) { high = ptr_uint64(false); }
    }
  };
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
