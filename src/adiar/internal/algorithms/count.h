#ifndef ADIAR_INTERNAL_ALGORITHMS_COUNT_H
#define ADIAR_INTERNAL_ALGORITHMS_COUNT_H

#include <stdint.h>

#include <adiar/label.h>
#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/util.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::count_t stats_count;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct path_sum
  {
    uid_t uid; // <-- TODO: rename to 'target'
    uint64_t sum;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<typename T>
  struct count_queue_lt;

  struct count_queue_label
  {
    inline static label_t label_of(const path_sum &s)
    {
      return s.uid.label();
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

  template <typename elem_t, size_t LOOK_AHEAD, memory::memory_mode mem_mode>
  using count_priority_queue_t =
    levelized_node_priority_queue<elem_t, count_queue_label,
                                  count_queue_lt<elem_t>, LOOK_AHEAD,
                                  mem_mode>;

  //////////////////////////////////////////////////////////////////////////////
  // Variadic behaviour
  class path_count_policy
  {
  public:
    typedef path_sum queue_t;

    template<typename count_pq_t>
    inline static uint64_t forward_request(count_pq_t &count_pq,
                                           const label_t /* varcount */,
                                           const ptr_uint64 child_to_resolve,
                                           const queue_t &request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      if (child_to_resolve.is_terminal()) {
        return child_to_resolve.value() ? request.sum : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum });
        return 0u;
      }
    }

    inline static queue_t combine_requests(const queue_t &acc, const queue_t &next)
    {
      adiar_debug(acc.uid == next.uid,
                  "Requests should be for the same node");

      return { acc.uid, acc.sum + next.sum };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<typename count_policy, typename count_pq_t>
  uint64_t __count(const decision_diagram &dd,
                   const label_t varcount,
                   const size_t pq_max_memory,
                   const size_t pq_max_size)
  {
    node_stream<> ns(dd);

    count_pq_t count_pq({dd}, pq_max_memory, pq_max_size, stats_count.lpq);

    uint64_t result = 0u;

    {
      node_t root = ns.pull();
      typename count_policy::queue_t request = { root.uid(), 1u };

      result += count_policy::forward_request(count_pq, varcount, root.low(), request);
      result += count_policy::forward_request(count_pq, varcount, root.high(), request);
    }

    // Take out the rest of the nodes and process them one by one
    while (ns.can_pull()) {
      node_t n = ns.pull();

      if (!count_pq.has_current_level() || count_pq.current_level() != n.label()) {
        count_pq.setup_next_level();
      }
      adiar_debug(count_pq.current_level() == n.label(),
                  "Priority queue is out-of-sync with node stream");
      adiar_debug(count_pq.can_pull() && count_pq.top().uid == n.uid(),
                  "Priority queue is out-of-sync with node stream");

      // Resolve requests
      typename count_policy::queue_t request = count_pq.pull();

      while (count_pq.can_pull() && count_pq.top().uid == n.uid()) {
        request = count_policy::combine_requests(request, count_pq.pull());
      }

      result += count_policy::forward_request(count_pq, varcount, n.low(), request);
      result += count_policy::forward_request(count_pq, varcount, n.high(), request);
    }

    return result;
  }

  template<typename count_policy>
  uint64_t count(const decision_diagram &dd, const label_t varcount)
  {
    adiar_debug(!is_terminal(dd),
                "Count algorithm does not work on terminal-only edge case");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.

    const size_t aux_available_memory = memory::available() - node_stream<>::memory_usage();

    const size_t pq_memory_fits =
      count_priority_queue_t<typename count_policy::queue_t, ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::memory_fits(aux_available_memory);

    const bool internal_only = memory::mode == memory::INTERNAL;
    const bool external_only = memory::mode == memory::EXTERNAL;

    const size_t pq_bound = dd.max_2level_cut(cut_type::INTERNAL);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound()) {
#ifdef ADIAR_STATS
      stats_count.lpq.unbucketed++;
#endif
      return __count<count_policy, count_priority_queue_t<typename count_policy::queue_t,
                                                          0,
                                                          memory::INTERNAL>>
        (dd, varcount, aux_available_memory, max_pq_size);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_count.lpq.internal++;
#endif
      return __count<count_policy, count_priority_queue_t<typename count_policy::queue_t,
                                                          ADIAR_LPQ_LOOKAHEAD,
                                                          memory::INTERNAL>>
        (dd, varcount, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_count.lpq.external++;
#endif
      return __count<count_policy, count_priority_queue_t<typename count_policy::queue_t,
                                                          ADIAR_LPQ_LOOKAHEAD,
                                                          memory::EXTERNAL>>
        (dd, varcount, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_COUNT_H
