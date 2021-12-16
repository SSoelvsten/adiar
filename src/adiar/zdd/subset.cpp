#include "binop.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/tuple.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

#include <adiar/zdd/zdd.h>
#include <adiar/zdd/build.h>

#include <adiar/assert.h>

namespace adiar
{
  class zdd_offset_policy : public zdd_policy
  {
  public:
  public:
    typedef label_file substitution_t;

    class substitution_mgr
    {
      label_stream<> ls;
      label_t l;

    public:
      substitution_mgr(substitution_t lf) : ls(lf)
      {
        l = ls.pull();
      }

      void setup_for_level(label_t level) {
        while (l < level && ls.can_pull()) {
          l = ls.pull();
        }
      }

      substitute_rec resolve_node(const node_t &n, const label_t level)
      {
        adiar_debug(label_of(n) == level, "level should be of the given node");

        if (l == level) {
          return substitute_rec_skipto { n.low };
        } else {
          return substitute_rec_output { n };
        }
      }
    };

  public:
    static inline bool disjoint_labels(const label_file &na, const zdd &dd)
    { return adiar::disjoint_labels<label_file, label_stream<>>(na, dd); }

    static inline zdd sink(bool sink_val)
    { return zdd_sink(sink_val); }
  };

  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_offset(const zdd &dd, const label_file &l)
  {
    return substitute<zdd_offset_policy>(dd, l);
  }
}
