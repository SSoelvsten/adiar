#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/util.h>
#include <adiar/internal/algorithms/substitution.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    template<typename AssignmentMgr>
    static internal::substitute_rec keep_node(const bdd::node_type &n, AssignmentMgr &/*amgr*/)
    { return internal::substitute_rec_output { n }; }

    template<typename AssignmentMgr>
    static internal::substitute_rec fix_false(const bdd::node_type &n, AssignmentMgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.low() }; }

    template<typename AssignmentMgr>
    static internal::substitute_rec fix_true(const bdd::node_type &n, AssignmentMgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.high() }; }

  public:
    template<typename AssignmentMgr>
    static inline bdd terminal(bool terminal_val, AssignmentMgr &/*amgr*/)
    { return bdd_terminal(terminal_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Variant: Generator-based Restrict
  class restrict_generator_mgr
  {
    const generator<pair<bdd::label_type, bool>> &_generator;
    pair<bdd::label_type, bool> _next_pair;

  public:
    restrict_generator_mgr(const generator<pair<bdd::label_type, bool>> &g)
      : _generator(g)
    {
      _next_pair = _generator();
    }

    bool empty() const
    {
      return bdd::max_label < _next_pair.first;
    }

    assignment assignment_for_level(bdd::label_type level)
    {
      while (_next_pair.first < level) {
        _next_pair = _generator();
      }
      return _next_pair.first == level
        ? static_cast<assignment>(_next_pair.second)
        : assignment::None;
    }
  };


  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd &f,
                     const generator<pair<bdd::label_type, bool>> &xs)
  {
    restrict_generator_mgr amgr(xs);

    if (amgr.empty() || bdd_isterminal(f)) {
      return f;
    }
    return internal::substitute<bdd_restrict_policy>(ep, f, amgr);
  }

  __bdd bdd_restrict(const bdd &f,
                     const generator<pair<bdd::label_type, bool>> &xs)
  {
    return bdd_restrict(exec_policy(), f, xs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Overload: Single-variable Restrict
  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd &f, bdd::label_type var, bool val)
  {
    bool gen_called = false;
    auto gen = [&gen_called, &var, &val]() -> pair<bdd::label_type, bool> {
      if (gen_called) {
        return generator_end<pair<bdd::label_type, bool>>::value;
      }
      gen_called = true;
      return {var, val};
    };

    return bdd_restrict(ep, f, gen);
  }

  __bdd bdd_restrict(const bdd &f, bdd::label_type var, bool val)
  {
    return bdd_restrict(exec_policy(), f, var, val);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Overload: Root Restrict
  __bdd bdd_low(const exec_policy &ep, const bdd &f)
  {
    return bdd_restrict(ep, f, bdd_topvar(f), false);
  }

  __bdd bdd_low(const bdd &f)
  {
    return bdd_low(exec_policy(), f);
  }

  __bdd bdd_high(const exec_policy &ep, const bdd &f)
  {
    return bdd_restrict(ep, f, bdd_topvar(f), true);
  }

  __bdd bdd_high(const bdd &f)
  {
    return bdd_high(exec_policy(), f);
  }
}
