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
    file_stream<map_pair<bdd::label_t, assignment>> mps;
    map_pair<bdd::label_t, assignment> mp;

  public:
    substitute_assignment_file_mgr(const shared_file<map_pair<bdd::label_t, assignment>> &mpf)
      : mps(mpf)
    {
      mp = mps.pull();
    }

    assignment assignment_for_level(bdd::label_t level) {
      while (mp.level() < level && mps.can_pull()) {
        mp = mps.pull();
      }

      return mp.level() == level ? mp.value() : assignment::NONE;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // TODO: template on manager?
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    static internal::substitute_rec keep_node(const bdd::node_t &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_output { n }; }

    static internal::substitute_rec fix_false(const bdd::node_t &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.low() }; }

    static internal::substitute_rec fix_true(const bdd::node_t &n, substitute_assignment_file_mgr &/*amgr*/)
    { return internal::substitute_rec_skipto { n.high() }; }

  public:
    static inline bdd terminal(bool terminal_val, substitute_assignment_file_mgr &/*amgr*/)
    { return bdd_terminal(terminal_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &dd,
                     const shared_file<map_pair<bdd::label_t, assignment>> &a)
  {
    if (a->size() == 0
        || is_terminal(dd)
        || internal::disjoint_levels<shared_file<map_pair<bdd::label_t, assignment>>,
                                     internal::file_stream<map_pair<bdd::label_t, assignment>>>(a, dd)) {
      return dd;
    }

    substitute_assignment_file_mgr amgr(a);
    return internal::substitute<bdd_restrict_policy>(dd, amgr);
  }
}
