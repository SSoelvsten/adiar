#include "restrict.h"

#include <adiar/data.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>

#include <adiar/bdd/bdd.h>
#include <adiar/bdd/build.h>

namespace adiar
{
  class restrict_policy : public bdd_policy
  {
  public:
    static inline bdd resolve_empty_assignment(const bdd &in)
    { return in; }

  public:
    static inline bdd resolve_sink_root(const bdd& in, const assignment_file&)
    { return in; }

  public:
    static inline bdd resolve_disjoint_labels(const bdd &in, const assignment_file &)
    { return in; }

  public:
    static inline __bdd resolve_root_assign(const node_t &n, const assignment_t &a,
                                            const bdd &/*in*/, const assignment_file &,
                                            ptr_t &rec_child)
    {
      rec_child = value_of(a) ? n.high : n.low;
      if(is_sink(rec_child)) {
        return bdd_sink(value_of(rec_child));
      }
      return __bdd(); // Return nothing
    }

  public:
    static inline bdd sink(bool sink_val)
    { return bdd_sink(sink_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &dd, const assignment_file &assignment)
  {
    return substitute<restrict_policy>(dd, assignment);
  }
}
