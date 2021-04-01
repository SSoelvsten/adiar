#ifndef ADIAR_COUNT_CPP
#define ADIAR_COUNT_CPP

#include "count.h"

#include <adiar/file_stream.h>
#include <adiar/priority_queue.h>
#include <adiar/reduce.h>

#include <adiar/assert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct path_sum
  {
    uid_t uid;
    uint64_t sum;
  };

  struct sat_sum : path_sum
  {
    label_t levels_visited = 0u;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<typename T>
  struct count_queue_lt;

  template<>
  struct count_queue_lt<path_sum>
  {
    bool operator()(const path_sum &a, const path_sum &b)
    {
      return a.uid < b.uid;
    }
  };

  template<>
  struct count_queue_lt<sat_sum>
  {
    bool operator()(const sat_sum &a, const sat_sum &b)
    {
      return a.uid < b.uid || (a.uid == b.uid && a.levels_visited < b.levels_visited);
    }
  };

  struct count_queue_label
  {
    label_t label_of(const path_sum &s)
    {
      return adiar::label_of(s.uid);
    }
  };

  template <typename T>
  using count_priority_queue_t = node_priority_queue<T, count_queue_label, count_queue_lt<T>>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename T>
  void count_resolve_request(count_priority_queue_t<T> &pq,
                             uint64_t &result, label_t varcount,
                             ptr_t child_to_resolve, T request);

  template<>
  void count_resolve_request<path_sum>(count_priority_queue_t<path_sum> &pq,
                                       uint64_t &result, label_t /* varcount */,
                                       ptr_t child_to_resolve, path_sum request)
  {
    adiar_debug(request.sum > 0, "No 'empty' request should be created");

    if (is_sink(child_to_resolve)) {
      result += value_of(child_to_resolve) ? request.sum : 0u;
    } else {
      pq.push({ child_to_resolve, request.sum });
    }
  }

  template<>
  void count_resolve_request<sat_sum>(count_priority_queue_t<sat_sum> &pq,
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
      pq.push({ child_to_resolve, request.sum, levels_visited });
    }
  }


  template<typename T>
  void count_resolve_requests(count_priority_queue_t<T> &pq,
                              uint64_t &result, label_t varcount,
                              const node_t& n);

  template<>
  void count_resolve_requests<path_sum>(count_priority_queue_t<path_sum> &pq,
                                        uint64_t &result, label_t varcount,
                                        const node_t& n)
  {
    // Sum all ingoing arcs
    path_sum request = pq.pull();

    while (pq.can_pull() && pq.top().uid == n.uid) {
      request.sum += pq.pull().sum;
    }

    // Resolve final request
    count_resolve_request<path_sum>(pq, result, varcount, n.low, request);
    count_resolve_request<path_sum>(pq, result, varcount, n.high, request);
  }

  template<>
  void count_resolve_requests<sat_sum>(count_priority_queue_t<sat_sum> &pq,
                                       uint64_t &result, label_t varcount,
                                       const node_t& n)
  {
    // Sum all ingoing arcs with the same number of visited levels
    sat_sum request = pq.pull();

    while (pq.can_pull() && pq.top().uid == n.uid) {
      sat_sum r = pq.pull();

      request.sum = request.sum * (1u << (r.levels_visited - request.levels_visited));
      request.sum += r.sum;

      request.levels_visited = r.levels_visited;
    }

    // Resolve final request
    count_resolve_request<sat_sum>(pq, result, varcount, n.low, request);
    count_resolve_request<sat_sum>(pq, result, varcount, n.high, request);
  }

  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &bdd)
  {
    return nodecount(bdd.file);
  }

  size_t bdd_varcount(const bdd &bdd)
  {
    return varcount(bdd.file);
  }

  template<typename T>
  inline uint64_t count(const bdd &bdd, label_t varcount)
  {
    adiar_debug(!is_sink(bdd),
                "Count algorithm does not work on sink-only edge case");

    uint64_t result = 0u;

    node_stream<> ns(bdd);

    count_priority_queue_t<T> partial_sums({bdd});

    {
      node_t root = ns.pull();
      T request = { root.uid, 1u };

      count_resolve_request<T>(partial_sums, result, varcount, root.low, request);
      count_resolve_request<T>(partial_sums, result, varcount, root.high, request);
    }

    // Take out the rest of the nodes and process them one by one
    while (ns.can_pull()) {
      node_t n = ns.pull();

      if (partial_sums.current_level() != label_of(n)) {
        partial_sums.setup_next_level();
      }
      adiar_debug(partial_sums.current_level() == label_of(n),
                  "Priority queue is out-of-sync with node stream");
      adiar_debug(partial_sums.can_pull() && partial_sums.top().uid == n.uid,
                  "Priority queue is out-of-sync with node stream");

      // Resolve requests
      count_resolve_requests<T>(partial_sums, result, varcount, n);
    }

    return result;
  }

  uint64_t bdd_pathcount(const bdd &bdd)
  {
    return is_sink(bdd)
      ? 0
      : count<path_sum>(bdd, bdd_varcount(bdd));
  }

  uint64_t bdd_satcount(const bdd& bdd, size_t varcount)
  {
    if (is_sink(bdd)) {
      return is_sink(bdd, is_true) ? 1u << varcount : 0u;
    }

    adiar_assert(bdd_varcount(bdd) <= varcount,
                 "given minimum_label should be smaller than the present root label");

    return count<sat_sum>(bdd, varcount);
  }

  uint64_t bdd_satcount(const bdd& bdd)
  {
    return is_sink(bdd)
      ? 0u
      : count<sat_sum>(bdd, bdd_varcount(bdd));
  }
}

#endif // ADIAR_COUNT_CPP
