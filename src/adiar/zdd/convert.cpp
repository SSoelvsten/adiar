#include <adiar/bdd/bdd_policy.h>
#include <adiar/domain.h>
#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/convert.h>
#include <adiar/internal/algorithms/intercut.h>

namespace adiar
{
  __zdd
  zdd_from(const exec_policy& ep, const bdd& f, const generator<zdd::label_type>& dom)
  {
    return internal::intercut<internal::convert_dd_policy<zdd_policy, bdd_policy>>(ep, f, dom);
  }

  __zdd
  zdd_from(const bdd& f, const generator<zdd::label_type>& dom)
  {
    return zdd_from(exec_policy(), f, dom);
  }

  __zdd
  zdd_from(const exec_policy& ep, const bdd& f)
  {
    const internal::shared_file<zdd::label_type> dom = domain_get();
    internal::ifstream<domain_var> ds(dom);

    return zdd_from(ep, f, make_generator(ds));
  }

  __zdd
  zdd_from(const bdd& f)
  {
    return zdd_from(exec_policy(), f);
  }
}
