#include "build.h"

#include <adiar/zdd/zdd_policy.h>
#include <adiar/domain.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/build.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_terminal(bool value)
  {
    return internal::build_terminal(value);
  }

  zdd zdd_empty()
  {
    return internal::build_terminal(false);
  }

  zdd zdd_null()
  {
    return internal::build_terminal(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_ithvar_policy : public zdd_policy
  {
  private:
    const zdd::label_t var;

  public:
    zdd_ithvar_policy(zdd::label_t v) : var(v)
    { }

  public:
    static constexpr bool init_terminal = true;

    inline zdd_policy::node_t
    make_node(const zdd_policy::label_t &l, const zdd_policy::ptr_t &r) const
    {
      if (l == var) {
        return zdd_policy::node_t(l, zdd_policy::MAX_ID, zdd_policy::ptr_t(false), r);
      }
      return zdd_policy::node_t(l, zdd_policy::MAX_ID, r, r);
    }
  };

  zdd zdd_ithvar(zdd::label_t var, const shared_file<zdd::label_t> &dom)
  {
    if (dom->size() == 0u) { return zdd_empty(); }

    zdd_ithvar_policy p(var);
    return internal::build_chain<>(p, dom);
  }

  zdd zdd_ithvar(zdd::label_t var)
  {
    zdd_ithvar_policy p(var);
    return internal::build_chain<>(p, adiar_get_domain());
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_vars(const shared_file<zdd::label_t> &labels)
  {
    internal::chain_high<zdd_policy> p;
    return internal::build_chain<>(p, labels);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singleton(zdd::label_t label)
  {
    return internal::build_ithvar(label);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singletons(const shared_file<zdd::label_t> &labels)
  {
    internal::chain_low<zdd_policy> p;
    return internal::build_chain<>(p, labels);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_powerset(const shared_file<zdd::label_t> &dom)
  {
    internal::chain_both<zdd_policy> p;
    return internal::build_chain<>(p, dom);
  }
}
