#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/domain.h>
#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar
{
  class zdd_complement_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = false;

    static constexpr bool cut_true_terminal = true;
    static constexpr bool cut_false_terminal = true;

    static constexpr size_t mult_factor = 2u;

  public:
    static zdd on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd on_terminal_input(const bool terminal_value, const zdd& /*dd*/, const internal::label_file &universe)
    {
      return terminal_value
        // The entire universe minus Ø
        ? internal::build_chain<false, true, true, false, true>(universe)
        // The entire universe
        : internal::build_chain<true, true, true, true, true>(universe);
    }

    // LCOV_EXCL_START
    static zdd terminal(const bool /*terminal_value*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_END

    // We can improve this by knowing whether we are at the very last label. If
    // that is the case, and the high edge becomes F, then it can be skipped.
    //
    // Yet, this is only 2 nodes that we can kill; that is 4 arcs. This is 32
    // bytes of data and very few computation cycles. For very large cases the
    // shortcutting in branch-prediction probably offsets this?
    static internal::intercut_rec_output hit_existing(const zdd::node_t &n)
    {
      const zdd::ptr_t low = n.low().is_terminal() ? negate(n.low()) : n.low();
      const zdd::ptr_t high = n.high().is_terminal() ? negate(n.high()) : n.high();

      return internal::intercut_rec_output { low, high };
    }

    static internal::intercut_rec_output hit_cut(const zdd::ptr_t &target)
    {
      // T chain: We are definitely outside of the given set
      if (target.is_true()) {
        return internal::intercut_rec_output { target, target };
      }

      // Otherwise, check whether this variable is true and so we move to the T chain
      return internal::intercut_rec_output { target, zdd::ptr_t(true) };
    }

    // LCOV_EXCL_START
    static internal::intercut_rec_output miss_existing(const zdd::node_t &/*n*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_END
  };

  __zdd zdd_complement(const zdd &dd, const internal::label_file &universe)
  {
    return internal::intercut<zdd_complement_policy>(dd, universe);
  }

  __zdd zdd_complement(const zdd &dd)
  {
    const internal::label_file universe = adiar_get_domain();
    return internal::intercut<zdd_complement_policy>(dd, universe);
  }
}
