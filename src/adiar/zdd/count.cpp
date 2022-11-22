#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/assert.h>

#include <adiar/internal/algorithms/count.h>

namespace adiar
{
  size_t zdd_nodecount(const zdd &zdd)
  {
    return nodecount(zdd.file);
  }

  zdd::label_t zdd_varcount(const zdd &zdd)
  {
    return varcount(zdd.file);
  }

  uint64_t zdd_size(const zdd &zdd)
  {
    return is_terminal(zdd)
      ? value_of(zdd)
      : internal::count<internal::path_count_policy<zdd_policy>>(zdd, zdd_varcount(zdd));
  }
}
