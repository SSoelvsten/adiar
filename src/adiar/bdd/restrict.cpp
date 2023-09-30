#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/assignment.h>
#include <adiar/internal/util.h>
#include <adiar/internal/algorithms/substitution.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  class substitute_assignment_file_mgr
  {
    file_stream<map_pair<bdd::label_type, assignment>> mps;
    map_pair<bdd::label_type, assignment> mp;

  public:
    substitute_assignment_file_mgr(const shared_file<map_pair<bdd::label_type, assignment>> &mpf)
      : mps(mpf)
    {
      mp = mps.pull();
    }

    assignment assignment_for_level(bdd::label_type level) {
      while (mp.level() < level && mps.can_pull()) {
        mp = mps.pull();
      }

      return mp.level() == level ? mp.value() : assignment::None;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // TODO: template on manager?
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    static internal::substitute_rec keep_node(const bdd::node_type &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_output { n }; }

    static internal::substitute_rec fix_false(const bdd::node_type &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.low() }; }

    static internal::substitute_rec fix_true(const bdd::node_type &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.high() }; }

  public:
    static inline bdd terminal(bool terminal_val, substitute_assignment_file_mgr &/*amgr*/)
    { return bdd_terminal(terminal_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<>
  struct internal::level_stream_t<shared_file<map_pair<bdd::label_type, assignment>>>
  {
    template<bool reverse = false>
    using stream_t = internal::file_stream<map_pair<bdd::label_type, assignment>>;
  };

  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd &f,
                     const shared_file<map_pair<bdd::label_type, assignment>> &a)
  {
    if (a->size() == 0
        || bdd_isterminal(f)
        || internal::disjoint_levels(a, f)) {
      return f;
    }

    substitute_assignment_file_mgr amgr(a);
    return internal::substitute<bdd_restrict_policy>(ep, f, amgr);
  }

  __bdd bdd_restrict(const bdd &f,
                     const shared_file<map_pair<bdd::label_type, assignment>> &a)
  {
    return bdd_restrict(exec_policy(), f, a);
  }
}
