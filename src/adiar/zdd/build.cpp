#include <adiar/zdd.h>

#include <limits>

#include <adiar/zdd/zdd_policy.h>
#include <adiar/domain.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/build.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_terminal(bool value)
  {
    return internal::build_terminal<zdd_policy>(value);
  }

  zdd zdd_empty()
  {
    return internal::build_terminal<zdd_policy>(false);
  }

  zdd zdd_null()
  {
    return internal::build_terminal<zdd_policy>(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_ithvar_policy : public zdd_policy
  {
  private:
    const zdd::label_type var;

  public:
    zdd_ithvar_policy(zdd::label_type v) : var(v)
    { }

  public:
    static constexpr bool init_terminal = true;

    constexpr bool
    skip(const zdd_policy::label_type &) const
    { return false; }

    inline zdd_policy::node_type
    make_node(const zdd_policy::label_type &l, const zdd_policy::pointer_type &r) const
    {
      if (l == var) {
        return zdd_policy::node_type(l, zdd_policy::max_id, zdd_policy::pointer_type(false), r);
      }
      return zdd_policy::node_type(l, zdd_policy::max_id, r, r);
    }
  };

  zdd zdd_ithvar(const zdd::label_type var, const generator<zdd::label_type> &dom)
  {
    // TODO: Move empty dom edge-case inside of `internal::build_chain<>`?

    zdd_ithvar_policy p(var);
    const zdd res = internal::build_chain<>(p, dom);

    return zdd_istrue(res) ? zdd_empty() : res;
  }

  zdd zdd_ithvar(const zdd::label_type var)
  {
    const shared_file<domain_var> dom = domain_get();
    internal::file_stream<domain_var, true> ds(dom);

    return zdd_ithvar(var, make_generator(ds));
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_nithvar_policy : public zdd_policy
  {
  private:
    const zdd::label_type var;

  public:
    zdd_nithvar_policy(zdd::label_type v) : var(v)
    { }

  public:
    static constexpr bool init_terminal = true;

    inline bool
    skip(const zdd_policy::label_type &l) const
    { return l == var; }

    inline zdd_policy::node_type
    make_node(const zdd_policy::label_type &l, const zdd_policy::pointer_type &r) const
    { return zdd_policy::node_type(l, zdd_policy::max_id, r, r); }
  };

  zdd zdd_nithvar(const zdd::label_type var, const generator<zdd::label_type> &dom)
  {
    zdd_nithvar_policy p(var);
    return internal::build_chain<>(p, dom);
  }

  zdd zdd_nithvar(const zdd::label_type var)
  {
    const shared_file<domain_var> dom = domain_get();
    internal::file_stream<domain_var, true> ds(dom);

    return zdd_nithvar(var, make_generator(ds));
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_vars(const generator<zdd::label_type> &vars)
  {
    internal::chain_high<zdd_policy> p;
    return internal::build_chain<>(p, vars);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singleton(const zdd::label_type label)
  {
    return internal::build_ithvar<zdd_policy>(label);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singletons(const generator<zdd::label_type> &vars)
  {
    internal::chain_low<zdd_policy> p;
    return internal::build_chain<>(p, vars);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_powerset(const generator<zdd::label_type> &vars)
  {
    internal::chain_both<zdd_policy> p;
    return internal::build_chain<>(p, vars);
  }

  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_bot(const generator<zdd::label_type> &/*dom*/)
  {
    return zdd_empty();
  }

  zdd zdd_bot()
  {
    return zdd_empty();
  }

  zdd zdd_top(const generator<zdd::label_type> &dom)
  {
    return zdd_powerset(dom);
  }

  zdd zdd_top()
  {
    if (!domain_isset()) {
      return zdd_null();
    }

    const shared_file<domain_var> dom = domain_get();
    internal::file_stream<domain_var, true> ds(dom);

    return zdd_powerset(make_generator(ds));
  }
}
