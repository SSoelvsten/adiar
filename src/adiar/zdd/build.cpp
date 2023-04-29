#include "build.h"

#include <adiar/zdd/zdd_policy.h>

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
  zdd zdd_powerset(const shared_file<zdd::label_t> &labels)
  {
    internal::chain_both<zdd_policy> p;
    return internal::build_chain<>(p, labels);
  }
}
