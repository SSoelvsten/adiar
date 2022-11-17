#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_types/node.h>

namespace adiar
{
  class zdd_change_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = true;

    static constexpr bool cut_true_terminal = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 1u;

  public:
    static zdd on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd on_terminal_input(const bool terminal_value, const zdd& dd, const label_file &labels)
    {
      return terminal_value ? zdd_vars(labels) : dd;
    }

    static zdd terminal(const bool terminal_value)
    {
      return zdd_terminal(terminal_value);
    }

    static intercut_rec hit_existing(const node &n)
    {
      if (n.low().is_false()) {
        return intercut_rec_skipto { n.high() };
      }

      return intercut_rec_output { n.high(), n.low() };
    }

    static intercut_rec_output hit_cut(const ptr_uint64 target)
    {
      return intercut_rec_output { ptr_uint64(false), target };
    }

    static intercut_rec_output miss_existing(const node &n)
    {
      return intercut_rec_output { n.low(), n.high() };
    }
  };


  __zdd zdd_change(const zdd &dd, const label_file &labels)
  {
    return intercut<zdd_change_policy>(dd, labels);
  }
}
