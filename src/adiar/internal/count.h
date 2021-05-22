#ifndef ADIAR_INTERNAL_COUNT_H
#define ADIAR_INTERNAL_COUNT_H

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/levelized_priority_queue.h>

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

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<typename T>
  struct count_queue_lt;

  struct count_queue_label
  {
    label_t label_of(const path_sum &s)
    {
      return adiar::label_of(s.uid);
    }
  };

  template<>
  struct count_queue_lt<path_sum>
  {
    bool operator()(const path_sum &a, const path_sum &b)
    {
      return a.uid < b.uid;
    }
  };

  template <typename T>
  using count_priority_queue_t = levelized_node_priority_queue<T, count_queue_label, count_queue_lt<T>>;

  //////////////////////////////////////////////////////////////////////////////
  // Variadic behaviour
  class path_count_policy
  {
  public:
    static void count_resolve_request(count_priority_queue_t<path_sum> &count_pq,
                                      uint64_t &result, label_t /* varcount */,
                                      ptr_t child_to_resolve, path_sum request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      if (is_sink(child_to_resolve)) {
        result += value_of(child_to_resolve) ? request.sum : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum });
      }
    }

    static void count_resolve_requests(count_priority_queue_t<path_sum> &count_pq,
                                       uint64_t &result, label_t varcount,
                                       const node_t& n)
    {
      // Sum all ingoing arcs
      path_sum request = count_pq.pull();

      while (count_pq.can_pull() && count_pq.top().uid == n.uid) {
        request.sum += count_pq.pull().sum;
      }

      // Resolve final request
      count_resolve_request(count_pq, result, varcount, n.low, request);
      count_resolve_request(count_pq, result, varcount, n.high, request);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<typename count_policy, typename queue_t, typename in_t>
  uint64_t count(const in_t &dd, label_t varcount)
  {
    adiar_debug(!is_sink(dd),
                "Count algorithm does not work on sink-only edge case");

    uint64_t result = 0u;

    node_stream<> ns(dd);

    count_priority_queue_t<queue_t> count_pq({dd});

    {
      node_t root = ns.pull();
      queue_t request = { root.uid, 1u };

      count_policy::count_resolve_request(count_pq, result, varcount, root.low, request);
      count_policy::count_resolve_request(count_pq, result, varcount, root.high, request);
    }

    // Take out the rest of the nodes and process them one by one
    while (ns.can_pull()) {
      node_t n = ns.pull();

      if (count_pq.current_level() != label_of(n)) {
        count_pq.setup_next_level();
      }
      adiar_debug(count_pq.current_level() == label_of(n),
                  "Priority queue is out-of-sync with node stream");
      adiar_debug(count_pq.can_pull() && count_pq.top().uid == n.uid,
                  "Priority queue is out-of-sync with node stream");

      // Resolve requests
      count_policy::count_resolve_requests(count_pq, result, varcount, n);
    }

    return result;
  }
}

#endif // ADIAR_INTERNAL_COUNT_H
