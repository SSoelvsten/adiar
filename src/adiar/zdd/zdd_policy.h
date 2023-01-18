#ifndef ADIAR_ZDD_ZDD_POLICY_H
#define ADIAR_ZDD_ZDD_POLICY_H

#include <adiar/zdd/zdd.h>

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/uid.h>

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
  inline zdd::node_t::children_t
  zdd_policy::reduction_rule_inv(const zdd::ptr_t &child)
  {
    return { child, zdd::ptr_t(false) };
  }

  // TODO: stop using this one in favour of the other one below
  template<>
  inline void
  zdd_policy::compute_cofactor(bool on_curr_level,
                               /*const*/ zdd::ptr_t &,
                               zdd::ptr_t &high)
  {
    if (!on_curr_level) { high = zdd::ptr_t(false); }
  }

  template<>
  inline zdd::node_t::children_t
  zdd_policy::compute_cofactor(bool on_curr_level,
                               const zdd::node_t::children_t &children)
  {
    return on_curr_level
      ? children :
      zdd::node_t::children_t(children[0], zdd::ptr_t(false));
  }
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
