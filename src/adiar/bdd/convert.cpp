#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/domain.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/convert.h>
#include <adiar/internal/algorithms/intercut.h>

namespace adiar
{
  __bdd
  bdd_from(const exec_policy& ep, const zdd& A, const generator<bdd::label_type>& dom)
  {
    return internal::intercut<internal::convert_dd_policy<bdd_policy, zdd_policy>>(ep, A, dom);
  }

  __bdd
  bdd_from(const zdd& A, const generator<bdd::label_type>& dom)
  {
    return bdd_from(exec_policy(), A, dom);
  }

  __bdd
  bdd_from(const exec_policy& ep, const zdd& A)
  {
    const internal::shared_file<bdd::label_type> dom = domain_get();
    internal::ifstream<domain_var> ds(dom);

    return bdd_from(ep, A, make_generator(ds));
  }

  __bdd
  bdd_from(const zdd& A)
  {
    return bdd_from(exec_policy(), A);
  }
}
