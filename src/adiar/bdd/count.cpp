/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

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
    inline static uint64_t forward_request(count_priority_queue_t<sat_sum> &count_pq,
                                           const label_t varcount,
                                           const ptr_t child_to_resolve,
                                           const sat_sum &request)
    {
      adiar_debug(request.sum > 0, "No 'empty' request should be created");

      adiar_debug(request.levels_visited < varcount,
                  "Cannot have already visited more levels than are expected");

      label_t levels_visited = request.levels_visited + 1u;

      if (is_sink(child_to_resolve)) {
        return value_of(child_to_resolve)
          ? request.sum * (1u << (varcount - levels_visited))
          : 0u;
      } else {
        count_pq.push({ child_to_resolve, request.sum, levels_visited });
        return 0u;
      }
    }

    inline static sat_sum combine_requests(const sat_sum &acc, const sat_sum &next)
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
