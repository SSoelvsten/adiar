#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/count.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/dd_func.h>

namespace adiar
{
  size_t
  zdd_nodecount(const zdd& A)
  {
    return internal::dd_nodecount(A);
  }

  zdd::label_type
  zdd_varcount(const zdd& A)
  {
    return internal::dd_varcount(A);
  }

  uint64_t
  zdd_size(const exec_policy& ep, const zdd& A)
  {
    return zdd_isterminal(A)
      ? internal::dd_valueof(A)
      : internal::count<internal::path_count_policy<zdd_policy>>(ep, A, zdd_varcount(A));
  }

  uint64_t
  zdd_size(const zdd& A)
  {
    return zdd_size(exec_policy(), A);
  }
}
