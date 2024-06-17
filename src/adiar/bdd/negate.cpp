#include <utility>

#include <adiar/bdd.h>

namespace adiar
{
  bdd
  bdd_not(const bdd& in)
  {
    bdd out     = in;
    out._negate = !in._negate;
    return out;
  }

  bdd
  bdd_not(bdd&& b)
  {
    b._negate = !b._negate;
    return std::move(b);
  }
}
