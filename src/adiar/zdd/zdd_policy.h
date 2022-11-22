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
  typedef internal::dd_policy<zdd, __zdd> zdd_policy;

  template<>
  inline zdd::ptr_t
  zdd_policy::reduction_rule(const node &n)
  {
    if (n.high().is_false()) { return n.low(); }
    return n.uid();
  }

  template<>
  inline internal::tuple<zdd::ptr_t>
  zdd_policy::reduction_rule_inv(const zdd::ptr_t &child)
  {
    return { child, zdd::ptr_t(false) };
  }

  template<>
  inline void
  zdd_policy::compute_cofactor(bool on_curr_level, zdd::ptr_t &, zdd::ptr_t &high)
  {
    if (!on_curr_level) { high = zdd::ptr_t(false); }
  }
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
