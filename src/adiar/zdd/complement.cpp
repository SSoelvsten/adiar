#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/data.h>
#include <adiar/domain.h>

#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/algorithms/intercut.h>

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

    static zdd on_terminal_input(const bool terminal_value, const zdd& /*dd*/, const label_file &universe)
    {
      return terminal_value
        // The entire universe minus Ø
        ? build_chain<false, true, true, false, true>(universe)
        // The entire universe
        : build_chain<true, true, true, true, true>(universe);
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
    static intercut_rec_output hit_existing(const node_t &n)
    {
      const ptr_t low = is_terminal(n.low) ? negate(n.low) : n.low;
      const ptr_t high = is_terminal(n.high) ? negate(n.high) : n.high;

      return intercut_rec_output { low, high };
    }

    static intercut_rec_output hit_cut(const ptr_t target)
    {
      // T chain: We are definitely outside of the given set
      if (is_true(target)) {
        return intercut_rec_output { target, target };
      }

      // Otherwise, check whether this variable is true and so we move to the T chain
      return intercut_rec_output { target, create_terminal_ptr(true) };
    }

    // LCOV_EXCL_START
    static intercut_rec_output miss_existing(const node_t &/*n*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_END
  };

  __zdd zdd_complement(const zdd &dd, const label_file &universe)
  {
    return intercut<zdd_complement_policy>(dd, universe);
  }

  __zdd zdd_complement(const zdd &dd)
  {
    const label_file universe = adiar_get_domain();
    return intercut<zdd_complement_policy>(dd, universe);
  }
}
