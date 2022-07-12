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
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    static substitute_rec keep_node(const node_t &n, substitute_assignment_act &/*amgr*/)
    { return substitute_rec_output { n }; }

    static substitute_rec fix_false(const node_t &n, substitute_assignment_act &/*amgr*/)
    { return substitute_rec_skipto { n.low }; }

    static substitute_rec fix_true(const node_t &n, substitute_assignment_act &/*amgr*/)
    { return substitute_rec_skipto { n.high }; }

  public:
    static inline bdd sink(bool sink_val,
                           substitute_assignment_act &/*amgr*/)
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
    return substitute<bdd_restrict_policy>(dd, amgr);
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
