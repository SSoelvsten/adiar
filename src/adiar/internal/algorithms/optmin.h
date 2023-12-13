
#ifndef ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H
#define ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H

#include "adiar/functional.h"
#include "adiar/internal/data_types/node.h"
#include "adiar/internal/data_types/ptr.h"
#include "adiar/internal/io/arc_stream.h"
#include "adiar/types.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdint.h>

#include <adiar/exec_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/node_stream.h>
#include <tuple>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  OptMin Algorithm
  // ==================
  //
  // Traverses a Decision Diagram, finding the cheapest path to True given a
  // cost function for choosing true for each layer.
  //
  // TODO: Examples of uses
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::optmin_t stats_optmin;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct optmin_data
  {
    double cost;
    node::pointer_type source;
    static constexpr bool sort_on_tiebreak = false;
  };

  using optmin_request = request_data<1, optmin_data>;

  template<size_t look_ahead, memory_mode mem_mode>
  using optmin_priority_queue_t =
      levelized_node_priority_queue<optmin_request,
                                    request_data_first_lt<optmin_request>,
                                    look_ahead,
                                    mem_mode,
                                    1u,
                                    0u>;

  template<typename Policy, typename PriorityQueue>
  double
  __optmin(Policy policy,
           const typename Policy::dd_type& dd,
           const size_t pq_max_memory,
           const size_t pq_max_size)
  {
    adiar_assert(!dd->is_terminal(), "OptMin Algorithm does not support terminal case");

    // Set up temporary outputs
    shared_levelized_file<arc> best_parent_graph;

    double min_so_far = std::numeric_limits<double>::infinity();
    node::pointer_type min_so_far_end = node::pointer_type::nil();

    {
      arc_writer aw(best_parent_graph);

      // Set up input
      node_stream<> ns(dd);

      // Set up cross-level priority queue with a request for the root
      PriorityQueue optmin_pq({dd}, pq_max_memory, pq_max_size, stats_optmin.lpq);
      {
        const node root = ns.peek();

        optmin_pq.push({{root.uid()}, {}, {0.0, node::pointer_type::nil()}});
      }

      // Take out the rest of the nodes and process them one by one
      while (!optmin_pq.empty()) {
        optmin_pq.setup_next_level();

        typename Policy::label_type label = optmin_pq.current_level();
        double c = policy.cost_fn(label);

        while (!optmin_pq.empty_level()) {
          // Merge requests for the next target, finding the request with the
          // smallest cost to get to n
          optmin_request best = optmin_pq.pull();

          while (optmin_pq.can_pull() && optmin_pq.top().target == best.target) {
            const optmin_request next = optmin_pq.pull();
            if (next.data.cost < best.data.cost) {
              best = next;
            }
          }

          double cost_low = best.data.cost;
          double cost_high = best.data.cost + c;

          if constexpr (Policy::bullying) {
            if (cost_low >= min_so_far) {
              continue;
            }
          }

          const node n = ns.seek(best.target[0]);
          std::cout << "seek " << n.uid().label() << ", id: " << n.id() << std::endl;
          adiar_assert(n.uid() == best.target[0]);

          if (!best.data.source.is_nil()) {
            aw.push_internal({best.data.source, n.uid()});
          }

          if (n.low().is_terminal()) {
            if (n.low().is_true() && cost_low < min_so_far) {
              min_so_far = cost_low;
              min_so_far_end = n.uid().with(false);
            }
          } else {
            optmin_pq.push({{n.low()}, {}, {cost_low, n.uid().with(false)}});
          }

          if constexpr (Policy::bullying) {
            if (cost_high >= min_so_far) {
              continue;
            }
          }

          if (n.high().is_terminal()) {
            if (n.high().is_true() && cost_high < min_so_far) {
              min_so_far = cost_high;
              min_so_far_end = n.uid().with(true);
            }
          } else {
            optmin_pq.push({{n.high()}, {}, {cost_high, n.uid().with(true)}});
          }
        }
      }
    }
    adiar_assert(min_so_far_end.is_node(), "No result found");

    {
      arc_stream ns(best_parent_graph);
      arc next = {min_so_far_end, node::pointer_type(true)};
      policy.out(min_so_far_end.label(), min_so_far_end.out_idx());
      while (ns.can_pull_internal()) {
        arc n = ns.pull_internal();
        if (n.target() == essential(next.source())) {
          std::cout << n.target().label() << ", " << n.target().id() << std::endl;
          next = n;
          policy.out(next.source().label(), next.source().out_idx());
        }
      }
    }

    return min_so_far;
  }

  template<typename Policy>
  double
  optmin(const exec_policy& ep, Policy policy, const typename Policy::dd_type& dd)
  {
    adiar_assert(!dd_isterminal(dd), "Count algorithm does not work on terminal-only edge case");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.

    const size_t aux_available_memory = memory_available() - node_stream<>::memory_usage();

    const size_t pq_memory_fits =
        optmin_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
            aux_available_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = dd.max_2level_cut(cut::Internal);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound()) {
#ifdef ADIAR_STATS
      stats_optmin.lpq.unbucketed += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<0, memory_mode::Internal>>(
          policy, dd, aux_available_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_optmin.lpq.internal += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>(
          policy, dd, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_optmin.lpq.external += 1u;
#endif
      return __optmin<Policy, optmin_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>(
          policy, dd, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_OPT_MIN_H
