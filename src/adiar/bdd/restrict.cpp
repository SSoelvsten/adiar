#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/select.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    template <typename AssignmentMgr>
    static internal::select_rec
    keep_node(const bdd::node_type& n, AssignmentMgr& /*amgr*/)
    {
      return n;
    }

    template <typename AssignmentMgr>
    static internal::select_rec
    fix_false(const bdd::node_type& n, AssignmentMgr& /*amgr*/)
    {
      return n.low();
    }

    template <typename AssignmentMgr>
    static internal::select_rec
    fix_true(const bdd::node_type& n, AssignmentMgr& /*amgr*/)
    {
      return n.high();
    }

  public:
    template <typename AssignmentMgr>
    static inline bdd
    terminal(bool terminal_val, AssignmentMgr& /*amgr*/)
    {
      return bdd_terminal(terminal_val);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Variant: Generator-based Restrict
  class restrict_generator_mgr
  {
    const generator<pair<bdd::label_type, bool>>& _generator;
    optional<pair<bdd::label_type, bool>> _next_pair;

  public:
    restrict_generator_mgr(const generator<pair<bdd::label_type, bool>>& g)
      : _generator(g)
    {
      _next_pair = _generator();
    }

    bool
    empty() const
    {
      return !_next_pair.has_value();
    }

    assignment
    assignment_for_level(bdd::label_type level)
    {
      while (_next_pair && _next_pair.value().first < level) { _next_pair = _generator(); }
      return _next_pair && _next_pair.value().first == level
        ? static_cast<assignment>(_next_pair.value().second)
        : assignment::None;
    }
  };

  __bdd
  bdd_restrict(const exec_policy& ep,
               const bdd& f,
               const generator<pair<bdd::label_type, bool>>& xs)
  {
    restrict_generator_mgr amgr(xs);

    if (amgr.empty() || bdd_isterminal(f)) { return f; }
    return internal::select<bdd_restrict_policy>(ep, f, amgr);
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
