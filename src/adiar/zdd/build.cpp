#include "build.h"

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/build.h>

namespace adiar
{
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

  zdd zdd_ithvar(zdd::label_t label)
  {
    return internal::build_ithvar(label);
  }

  zdd zdd_vars(const shared_file<zdd::label_t> &labels)
  {
    return internal::build_chain<true, false, true>(labels);
  }

  zdd zdd_singletons(const shared_file<zdd::label_t> &labels)
  {
    return internal::build_chain<false, true, false>(labels);
  }

  zdd zdd_powerset(const shared_file<zdd::label_t> &labels)
  {
    return internal::build_chain<true, true, true, true>(labels);
  }
}
