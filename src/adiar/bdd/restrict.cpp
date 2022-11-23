#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/assignment.h>
#include <adiar/internal/util.h>
#include <adiar/internal/algorithms/substitution.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>

namespace adiar
{
  class bdd_restrict_policy : public bdd_policy
  {
  public:
    static internal::substitute_rec keep_node(const bdd::node_t &n, internal::substitute_assignment_act &/*amgr*/)
    { return internal::substitute_rec_output { n }; }

    static internal::substitute_rec fix_false(const bdd::node_t &n, internal::substitute_assignment_act &/*amgr*/)
    { return internal::substitute_rec_skipto { n.low() }; }

    static internal::substitute_rec fix_true(const bdd::node_t &n, internal::substitute_assignment_act &/*amgr*/)
    { return internal::substitute_rec_skipto { n.high() }; }

  public:
    static inline bdd terminal(bool terminal_val, internal::substitute_assignment_act &/*amgr*/)
    { return bdd_terminal(terminal_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &dd, const internal::assignment_file &a)
  {
    if (a.size() == 0
        || is_terminal(dd)
        || internal::disjoint_labels<internal::assignment_file, internal::assignment_stream<>>(a, dd)) {
      return dd;
    }

    internal::substitute_assignment_act amgr(a);
    return internal::substitute<bdd_restrict_policy>(dd, amgr);
  }
}
