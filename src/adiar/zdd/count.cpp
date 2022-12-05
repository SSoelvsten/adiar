#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/count.h>

namespace adiar
{
  size_t zdd_nodecount(const zdd &A)
  {
    return is_terminal(A) ? 0u : A->size();
  }

  zdd::label_t zdd_varcount(const zdd &A)
  {
    return A->levels();
  }

  uint64_t zdd_size(const zdd &A)
  {
    return is_terminal(A)
      ? value_of(A)
      : internal::count<internal::path_count_policy<zdd_policy>>(A, zdd_varcount(A));
  }
}
