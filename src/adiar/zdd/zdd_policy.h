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
  typedef decision_diagram_policy<zdd, __zdd> zdd_policy;

  template<>
  inline ptr_uint64
  zdd_policy::reduction_rule(const node &n)
  {
    if (n.high().is_false()) { return n.low(); }
    return n.uid();
  }

  template<>
  inline tuple<ptr_uint64>
  zdd_policy::reduction_rule_inv(const ptr_uint64 &child)
  {
    return { child, ptr_uint64(false) };
  }

  template<>
  inline void
  zdd_policy::compute_cofactor(bool on_curr_level, ptr_uint64 &, ptr_uint64 &high)
  {
    if (!on_curr_level) { high = ptr_uint64(false); }
  }
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
