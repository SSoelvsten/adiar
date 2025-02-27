#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/select.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  template <typename AssignmentPolicy>
  class bdd_restrict_policy
    : public bdd_policy
    , public AssignmentPolicy
  {
  public:
    template <typename Arg>
    bdd_restrict_policy(const Arg& a)
      : AssignmentPolicy(a)
    {}

  public:
    internal::select_rec
    process(const bdd::node_type& n)
    {
      switch (AssignmentPolicy::current_assignment()) {
      case assignment::False: {
        return n.low();
      }
      case assignment::True: {
        return n.high();
      }
      case assignment::None:
      default: {
        return n;
      }
      }
    }

    bdd
    terminal(bool terminal_val)
    {
      return bdd_terminal(terminal_val);
    }

    static constexpr bool skip_reduce = true;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Variant: Generator-based Restrict
  class restrict_generator_mgr
  {
    assignment _current = assignment::None;

    const generator<pair<bdd::label_type, bool>>& _generator;
    optional<pair<bdd::label_type, bool>> _peeked;

  public:
    restrict_generator_mgr(const generator<pair<bdd::label_type, bool>>& g)
      : _generator(g)
    {
      _peeked = _generator();
    }

    bool
    empty() const
    {
      return !_peeked.has_value();
    }

    void
    setup_level(bdd::label_type level)
    {
      while (_peeked && _peeked.value().first < level) { _peeked = _generator(); }
      _current = _peeked && _peeked.value().first == level
        ? static_cast<assignment>(_peeked.value().second)
        : assignment::None;
    }

  protected:
    const assignment&
    current_assignment()
    {
      return _current;
    }
  };

  __bdd
  bdd_restrict(const exec_policy& ep,
               const bdd& f,
               const generator<pair<bdd::label_type, bool>>& xs)
  {
    bdd_restrict_policy<restrict_generator_mgr> policy(xs);

    if (policy.empty() || bdd_isterminal(f)) { return f; }
    return internal::select(ep, f, policy);
  }

  __bdd
  bdd_restrict(const bdd& f, const generator<pair<bdd::label_type, bool>>& xs)
  {
    return bdd_restrict(exec_policy(), f, xs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Overload: Single-variable Restrict
  __bdd
  bdd_restrict(const exec_policy& ep, const bdd& f, bdd::label_type var, bool val)
  {
    return bdd_restrict(ep, f, make_generator(make_pair<bdd::label_type, bool>(var, val)));
  }

  __bdd
  bdd_restrict(const bdd& f, bdd::label_type var, bool val)
  {
    return bdd_restrict(exec_policy(), f, var, val);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Overload: Root Restrict
  __bdd
  bdd_low(const exec_policy& ep, const bdd& f)
  {
    return bdd_restrict(ep, f, bdd_topvar(f), false);
  }

  __bdd
  bdd_low(const bdd& f)
  {
    return bdd_low(exec_policy(), f);
  }

  __bdd
  bdd_high(const exec_policy& ep, const bdd& f)
  {
    return bdd_restrict(ep, f, bdd_topvar(f), true);
  }

  __bdd
  bdd_high(const bdd& f)
  {
    return bdd_high(exec_policy(), f);
  }
}
