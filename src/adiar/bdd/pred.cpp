#include <adiar/bdd.h>

#include <adiar/internal/algorithms/pred.h>

namespace adiar
{
  bool bdd_iscanonical(const bdd& f)
  {
    return internal::dd_iscanonical(f);
  }

  bool bdd_isterminal(const bdd& f)
  {
    return internal::dd_isterminal(f);
  }

  bool bdd_isfalse(const bdd& f)
  {
    return internal::dd_isfalse(f);
  }

  bool bdd_istrue(const bdd& f)
  {
    return internal::dd_istrue(f);
  }

  bool bdd_equal(const bdd &f, const bdd &g)
  {
    return internal::is_isomorphic(f, g);
  }

  bool bdd_unequal(const bdd &f, const bdd &g)
  {
    return !bdd_equal(f, g);
  }
}
