#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/prod2.h>

namespace adiar
{
  bool zdd_iscanonical(const zdd& A)
  {
    return internal::dd_iscanonical(A);
  }

  bool zdd_isterminal(const zdd& A)
  {
    return internal::dd_isterminal(A);
  }

  bool zdd_isfalse(const zdd& A)
  {
    return internal::dd_isfalse(A);
  }

  bool zdd_isempty(const zdd &A)
  {
    return zdd_isfalse(A);
  }

  bool zdd_istrue(const zdd& A)
  {
    return internal::dd_istrue(A);
  }

  bool zdd_isnull(const zdd &A)
  {
    return zdd_istrue(A);
  }

  bool zdd_equal(const zdd &s1, const zdd &s2)
  {
    return internal::is_isomorphic(s1, s2);
  }

  bool zdd_unequal(const zdd &A, const zdd &B)
  {
    return !zdd_equal(A,B);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<internal::cut::type ct_1, internal::cut::type ct_2>
  class ignore_levels
  {
  public:
    static size_t pq1_upper_bound(const internal::shared_levelized_file<zdd::node_type> &in_1,
                                  const internal::shared_levelized_file<zdd::node_type> &in_2)
    {
      const internal::safe_size_t max_2level_cut_1 = in_1->max_2level_cut[ct_1];
      const internal::safe_size_t max_2level_cut_2 = in_2->max_2level_cut[ct_2];

      return internal::to_size(max_2level_cut_1 * max_2level_cut_2);
    }

    static size_t pq2_upper_bound(const internal::shared_levelized_file<zdd::node_type> &in_1,
                                  const internal::shared_levelized_file<zdd::node_type> &in_2)
    {
      const internal::safe_size_t max_1level_cut_1 = in_1->max_1level_cut[ct_1];
      const internal::safe_size_t max_1level_cut_2 = in_2->max_1level_cut[ct_2];

      return internal::to_size(max_1level_cut_1 * max_1level_cut_2);
    }

    static constexpr size_t memory_usage()
    {
      return 0u;
    }

  public:
    ignore_levels(const internal::shared_levelized_file<zdd::node_type> &/*f1*/,
                  const internal::shared_levelized_file<zdd::node_type> &/*f2*/)
    { /* do nothing */ }

    void next_level(zdd::label_type /* level */)
    { /* do nothing */ }

    bool on_step()
    { return false; }

    static constexpr bool termination_value = false;
  };

  class zdd_subseteq_policy
    : public zdd_policy
    , public internal::prod2_mixed_level_merger<zdd_policy>
  {
  public:
    using level_check_t = ignore_levels<internal::cut::Internal_True, internal::cut::Internal_True>;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 1;
    }

  public:
    static bool resolve_terminals(const zdd::node_type &v1, const zdd::node_type &v2, bool &ret_value)
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
    static bool resolve_singletons(const zdd::node_type &v1, const zdd::node_type &v2)
    {
      return v1.label() == v2.label()
        && v1.low() <= v2.low() && v1.high() <= v2.high();
    }

  public:
    template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, const internal::tuple<zdd::pointer_type> &rp)
    {
      // Are they both a terminal? If so, check whether the left-hand side is true
      // and not the right, which would contradict being an implication (i.e.
      // representing a subset).
      if (rp[0].is_terminal() && rp[1].is_terminal()) {
        return essential(rp[0]) > essential(rp[1]);
      }

      // Has the left-hand side fallen out of its set?
      if (rp[0].is_false()) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (rp[1].is_false()) {
        return true;
      }

      // Otherwise, recurse
      pq.push({ rp, {} });
      return false;
    }

  public:
    static constexpr bool early_return_value = false;
    static constexpr bool no_early_return_value = true;
  };

  bool zdd_subseteq(const zdd &s1, const zdd &s2)
  {
    if (s1.file == s2.file) {
      return true;
    }

    return internal::comparison_check<zdd_subseteq_policy>(s1, s2);
  }

  //////////////////////////////////////////////////////////////////////////////
  bool zdd_subset(const zdd &A, const zdd &B)
  {
    return zdd_subseteq(A,B) && zdd_unequal(A,B);
  }

  //////////////////////////////////////////////////////////////////////////////
  class zdd_disjoint_policy
    : public zdd_policy
    , public internal::prod2_mixed_level_merger<zdd_policy>
  {
  public:
    using level_check_t = ignore_levels<internal::cut::All, internal::cut::All>;

  public:
    static constexpr size_t lookahead_bound()
    {
      return 2;
    }

  public:
    static bool resolve_terminals(const zdd::node_type &v1, const zdd::node_type &v2, bool &ret_value)
    {
      ret_value = v1.is_false() || v2.is_false();
      return (v1.is_terminal() && v2.is_terminal()) || ret_value;
    }

  public:
    static bool resolve_singletons(const zdd::node_type &v1, const zdd::node_type &v2)
    {
      return v1.label() != v2.label()
        || v1.low() != v2.low()
        || v1.high() != v2.high();
    }

  public:
    template<typename pq_1_t>
    static bool resolve_request(pq_1_t &pq, const internal::tuple<zdd::pointer_type> &rp)
    {
      // Are they both a terminal? If so, check whether they both are true, which
      // verify there is a satisfiying conjunction (i.e. representing a shared
      // path/element).
      if (rp[0].is_terminal() && rp[1].is_terminal()) {
        return rp[0].value() && rp[1].value();
      }

      // Has the left-hand side fallen out of its set?
      if (rp[0].is_false()) {
        return false;
      }

      // Has the right-hand side fallen out of its set?
      if (rp[0].is_false()) {
        return false;
      }

      // Otherwise, recurse
      pq.push({ rp, {} });
      return false;
    }

  public:
    static constexpr bool early_return_value = false;
    static constexpr bool no_early_return_value = true;
  };

  bool zdd_disjoint(const zdd &s1, const zdd &s2)
  {
    if (s1.file == s2.file) {
      return zdd_isfalse(s1);
    }

    return internal::comparison_check<zdd_disjoint_policy>(s1, s2);
  }
}
