#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/cnl.h>

#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/prod2.h>

namespace adiar
{
  bool zdd_equal(const zdd &s1, const zdd &s2)
  {
    return internal::is_isomorphic(s2, s1);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<internal::cut_type ct_1, internal::cut_type ct_2>
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

    void next_level(zdd::label_t /* level */)
    { /* do nothing */ }

    bool on_step()
    { return false; }

    static constexpr bool termination_value = false;
  };

  class zdd_subseteq_policy : public zdd_policy, public internal::prod2_mixed_level_merger
  {
  public:
    typedef ignore_levels<internal::cut_type::INTERNAL_TRUE, internal::cut_type::INTERNAL_TRUE> level_check_t;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 1;
    }

  public:
    static bool resolve_terminals(const zdd::node_t &v1, const zdd::node_t &v2, bool &ret_value)
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
    static bool resolve_singletons(const zdd::node_t &v1, const zdd::node_t &v2)
    {
      return v1.label() == v2.label()
        && v1.low() <= v2.low() && v1.high() <= v2.high();
    }

  public:
    template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, zdd::ptr_t r1, zdd::ptr_t r2)
    {
      // Are they both a terminal? If so, check whether the left-hand side is true
      // and not the right, which would contradict being an implication (i.e.
      // representing a subset).
      if (r1.is_terminal() && r2.is_terminal()) {
        return unflag(r1) > unflag(r2);
      }

      // Has the left-hand side fallen out of its set?
      if (r1.is_false()) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (r2.is_false()) {
        return true;
      }

      // Otherwise, recurse
      pq.push({ {r1,r2}, {} });
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

    return internal::comparison_check<zdd_subseteq_policy>(s1, s2);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_disjoint_policy : public zdd_policy, public internal::prod2_mixed_level_merger
  {
  public:
    typedef ignore_levels<internal::cut_type::ALL, internal::cut_type::ALL> level_check_t;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 2;
    }

  public:
    static bool resolve_terminals(const zdd::node_t &v1, const zdd::node_t &v2, bool &ret_value)
    {
      ret_value = v1.is_false() || v2.is_false();
      return (v1.is_terminal() && v2.is_terminal()) || ret_value;
    }

  public:
    static bool resolve_singletons(const zdd::node_t &v1, const zdd::node_t &v2)
    {
      return v1.label() != v2.label()
        || v1.low() != v2.low()
        || v1.high() != v2.high();
    }

  public:
  template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, zdd::ptr_t r1, zdd::ptr_t r2)
    {
      // Are they both a terminal? If so, check whether they both are true, which
      // verify there is a satisfiying conjunction (i.e. representing a shared
      // path/element).
      if (r1.is_terminal() && r2.is_terminal()) {
        return r1.value() && r2.value();
      }

      // Has the left-hand side fallen out of its set?
      if (r1.is_false()) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (r2.is_false()) {
        return false;
      }

      // Otherwise, recurse
      pq.push({ {r1,r2}, {} });
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

    return internal::comparison_check<zdd_disjoint_policy>(s1, s2);
  }
}
