#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/util.h>

#include <adiar/internal/algorithms/substitution.h>

namespace adiar
{
  template<internal::substitute_act FIX_VALUE>
  class zdd_subset_label_act
  {
    label_stream<> ls;

    zdd::label_t l_incl;
    zdd::label_t l_excl;

    bool has_excl = false;

    // We will rememeber how far the algorithm in substitution.h has got
    zdd::label_t alg_level = 0;

  public:
    typedef label_file action_t;

    zdd_subset_label_act(const action_t &lf) : ls(lf)
    {
      l_incl = ls.pull();
    }

  private:
    inline void forward_to_level(const zdd::label_t new_level) {
      adiar_debug(alg_level <= new_level,
                  "The algorithm should ask for the levels in increasing order.");

      alg_level = new_level;

      if (l_incl < new_level && has_excl) {
        l_incl = l_excl;
        has_excl = false;
      }

      while (l_incl < new_level && ls.can_pull()) {
        l_incl = ls.pull();
      }
    }

  public:
    internal::substitute_act action_for_level(const zdd::label_t new_level) {
      forward_to_level(new_level);
      return l_incl == new_level ? FIX_VALUE : internal::substitute_act::KEEP;
    }

  public:
    bool has_level_incl() {
      return alg_level <= l_incl || ls.can_pull();
    }

    zdd::label_t level_incl()
    {
      return l_incl;
    }

    bool has_level_excl() {
      return alg_level < l_incl || has_excl || ls.can_pull();
    }

    zdd::label_t level_excl()
    {
      if (alg_level < l_incl) { return l_incl; }

      if (!has_excl) {
        l_excl = ls.pull();
        has_excl = true;
      }

      return l_excl;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<typename zdd_subset_act>
  class zdd_offset_policy : public zdd_policy
  {
  public:
    static internal::substitute_rec keep_node(const zdd::node_t &n, zdd_subset_act &/*amgr*/)
    { return internal::substitute_rec_output { n }; }

    static internal::substitute_rec fix_false(const zdd::node_t &n, zdd_subset_act &/*amgr*/)
    { return internal::substitute_rec_skipto { n.low() }; }

    static internal::substitute_rec fix_true(const zdd::node_t &/*n*/, zdd_subset_act &/*amgr*/)
    {
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

  public:
    static inline zdd terminal(bool terminal_val,
                               zdd_subset_label_act<internal::substitute_act::FIX_FALSE>& /*amgr*/)
    { return zdd_terminal(terminal_val); }
  };

  __zdd zdd_offset(const zdd &dd, const label_file &l)
  {
    if (l.size() == 0
        || is_terminal(dd)
        || internal::disjoint_labels<label_file, label_stream<>>(l, dd)) {
      return dd;
    }

    zdd_subset_label_act<internal::substitute_act::FIX_FALSE> amgr(l);
    return internal::substitute<zdd_offset_policy<zdd_subset_label_act<internal::substitute_act::FIX_FALSE>>>(dd, amgr);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename zdd_subset_act>
  class zdd_onset_policy : public zdd_policy
  {
  public:
    static internal::substitute_rec keep_node(const zdd::node_t &n, zdd_subset_act &amgr)
    {
      if (amgr.has_level_incl()) {
        // If recursion goes past the intended level, then it is replaced with
        // the false terminal.
        const zdd::ptr_t low  = n.low().is_terminal() || n.low().label() > amgr.level_incl()
          ? zdd::ptr_t(false)
          : n.low();

        // If this applies to high, then the node should be skipped entirely.
        if (n.high().is_terminal() || n.high().label() > amgr.level_incl()) {
          return internal::substitute_rec_skipto { low };
        }
        return internal::substitute_rec_output { zdd::node_t(n.uid(), low, n.high()) };
      }
      return internal::substitute_rec_output { n };
    }

    static internal::substitute_rec fix_false(const zdd::node_t &/*n*/, zdd_subset_act &/*amgr*/)
    {
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    static internal::substitute_rec fix_true(const zdd::node_t &n, zdd_subset_act &amgr)
    {
      if (amgr.has_level_excl()) {
        if (n.high().is_terminal() || n.high().label() > amgr.level_excl()) {
          return internal::substitute_rec_skipto { zdd::ptr_t(false) };
        }
      }
      return internal::substitute_rec_output { zdd::node_t(n.uid(), zdd::ptr_t(false), n.high()) };
    }

  public:
    static inline zdd terminal(bool terminal_val,
                               zdd_subset_label_act<internal::substitute_act::FIX_TRUE>& amgr)
    {
      return zdd_terminal(!amgr.has_level_excl() && terminal_val);
    }
  };

  __zdd zdd_onset(const zdd &dd, const label_file &l)
  {
    if (l.size() == 0 || (is_false(dd))) { return dd; }
    if ((is_true(dd)) || internal::disjoint_labels<label_file, label_stream<>>(l, dd)) {
      return zdd_empty();
    }

    zdd_subset_label_act<internal::substitute_act::FIX_TRUE> amgr(l);
    return internal::substitute<zdd_onset_policy<zdd_subset_label_act<internal::substitute_act::FIX_TRUE>>>(dd, amgr);
  }
}
