#include <stdint.h>

#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/domain.h>
#include <adiar/exception.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/algorithms/count.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct sat_sum : internal::path_sum
  {
    bdd::label_type levels_visited = 0u;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<>
  struct internal::count_queue_lt<sat_sum>
  {
    bool operator()(const sat_sum &a, const sat_sum &b)
    {
      return a.target < b.target || (a.target == b.target && a.levels_visited < b.levels_visited);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  class sat_count_policy : public bdd_policy
  {
  public:
    using queue_t = sat_sum;

    template<typename count_pq_t>
    inline static uint64_t forward_request(count_pq_t &count_pq,
                                           const bdd::label_type varcount,
                                           const bdd::pointer_type child_to_resolve,
                                           const queue_t &request)
    {
      adiar_assert(request.sum > 0, "No 'empty' request should be created");

      adiar_assert(request.levels_visited < varcount,
                   "Cannot have already visited more levels than are expected");

      bdd::label_type levels_visited = request.levels_visited + 1u;

      if (child_to_resolve.is_terminal()) {
        return child_to_resolve.value()
          ? request.sum * (1u << (varcount - levels_visited))
          : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum, levels_visited });
        return 0u;
      }
    }

    inline static queue_t combine_requests(const queue_t &acc, const queue_t &next)
    {
      adiar_assert(acc.target == next.target,
                   "Requests should be for the same node");

      adiar_assert(acc.levels_visited <= next.levels_visited,
                   "Requests should be ordered on the number of levels visited");

      return {
        acc.target,
        acc.sum * (1u << (next.levels_visited - acc.levels_visited)) + next.sum,
        next.levels_visited
      };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &f)
  {
    return internal::dd_nodecount(f);
  }

  bdd::label_type bdd_varcount(const bdd &f)
  {
    return internal::dd_varcount(f);
  }

  uint64_t bdd_pathcount(const exec_policy &ep, const bdd &f)
  {
    return bdd_isterminal(f)
      ? 0
      : internal::count<internal::path_count_policy<bdd_policy>>(ep, f, bdd_varcount(f));
  }

  uint64_t bdd_pathcount(const bdd &f)
  { return bdd_pathcount(exec_policy(), f); }

  uint64_t bdd_satcount(const exec_policy &ep, const bdd& f, bdd::label_type varcount)
  {
    if (varcount < bdd_varcount(f)) {
      throw invalid_argument("'varcount' ought to be at least the number of levels in the BDD");
    }

    if (bdd_isterminal(f)) {
      return dd_valueof(f) ? std::min(1u, varcount) << varcount : 0u;
    }

    return internal::count<sat_count_policy>(ep, f, varcount);
  }

  uint64_t bdd_satcount(const bdd& f, bdd::label_type varcount)
  { return bdd_satcount(exec_policy(), f, varcount); }

  uint64_t bdd_satcount(const exec_policy &ep, const bdd &f)
  {
    const bdd::label_type domain_size = domain_isset() ? domain_get()->size() : 0;
    const bdd::label_type varcount = bdd_varcount(f);
    return bdd_satcount(ep, f, std::max(domain_size, varcount));
  };

  uint64_t bdd_satcount(const bdd &f)
  { return bdd_satcount(exec_policy(), f); };
}
