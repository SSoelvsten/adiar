#include "expand.h"

#include <adiar/data.h>

#include <adiar/internal/intercut.h>

#include <adiar/zdd/zdd.h>
#include <adiar/zdd/build.h>

namespace adiar
{
  class zdd_expand_policy : public zdd_policy
  {
  public:
    static zdd on_sink_input(const bool sink_value, const zdd& dd, const label_file &labels)
    {
      return sink_value ? zdd_powerset(labels) : dd;
    }

    static zdd sink(const bool sink_value)
    {
      adiar_unreachable();
      return zdd_sink(sink_value);
    }

    static inline intercut_rec hit_existing(const node_t &/*n*/)
    {
      // The user should NOT have supplied a label that hits any existing nodes.
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    static inline intercut_rec_output hit_cut(const ptr_t target)
    {
      return intercut_rec_output { target, target };
    }

    static inline intercut_rec_output miss_existing(const node_t &n)
    {
      return intercut_rec_output { n.low, n.high };
    }
  };

  __zdd zdd_expand(const zdd &dd, const label_file &labels)
  {
    return intercut<zdd_expand_policy>(dd, labels);
  }
}
