#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/pred.h>
#include <adiar/internal/product_construction.h>

namespace adiar {
  bool zdd_equal(const zdd &s1, const zdd &s2)
  {
    return is_isomorphic(s2, s1);
  }

  //////////////////////////////////////////////////////////////////////////////
  class ignore_levels
  {
  public:
    static size_t __comparison_size_based_upper_bound
      (const node_file &in_1, const node_file &in_2)
    {
      // Can the size_bound computation overflow?
      const bits_approximation in_1_bits(in_1.size());
      const bits_approximation in_2_bits(in_2.size());

      const bits_approximation bound_bits = in_1_bits * in_2_bits;

      if(bound_bits.may_overflow()) {
        return std::numeric_limits<size_t>::max();
      } else {
        return in_1.size() * in_2.size();
      }
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
    typedef ignore_levels level_check_t;

  public:
    static bool resolve_sinks(const node_t &v1, const node_t &v2, bool &ret_value)
    {
      if (is_sink(v1) && is_sink(v2)) {
        ret_value = !value_of(v1) || value_of(v2);
        return true;
      } if (is_false(v1)) {
        ret_value = true;
        return true;
      } else if (is_true(v2)) {
        ret_value = false;
        return true;
      }
      return false;
    }

  public:
    static bool resolve_singletons(const node_t &v1, const node_t &v2)
    {
      return label_of(v1) == label_of(v2)
        && v1.low <= v2.low && v1.high <= v2.high;
    }

  public:
    template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, ptr_t r1, ptr_t r2)
    {
      // Are they both a sink? If so, check whether the left-hand side is true
      // and not the right, which would contradict being an implication (i.e.
      // representing a subset).
      if (is_sink(r1) && is_sink(r2)) {
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
    typedef ignore_levels level_check_t;

  public:
    static bool resolve_sinks(const node_t &v1, const node_t &v2, bool &ret_value)
    {
      ret_value = is_false(v1) || is_false(v2);
      return (is_sink(v1) && is_sink(v2)) || ret_value;
    }

  public:
    static bool resolve_singletons(const node_t &v1, const node_t &v2)
    {
      return label_of(v1) != label_of(v2)
        || v1.low != v2.low
        || v1.high != v2.high;
    }

  public:
  template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, ptr_t r1, ptr_t r2)
    {
      // Are they both a sink? If so, check whether they both are true, which
      // verify there is a satisfiying conjunction (i.e. representing a shared
      // path/element).
      if (is_sink(r1) && is_sink(r2)) {
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
