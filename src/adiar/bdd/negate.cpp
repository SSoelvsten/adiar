#include <adiar/bdd.h>

namespace adiar
{
  bdd bdd_not(const bdd &in)
  {
    bdd out = in;
    out.negate = !in.negate;
    return out;
  }

  bdd bdd_not(bdd &&b)
  {
    b.negate = !b.negate;
    return b;
  }
}
