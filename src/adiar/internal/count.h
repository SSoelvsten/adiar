#ifndef ADIAR_INTERNAL_COUNT_H
#define ADIAR_INTERNAL_COUNT_H

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::count_t stats_count;

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
    inline static label_t label_of(const path_sum &s)
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

  template <typename elem_t,
            template<typename, typename> typename sorter_template = external_sorter,
            template<typename, typename> typename priority_queue_template = external_priority_queue>
  using count_priority_queue_t =
    levelized_node_priority_queue<elem_t, count_queue_label, count_queue_lt<elem_t>,
                                  sorter_template, priority_queue_template>;

  //////////////////////////////////////////////////////////////////////////////
  // Variadic behaviour
  class path_count_policy
  {
  public:
    typedef path_sum queue_t;

    template<typename count_pq_t>
    inline static uint64_t forward_request(count_pq_t &count_pq,
                                           const label_t /* varcount */,
                                           const ptr_t child_to_resolve,
                                           const queue_t &request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      if (is_sink(child_to_resolve)) {
        return value_of(child_to_resolve) ? request.sum : 0u;
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
  // Helper functions
  inline size_t __count_max_cut_upper_bound(const decision_diagram &dd)
  {
    const size_t input_size = dd.file_ptr()->size();
    const bits_approximation input_bits(input_size);

    const bits_approximation bound_bits = input_bits + 1;

    if (bound_bits.may_overflow()) {
      return std::numeric_limits<size_t>::max();
    } else {
      return input_size + 1;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename count_policy, typename count_pq_t>
  uint64_t __count(const decision_diagram &dd,
                   const label_t varcount,
                   const size_t pq_max_memory,
                   const size_t pq_max_size)
  {
    node_stream<> ns(dd);

    count_pq_t count_pq({dd}, pq_max_memory, pq_max_size);

    uint64_t result = 0u;

    {
      node_t root = ns.pull();
      typename count_policy::queue_t request = { root.uid, 1u };

      result += count_policy::forward_request(count_pq, varcount, root.low, request);
      result += count_policy::forward_request(count_pq, varcount, root.high, request);
    }

    // Take out the rest of the nodes and process them one by one
    while (ns.can_pull()) {
      node_t n = ns.pull();

      if (!count_pq.has_current_level() || count_pq.current_level() != label_of(n)) {
        count_pq.setup_next_level();
      }
      adiar_debug(count_pq.current_level() == label_of(n),
                  "Priority queue is out-of-sync with node stream");
      adiar_debug(count_pq.can_pull() && count_pq.top().uid == n.uid,
                  "Priority queue is out-of-sync with node stream");

      // Resolve requests
      typename count_policy::queue_t request = count_pq.pull();

      while (count_pq.can_pull() && count_pq.top().uid == n.uid) {
        request = count_policy::combine_requests(request, count_pq.pull());
      }

      result += count_policy::forward_request(count_pq, varcount, n.low, request);
      result += count_policy::forward_request(count_pq, varcount, n.high, request);
    }

    return result;
  }

  template<typename count_policy>
  uint64_t count(const decision_diagram &dd, const label_t varcount)
  {
    adiar_debug(!is_sink(dd),
                "Count algorithm does not work on sink-only edge case");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t max_pq_size = __count_max_cut_upper_bound(dd);

    const size_t aux_available_memory = memory::available() - node_stream<>::memory_usage();

    const size_t pq_memory_fits =
      count_priority_queue_t<typename count_policy::queue_t>::memory_fits(aux_available_memory);

    if (max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_count.lpq_internal++;
#endif
      return __count<count_policy, count_priority_queue_t<typename count_policy::queue_t,
                                                          internal_sorter,
                                                          internal_priority_queue>>
        (dd, varcount, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_count.lpq_external++;
#endif
      return __count<count_policy, count_priority_queue_t<typename count_policy::queue_t,
                                                          external_sorter,
                                                          external_priority_queue>>
        (dd, varcount, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_COUNT_H
