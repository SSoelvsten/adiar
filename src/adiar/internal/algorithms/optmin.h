
#ifndef ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H
#define ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H

#include <stdint.h>

#include <adiar/exec_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/io/node_stream.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>

namespace adiar::internal
{
  //TODO!: Here be dragons
    
  //////////////////////////////////////////////////////////////////////////////
  //  Count Algorithm
  // =================
  //
  // Traverses a Decision Diagram and accumulates a numeric count along each of
  // its paths.
  //
  // Examples of uses are `bdd_pathcount` and `bdd_satcount`.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::optmin_t stats_optmin;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template<typename Data>
  using optmin_request = request_data<1, Data>;

  template <typename Data, size_t look_ahead, memory_mode mem_mode>
  using optmin_priority_queue_t =
    levelized_node_priority_queue<optmin_request<Data>,
                                  request_data_first_lt<optmin_request<Data>>,
                                  look_ahead,
                                  mem_mode,
                                  1u,
                                  0u>;

  //////////////////////////////////////////////////////////////////////////////
  // PathCount Policy

  template<typename Policy, typename PriorityQueue>
  inline uint64_t
  __optmin_resolve(PriorityQueue &optmin_pq,
                  const typename Policy::pointer_type &target,
                  const typename Policy::data_type &data,
                  const typename Policy::label_type varcount)
  {
    if (target.is_false()) {
      return Policy::resolve_false(data, varcount);
    }
    if (target.is_true()) {
      return Policy::resolve_true(data, varcount);
    }
    optmin_pq.push({ {target}, {}, data });
    return 0u;
  }

  template<typename Policy, typename PriorityQueue>
  uint64_t __optmin(const typename Policy::dd_type &dd,
                   const typename Policy::label_type varcount,
                   const size_t pq_max_memory,
                   const size_t pq_max_size)
  {
    adiar_assert(!dd->is_terminal(),
                 "Count Algorithm does not support terminal case");

    // Set up output
    uint64_t result = 0u;

    // Set up input
    node_stream<> ns(dd);

    // Set up cross-level priority queue with a request for the root
    PriorityQueue optmin_pq({dd}, pq_max_memory, pq_max_size, stats_optmin.lpq);
    {
      const node root = ns.peek();

      optmin_pq.push({ {root.uid()}, {}, Policy::init_data });
    }

    // Take out the rest of the nodes and process them one by one
    while (!optmin_pq.empty()) {
      optmin_pq.setup_next_level();

      while (!optmin_pq.empty_level()) {
        // Assuming there are no dead nodes, we should visit every node of dd
        const node n = ns.pull();

        adiar_assert(optmin_pq.top().target == n.uid(),
                     "Decision Diagram includes dead nodes");

        // Merge requests for 'n'
        const typename PriorityQueue::value_type request = optmin_pq.pull();
        typename Policy::data_type data = request.data;

        while (optmin_pq.can_pull() && optmin_pq.top().target == n.uid()) {
          data = Policy::merge(std::move(data), optmin_pq.pull().data);
        }

        data = Policy::merge_end(std::move(data));

        // Forward requests for children of 'n'
        result += __optmin_resolve<Policy>(optmin_pq, n.low(),  data, varcount);
        result += __optmin_resolve<Policy>(optmin_pq, n.high(), data, varcount);
      }
    }

    return result;
  }

  template<typename Policy>
  uint64_t optmin(const exec_policy &ep,
                 const typename Policy::dd_type &dd,
                 const typename Policy::label_type varcount)
  {
    adiar_assert(!dd_isterminal(dd),
                 "Count algorithm does not work on terminal-only edge case");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.

    const size_t aux_available_memory = memory_available() - node_stream<>::memory_usage();

    const size_t pq_memory_fits =
      optmin_priority_queue_t<typename Policy::data_type, ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(aux_available_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = dd.max_2level_cut(cut::Internal);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound()) {
#ifdef ADIAR_STATS
      stats_optmin.lpq.unbucketed += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<typename Policy::data_type,
                                                    0,
                                                    memory_mode::Internal>>
        (dd, varcount, aux_available_memory, max_pq_size);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_optmin.lpq.internal += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<typename Policy::data_type,
                                                    ADIAR_LPQ_LOOKAHEAD,
                                                    memory_mode::Internal>>
        (dd, varcount, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_optmin.lpq.external += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<typename Policy::data_type,
                                                    ADIAR_LPQ_LOOKAHEAD,
                                                    memory_mode::External>>
        (dd, varcount, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H
