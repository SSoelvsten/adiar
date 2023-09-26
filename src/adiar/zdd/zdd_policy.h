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
  using zdd_policy = internal::dd_policy<zdd, __zdd>;

  template<>
  inline zdd::pointer_type
  zdd_policy::reduction_rule(const node &n)
  {
    if (n.high().is_false()) { return n.low(); }
    return n.uid();
  }

  template<>
  inline zdd::node_type::children_type
  zdd_policy::reduction_rule_inv(const zdd::pointer_type &child)
  {
    return { child, zdd::pointer_type(false) };
  }

  // TODO: stop using these in favour of 'reduction_rule_inv' above
  template<>
  inline void
  zdd_policy::compute_cofactor(bool on_curr_level,
                               /*const*/ zdd::pointer_type &,
                               zdd::pointer_type &high)
  {
    if (!on_curr_level) { high = zdd::pointer_type(false); }
  }

  template<>
  inline zdd::node_type::children_type
  zdd_policy::compute_cofactor(const bool on_curr_level,
                               const zdd::node_type::children_type &children)
  {
    if (!on_curr_level) {
      return zdd::node_type::children_type(children[false], zdd::pointer_type(false));
    }
    return children;
  }
}

#endif // ADIAR_ZDD_ZDD_POLICY_H
