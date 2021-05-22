#include "count.h"

namespace adiar
{
  size_t zdd_nodecount(const zdd &zdd)
  {
    return nodecount(zdd.file);
  }

  size_t zdd_varcount(const zdd &zdd)
  {
    return varcount(zdd.file);
  }
}
