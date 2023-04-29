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

  private:
    class on_true_terminal_chain_policy : public zdd_policy
    {
    public:
      static constexpr bool init_terminal = false;

      constexpr bool
      skip(const typename dd_policy::label_t &) const
      { return false; }

      inline
      bdd::node_t
      make_node(const zdd::label_t &l, const zdd::ptr_t &r) const
      {
        if (r.is_terminal()) {
          adiar_debug(r.value() == false, "Root should be Ø");
          return zdd::node_t(l, zdd::MAX_ID, r, zdd_policy::ptr_t(true));
        } else {
          return zdd::node_t(l, zdd::MAX_ID, r, r);
        }
      }

      static constexpr bool link[2]           = {true, true};
      static constexpr bool terminal_value[2] = {true, true};
    };

  public:
    static zdd on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd on_terminal_input(const bool terminal_value, const zdd& /*dd*/,
                                 const shared_file<zdd::label_t> &universe)
    {
      if (terminal_value) { // Include everything but Ø
        on_true_terminal_chain_policy p;
        return internal::build_chain<>(p, universe);
      } else { // The complement of nothing is everything
        internal::chain_both<zdd_policy> p;
        return internal::build_chain<>(p, universe);
      }
    }

    // LCOV_EXCL_START
    static zdd terminal(const bool /*terminal_value*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_STOP

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
    // LCOV_EXCL_STOP
  };

  __zdd zdd_complement(const zdd &dd, const shared_file<zdd::label_t> &universe)
  {
    return internal::intercut<zdd_complement_policy>(dd, universe);
  }

  __zdd zdd_complement(const zdd &dd)
  {
    const shared_file<zdd::label_t> universe = adiar_get_domain();
    return internal::intercut<zdd_complement_policy>(dd, universe);
  }
}
