#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/cnl.h>

#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/product_construction.h>

namespace adiar {
  bool zdd_equal(const zdd &s1, const zdd &s2)
  {
    return is_isomorphic(s2, s1);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<cut_type ct_1, cut_type ct_2>
  class ignore_levels
  {
  public:
    static size_t pq1_upper_bound(const node_file &in_1, const node_file &in_2)
    {
      const safe_size_t max_2level_cut_1 = in_1->max_2level_cut[ct_1];
      const safe_size_t max_2level_cut_2 = in_2->max_2level_cut[ct_2];

      return to_size(max_2level_cut_1 * max_2level_cut_2);
    }

    static size_t pq2_upper_bound(const node_file &in_1, const node_file &in_2)
    {
      const safe_size_t max_1level_cut_1 = in_1->max_1level_cut[ct_1];
      const safe_size_t max_1level_cut_2 = in_2->max_1level_cut[ct_2];

      return to_size(max_1level_cut_1 * max_1level_cut_2);
    }

    static constexpr size_t memory_usage()
    {
      return 0u;
    }

  public:
    ignore_levels(const node_file &/*f1*/, const node_file &/*f2*/)
    { /* do nothing */ }

    void next_level(label_t /* level */)
    { /* do nothing */ }

    bool on_step()
    { return false; }

    static constexpr bool termination_value = false;
  };

  class zdd_subseteq_policy : public zdd_policy, public prod_mixed_level_merger
  {
  public:
    typedef ignore_levels<cut_type::INTERNAL_TRUE, cut_type::INTERNAL_TRUE> level_check_t;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 1;
    }

  public:
    static bool resolve_terminals(const node_t &v1, const node_t &v2, bool &ret_value)
    {
      if (v1.is_terminal() && v2.is_terminal()) {
        ret_value = !v1.value() || v2.value();
        return true;
      } if (v1.is_false()) {
        ret_value = true;
        return true;
      } else if (v2.is_true()) {
        ret_value = false;
        return true;
      }
      return false;
    }

  public:
    static bool resolve_singletons(const node_t &v1, const node_t &v2)
    {
      return v1.label() == v2.label()
        && v1.low() <= v2.low() && v1.high() <= v2.high();
    }

  public:
    template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, ptr_t r1, ptr_t r2)
    {
      // Are they both a terminal? If so, check whether the left-hand side is true
      // and not the right, which would contradict being an implication (i.e.
      // representing a subset).
      if (is_terminal(r1) && is_terminal(r2)) {
        return unflag(r1) > unflag(r2);
      }

      // Has the left-hand side fallen out of its set?
      if (is_false(r1)) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (is_false(r2)) {
        return true;
      }

      // Otherwise, recurse
      pq.push({ r1, r2 });
      return false;
    }

  public:
    static constexpr bool early_return_value = false;
    static constexpr bool no_early_return_value = true;
  };

  bool zdd_subseteq(const zdd &s1, const zdd &s2)
  {
    if (s1.file._file_ptr == s2.file._file_ptr) {
      return true;
    }

    return comparison_check<zdd_subseteq_policy>(s1, s2);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_disjoint_policy : public zdd_policy, public prod_mixed_level_merger
  {
  public:
    typedef ignore_levels<cut_type::ALL, cut_type::ALL> level_check_t;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 2;
    }

  public:
    static bool resolve_terminals(const node_t &v1, const node_t &v2, bool &ret_value)
    {
      ret_value = v1.is_false() || v2.is_false();
      return (v1.is_terminal() && v2.is_terminal()) || ret_value;
    }

  public:
    static bool resolve_singletons(const node_t &v1, const node_t &v2)
    {
      return v1.label() != v2.label()
        || v1.low() != v2.low()
        || v1.high() != v2.high();
    }

  public:
  template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, ptr_t r1, ptr_t r2)
    {
      // Are they both a terminal? If so, check whether they both are true, which
      // verify there is a satisfiying conjunction (i.e. representing a shared
      // path/element).
      if (is_terminal(r1) && is_terminal(r2)) {
        return value_of(r1) && value_of(r2);
      }

      // Has the left-hand side fallen out of its set?
      if (is_false(r1)) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (is_false(r2)) {
        return false;
      }

      // Otherwise, recurse
      pq.push({ r1, r2 });
      return false;
    }

  public:
    static constexpr bool early_return_value = false;
    static constexpr bool no_early_return_value = true;
  };

  bool zdd_disjoint(const zdd &s1, const zdd &s2)
  {
    if (s1.file._file_ptr == s2.file._file_ptr) {
      return is_false(s1);
    }

    return comparison_check<zdd_disjoint_policy>(s1, s2);
  }
}
