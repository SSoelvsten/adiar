#include <adiar/zdd.h>

#include <adiar/data.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/count.h>

namespace adiar
{
  size_t zdd_nodecount(const zdd &zdd)
  {
    return nodecount(zdd.file);
  }

  label_t zdd_varcount(const zdd &zdd)
  {
    return varcount(zdd.file);
  }

  uint64_t zdd_size(const zdd &zdd)
  {
    return is_sink(zdd)
      ? value_of(zdd)
      : count<path_count_policy>(zdd, zdd_varcount(zdd));
  }
}
