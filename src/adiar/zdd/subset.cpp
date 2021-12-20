#include "subset.h"

#include <adiar/file_stream.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

#include <adiar/zdd/zdd.h>
#include <adiar/zdd/build.h>

#include <adiar/assert.h>

namespace adiar
{
  template<substitute_act FIX_VALUE>
  class zdd_subset_label_act
  {
    label_stream<> ls;

    label_t l_incl;
    label_t l_excl;

    bool has_excl = false;

    // We will rememeber how far the algorithm in substitution.h has got
    label_t alg_level = 0;

  public:
    typedef label_file action_t;

    zdd_subset_label_act(const action_t &lf) : ls(lf)
    {
      l_incl = ls.pull();
    }

  private:
    inline void forward_to_level(const label_t new_level) {
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
    substitute_act action_for_level(const label_t new_level) {
      forward_to_level(new_level);
      return l_incl == new_level ? FIX_VALUE : substitute_act::KEEP;
    }

  public:
    bool has_level_incl() {
      return alg_level <= l_incl || ls.can_pull();
    }

    label_t level_incl()
    {
      return l_incl;
    }

    bool has_level_excl() {
      return alg_level < l_incl || has_excl || ls.can_pull();
    }

    label_t level_excl()
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
    static substitute_rec keep_node(const node_t &n, zdd_subset_act &/*amgr*/)
    { return substitute_rec_output { n }; }

    static substitute_rec fix_false(const node_t &n, zdd_subset_act &/*amgr*/)
    { return substitute_rec_skipto { n.low }; }

    static substitute_rec fix_true(const node_t &/*n*/, zdd_subset_act &/*amgr*/)
    {
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

  public:
    static inline zdd sink(bool sink_val,
                           zdd_subset_label_act<substitute_act::FIX_FALSE>& /*amgr*/)
    { return zdd_sink(sink_val); }
  };

  __zdd zdd_offset(const zdd &dd, const label_file &l)
  {
    if (l.size() == 0
        || is_sink(dd, is_any)
        || disjoint_labels<label_file, label_stream<>>(l, dd)) {
      return dd;
    }

    zdd_subset_label_act<substitute_act::FIX_FALSE> amgr(l);
    return substitute<zdd_offset_policy<zdd_subset_label_act<substitute_act::FIX_FALSE>>>(dd, amgr);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename zdd_subset_act>
  class zdd_onset_policy : public zdd_policy
  {
  public:
    static substitute_rec keep_node(const node_t &n, zdd_subset_act &amgr)
    {
      if (amgr.has_level_incl()) {
        // If recursion goes past the intended level, then it is replaced with
        // the false sink.
        const ptr_t low  = is_sink(n.low) || label_of(n.low) > amgr.level_incl()
          ? create_sink_ptr(false)
          : n.low;

        // If this applies to high, then the node should be skipped entirely.
        if (is_sink(n.high) || label_of(n.high) > amgr.level_incl()) {
          return substitute_rec_skipto { low };
        }
        return substitute_rec_output { create_node(n.uid, low, n.high) };
      }
      return substitute_rec_output { n };
    }

    static substitute_rec fix_false(const node_t &/*n*/, zdd_subset_act &/*amgr*/)
    {
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    static substitute_rec fix_true(const node_t &n, zdd_subset_act &amgr)
    {
      if (amgr.has_level_excl()) {
        if (is_sink(n.high) || label_of(n.high) > amgr.level_excl()) {
          return substitute_rec_skipto { create_sink_ptr(false) };
        }
      }
      return substitute_rec_output { create_node(n.uid, create_sink_ptr(false), n.high) };
    }

  public:
    static inline zdd sink(bool sink_val,
                           zdd_subset_label_act<substitute_act::FIX_TRUE>& amgr)
    {
      return zdd_sink(!amgr.has_level_excl() && sink_val);
    }
  };

  __zdd zdd_onset(const zdd &dd, const label_file &l)
  {
    if (l.size() == 0 || is_sink(dd, is_false)) { return dd; }
    if (is_sink(dd, is_true) || disjoint_labels<label_file, label_stream<>>(l, dd)) {
      return zdd_empty();
    }

    zdd_subset_label_act<substitute_act::FIX_TRUE> amgr(l);
    return substitute<zdd_onset_policy<zdd_subset_label_act<substitute_act::FIX_TRUE>>>(dd, amgr);
  }
}
