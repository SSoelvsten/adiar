#include "build.h"

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
  class zdd_vars_chain_policy
  {
  public:
    static constexpr bool on_empty          = true;
    static constexpr bool link[2]           = {false, true};
    static constexpr bool terminal_value[2] = {false, true};
  };

  zdd zdd_vars(const shared_file<zdd::label_t> &labels)
  {
    zdd_vars_chain_policy p;
    return internal::build_chain<zdd_vars_chain_policy>(p, labels);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singleton(zdd::label_t label)
  {
    return internal::build_ithvar(label);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_singletons_chain_policy
  {
  public:
    static constexpr bool on_empty          = false;
    static constexpr bool link[2]           = {true, false};
    static constexpr bool terminal_value[2] = {false, true};
  };

  zdd zdd_singletons(const shared_file<zdd::label_t> &labels)
  {
    zdd_singletons_chain_policy p;
    return internal::build_chain<zdd_singletons_chain_policy>(p, labels);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_powerset_chain_policy
  {
  public:
    static constexpr bool on_empty          = true;
    static constexpr bool link[2]           = {true, true};
    static constexpr bool terminal_value[2] = {true, true};
  };

  zdd zdd_powerset(const shared_file<zdd::label_t> &labels)
  {
    zdd_powerset_chain_policy p;
    return internal::build_chain<zdd_powerset_chain_policy>(p, labels);
  }
}
