#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/functional.h>

#include <adiar/internal/unreachable.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar
{
  class zdd_expand_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = false;

    static constexpr bool cut_true_terminal = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 2u;

  public:
    static zdd on_empty_labels(const zdd& dd)
    {
      return dd;
    }


    static zdd on_terminal_input(const bool terminal_value,
                                 const zdd& dd,
                                 const internal::shared_file<zdd::label_t> &labels)
    {
      // TODO: simplify when labels are a generator
      if (terminal_value) {
        internal::file_stream<zdd::label_t, true> ls(labels);
        return zdd_powerset(make_generator(ls));
      } else {
        return dd;
      }
    }

    // LCOV_EXCL_START
    static zdd terminal(const bool /*terminal_value*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_STOP

    // LCOV_EXCL_START
    static inline internal::intercut_rec hit_existing(const zdd::node_t &/*n*/)
    {
      // The user should NOT have supplied a label that hits any existing nodes.
      adiar_unreachable();
    }
    // LCOV_EXCL_STOP

    static inline internal::intercut_rec_output hit_cut(const zdd::ptr_t &target)
    {
      return internal::intercut_rec_output { target, target };
    }

    static inline internal::intercut_rec_output miss_existing(const zdd::node_t &n)
    {
      return internal::intercut_rec_output { n.low(), n.high() };
    }
  };

  __zdd zdd_expand(const zdd &dd, const generator<zdd::label_t> &vars)
  {
    return internal::intercut<zdd_expand_policy>(dd, vars);
  }
}
