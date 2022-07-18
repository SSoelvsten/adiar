#include <adiar/bdd.h>

#include <adiar/data.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/count.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct sat_sum : path_sum
  {
    label_t levels_visited = 0u;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<>
  struct count_queue_lt<sat_sum>
  {
    bool operator()(const sat_sum &a, const sat_sum &b)
    {
      return a.uid < b.uid || (a.uid == b.uid && a.levels_visited < b.levels_visited);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  class sat_count_policy
  {
  public:
    typedef sat_sum queue_t;

    template<typename count_pq_t>
    inline static uint64_t forward_request(count_pq_t &count_pq,
                                           const label_t varcount,
                                           const ptr_t child_to_resolve,
                                           const queue_t &request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      adiar_debug(request.levels_visited < varcount,
                  "Cannot have already visited more levels than are expected");

      label_t levels_visited = request.levels_visited + 1u;

      if (is_terminal(child_to_resolve)) {
        return value_of(child_to_resolve)
          ? request.sum * (1u << (varcount - levels_visited))
          : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum, levels_visited });
        return 0u;
      }
    }

    inline static queue_t combine_requests(const queue_t &acc, const queue_t &next)
    {
      adiar_debug(acc.uid == next.uid,
                  "Requests should be for the same node");

      adiar_debug(acc.levels_visited <= next.levels_visited,
                  "Requests should be ordered on the number of levels visited");

      return {
        acc.uid,
        acc.sum * (1u << (next.levels_visited - acc.levels_visited)) + next.sum,
        next.levels_visited
      };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &bdd)
  {
    return nodecount(bdd.file);
  }

  label_t bdd_varcount(const bdd &bdd)
  {
    return varcount(bdd.file);
  }

  uint64_t bdd_pathcount(const bdd &bdd)
  {
    return is_terminal(bdd)
      ? 0
      : count<path_count_policy>(bdd, bdd_varcount(bdd));
  }

  uint64_t bdd_satcount(const bdd& bdd, label_t varcount)
  {
    if (is_terminal(bdd)) {
      return value_of(bdd) ? std::min(1u, varcount) << varcount : 0u;
    }

    adiar_assert(bdd_varcount(bdd) <= varcount,
                 "number of variables in domain should be greater than the ones present in the BDD.");

    return count<sat_count_policy>(bdd, varcount);
  }
}
