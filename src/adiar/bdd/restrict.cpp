#include "restrict.h"

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

#include <adiar/bdd/bdd.h>
#include <adiar/bdd/build.h>

namespace adiar
{
  class restrict_policy : public bdd_policy
  {
  public:
    typedef assignment_file substitution_t;

    class substitution_mgr
    {
      assignment_stream<> as;
      assignment_t a;

    public:
      substitution_mgr(substitution_t af) : as(af)
      {
        a = as.pull();
      }

      void setup_for_level(label_t level) {
        while (label_of(a) < level && as.can_pull()) {
          a = as.pull();
        }
      }

      substitute_rec resolve_node(const node_t &n, const label_t level)
      {
        adiar_debug(label_of(n) == level, "level should be of the given node");

        if (label_of(a) == level) {
          return substitute_rec_skipto { value_of(a) ? n.high : n.low };
        } else {
          return substitute_rec_output { n };
        }
      }
    };

  public:
    static inline bool disjoint_labels(const assignment_file &na, const bdd &dd)
    { return adiar::disjoint_labels<assignment_file, assignment_stream<>>(na, dd); }

    static inline bdd sink(bool sink_val)
    { return bdd_sink(sink_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &dd, const assignment_file &assignment)
  {
    return substitute<restrict_policy>(dd, assignment);
  }
}
