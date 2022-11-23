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

    static zdd on_terminal_input(const bool terminal_value, const zdd& dd, const internal::label_file &labels)
    {
      return terminal_value ? zdd_vars(labels) : dd;
    }

    static zdd terminal(const bool terminal_value)
    {
      return zdd_terminal(terminal_value);
    }

    static internal::intercut_rec hit_existing(const zdd::node_t &n)
    {
      if (n.low().is_false()) {
        return internal::intercut_rec_skipto { n.high() };
      }

      return internal::intercut_rec_output { n.high(), n.low() };
    }

    static internal::intercut_rec_output hit_cut(const zdd::ptr_t &target)
    {
      return internal::intercut_rec_output { zdd::ptr_t(false), target };
    }

    static internal::intercut_rec_output miss_existing(const zdd::node_t &n)
    {
      return internal::intercut_rec_output { n.low(), n.high() };
    }
  };

  __zdd zdd_change(const zdd &dd, const internal::label_file &labels)
  {
    return internal::intercut<zdd_change_policy>(dd, labels);
  }
}
