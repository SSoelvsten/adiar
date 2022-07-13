#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

namespace adiar
{
  class substitute_assignment_act
  {
    assignment_stream<> as;
    assignment_t a;

  public:
    typedef assignment_file action_t;

    substitute_assignment_act(const action_t &af) : as(af)
    {
      a = as.pull();
    }

    substitute_act action_for_level(label_t level) {
      while (label_of(a) < level && as.can_pull()) {
        a = as.pull();
      }

      if (label_of(a) == level) {
        return value_of(a) ? substitute_act::FIX_TRUE : substitute_act::FIX_FALSE;
      } else {
        return substitute_act::KEEP;
      }
    }
  };

  class substitute_predicate_act
  {
    act_predicate pred;

  public:
    substitute_predicate_act(const act_predicate &ap) : pred(ap)
    { }

    substitute_act action_for_level(label_t level) {
      return pred(level);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<typename bdd_restrict_act>
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    static substitute_rec keep_node(const node_t &n, bdd_restrict_act &/*amgr*/)
    { return substitute_rec_output { n }; }

    static substitute_rec fix_false(const node_t &n, bdd_restrict_act &/*amgr*/)
    { return substitute_rec_skipto { n.low }; }

    static substitute_rec fix_true(const node_t &n, bdd_restrict_act &/*amgr*/)
    { return substitute_rec_skipto { n.high }; }

  public:
    static inline bdd sink(bool sink_val,
                           bdd_restrict_act &/*amgr*/)
    { return bdd_sink(sink_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &dd, const assignment_file &a)
  {
    if (a.size() == 0
        || is_sink(dd)
        || disjoint_labels<assignment_file, assignment_stream<>>(a, dd)) {
      return dd;
    }

    substitute_assignment_act amgr(a);
    return substitute<bdd_restrict_policy<substitute_assignment_act>>(dd, amgr);
  }

  __bdd bdd_restrict(const bdd &dd, const act_predicate &lp)
  {
    if (is_sink(dd)) {
      return dd;
    }

    substitute_predicate_act amgr(lp);
    return substitute<bdd_restrict_policy<substitute_predicate_act>>(dd, amgr);
  }

  __bdd bdd_restrict(const bdd &dd, const label_t &var, const bool &val)
  {
    assignment_t var_assignment = { var, val };
    assignment_file af;
    {
      assignment_writer aw(af);
      aw << var_assignment;
    }

    return bdd_restrict(dd, af);
  }
}
