#include "count.h"

#include <adiar/data.h>
#include <adiar/internal/count.h>

#include <adiar/assert.h>

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
    static void count_resolve_request(count_priority_queue_t<sat_sum> &count_pq,
                                      uint64_t &result, label_t varcount,
                                      ptr_t child_to_resolve, sat_sum request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      adiar_debug(request.levels_visited < varcount,
                  "Cannot have already visited more levels than are expected");

      label_t levels_visited = request.levels_visited + 1u;

      if (is_sink(child_to_resolve)) {
        result += value_of(child_to_resolve)
          ? request.sum * (1u << (varcount - levels_visited))
          : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum, levels_visited });
      }
    }

    static void count_resolve_requests(count_priority_queue_t<sat_sum> &count_pq,
                                       uint64_t &result, label_t varcount,
                                       const node_t& n)
    {
      // Sum all ingoing arcs with the same number of visited levels
      sat_sum request = count_pq.pull();

      while (count_pq.can_pull() && count_pq.top().uid == n.uid) {
        sat_sum r = count_pq.pull();

        request.sum = request.sum * (1u << (r.levels_visited - request.levels_visited));
        request.sum += r.sum;

        request.levels_visited = r.levels_visited;
      }

      // Resolve final request
      count_resolve_request(count_pq, result, varcount, n.low, request);
      count_resolve_request(count_pq, result, varcount, n.high, request);
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
    return is_sink(bdd)
      ? 0
      : count<path_count_policy, path_sum>(bdd, bdd_varcount(bdd));
  }

  uint64_t bdd_satcount(const bdd& bdd, label_t varcount)
  {
    if (is_sink(bdd)) {
      return is_sink(bdd, is_true) ? 1u << varcount : 0u;
    }

    adiar_assert(bdd_varcount(bdd) <= varcount,
                 "given minimum_label should be smaller than the present root label");

    return count<sat_count_policy, sat_sum>(bdd, varcount);
  }

  uint64_t bdd_satcount(const bdd& bdd)
  {
    return is_sink(bdd)
      ? 0u
      : count<sat_count_policy, sat_sum>(bdd, bdd_varcount(bdd));
  }
}
