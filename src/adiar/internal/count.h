/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

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
    inline static uint64_t forward_request(count_priority_queue_t<path_sum> &count_pq,
                                           const label_t /* varcount */,
                                           const ptr_t child_to_resolve,
                                           const path_sum &request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      if (is_sink(child_to_resolve)) {
        return value_of(child_to_resolve) ? request.sum : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum });
        return 0u;
      }
    }

    inline static path_sum combine_requests(const path_sum &acc, const path_sum &next)
    {
      adiar_debug(acc.uid == next.uid,
                  "Requests should be for the same node");

      return { acc.uid, acc.sum + next.sum };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template<typename count_policy, typename queue_t>
  uint64_t count(const decision_diagram &dd, label_t varcount)
  {
    adiar_debug(!is_sink(dd),
                "Count algorithm does not work on sink-only edge case");

    uint64_t result = 0u;

    node_stream<> ns(dd);

    count_priority_queue_t<queue_t> count_pq({dd});

    {
      node_t root = ns.pull();
      queue_t request = { root.uid, 1u };

      result += count_policy::forward_request(count_pq, varcount, root.low, request);
      result += count_policy::forward_request(count_pq, varcount, root.high, request);
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
      queue_t request = count_pq.pull();

      while (count_pq.can_pull() && count_pq.top().uid == n.uid) {
        request = count_policy::combine_requests(request, count_pq.pull());
      }

      result += count_policy::forward_request(count_pq, varcount, n.low, request);
      result += count_policy::forward_request(count_pq, varcount, n.high, request);
    }

    return result;
  }
}

#endif // ADIAR_INTERNAL_COUNT_H
