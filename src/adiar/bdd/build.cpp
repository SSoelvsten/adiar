#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/levelized_ofstream.h>
#include <adiar/internal/util.h>

namespace adiar
{
  template <typename Generator, bool negate>
  using bdd_chain_converter = internal::chain_converter<bdd_policy, Generator, negate>;

  //////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_const(bool value)
  {
    return internal::build_terminal<bdd_policy>(value);
  }

  bdd
  bdd_terminal(bool value)
  {
    return bdd_const(value);
  }

  bdd
  bdd_false()
  {
    return bdd_const(false);
  }

  bdd
  bdd_true()
  {
    return bdd_const(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_ithvar(bdd::label_type label)
  {
    return internal::build_ithvar<bdd_policy>(label);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_nithvar(bdd::label_type label)
  {
    return bdd_not(internal::build_ithvar<bdd_policy>(label));
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_and_policy : public bdd_policy
  {
  public:
    static constexpr bool init_terminal = true;

    constexpr bool
    skip(const bdd::label_type&) const
    {
      return false;
    }

    inline typename bdd::node_type
    make_node(const bdd::label_type& l, const bdd::pointer_type& r, const bool negated) const
    {
      const bdd::pointer_type low  = negated ? r : bdd::pointer_type(false);
      const bdd::pointer_type high = negated ? bdd::pointer_type(false) : r;

      return typename bdd::node_type(l, bdd::max_id, low, high);
    }
  };

  bdd
  bdd_and(const generator<pair<bdd::label_type, bool>>& vars)
  {
    bdd_and_policy p;
    bdd_chain_converter<decltype(vars), false> vars_wrapper(vars);

    return internal::build_chain<>(p, vars_wrapper);
  }

  bdd
  bdd_and(const generator<int>& vars)
  {
    bdd_and_policy p;
    bdd_chain_converter<decltype(vars), false> vars_wrapper(vars);

    return internal::build_chain<>(p, vars_wrapper);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_or(const generator<pair<bdd::label_type, bool>>& vars)
  {
    bdd_and_policy p;
    bdd_chain_converter<decltype(vars), true> vars_wrapper(vars);

    return bdd_not(internal::build_chain<>(p, vars_wrapper));
  }

  bdd
  bdd_or(const generator<int>& vars)
  {
    bdd_and_policy p;
    bdd_chain_converter<decltype(vars), true> vars_wrapper(vars);

    return bdd_not(internal::build_chain<>(p, vars_wrapper));
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_cube(const generator<pair<bdd::label_type, bool>>& vars)
  {
    return bdd_and(vars);
  }

  bdd
  bdd_cube(const generator<int>& vars)
  {
    return bdd_and(vars);
  }
}
