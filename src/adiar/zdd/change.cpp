#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/data.h>

#include <adiar/internal/intercut.h>

namespace adiar
{
  class zdd_change_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = true;

    static constexpr bool cut_true_sink = true;
    static constexpr bool cut_false_sink = false;

    static constexpr size_t mult_factor = 1u;

  public:
    static zdd on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd on_sink_input(const bool sink_value, const zdd& dd, const label_file &labels)
    {
      return sink_value ? zdd_vars(labels) : dd;
    }

    static zdd sink(const bool sink_value)
    {
      return zdd_sink(sink_value);
    }

    static intercut_rec hit_existing(const node_t &n)
    {
      if (is_false(n.low)) {
        return intercut_rec_skipto { n.high };
      }

      return intercut_rec_output { n.high, n.low };
    }

    static intercut_rec_output hit_cut(const ptr_t target)
    {
      return intercut_rec_output { create_sink_ptr(false), target };
    }

    static intercut_rec_output miss_existing(const node_t &n)
    {
      return intercut_rec_output { n.low, n.high };
    }
  };


  __zdd zdd_change(const zdd &dd, const label_file &labels)
  {
    return intercut<zdd_change_policy>(dd, labels);
  }
}
