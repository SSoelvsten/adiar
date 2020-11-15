#ifndef COOM_COUNT_CPP
#define COOM_COUNT_CPP

#include "count.h"

#include <coom/file_stream.h>
#include <coom/priority_queue.h>
#include <coom/reduce.h>

#include <coom/assert.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct partial_sum
  {
    uid_t uid;
    uint64_t sum;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct count_queue_lt
  {
    bool operator()(const partial_sum &a, const partial_sum &b)
    {
      return a.uid < b.uid;
    }
  };

  struct count_queue_label
  {
    label_t label_of(const partial_sum &s)
    {
      return coom::label_of(s.uid);
    }
  };

  typedef node_priority_queue<partial_sum, count_queue_label, count_queue_lt> count_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<bool count_skipped_layers>
  inline void count_resolve_request(count_priority_queue_t &pq, uint64_t &result, const sink_pred &sink_pred,
                                    ptr_t child_to_resolve, label_t current_label, label_t max_label,
                                    uint64_t ingoing_sum = 1u)
  {
#if COOM_ASSERT
    assert(ingoing_sum != 0);
#endif

    if (is_sink_ptr(child_to_resolve)) {
      if (sink_pred(child_to_resolve)) {
        result += ingoing_sum * (count_skipped_layers
                                 ? 1u << (max_label - current_label)
                                 : 1u);
      }
    } else {
      pq.push({ child_to_resolve,
                ingoing_sum * (count_skipped_layers
                               ? 1u << (label_of(child_to_resolve) - current_label - 1u)
                               : 1u) });
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_nodecount(const node_file &nodes)
  {
    return nodes.size();
  }

  uint64_t bdd_nodecount(const arc_file &arcs)
  {
    // Every node is represented by two arcs
    return arcs.size() / 2;
  }

  uint64_t bdd_nodecount(const node_or_arc_file &file)
  {
    return file.has<node_file>()
      ? bdd_nodecount(file.get<node_file>())
      : bdd_nodecount(file.get<arc_file>());
  }

  template<bool count_skipped_layers>
  inline uint64_t count(const node_file &nodes,
                        label_t min_label,
                        label_t max_label,
                        const sink_pred &sink_pred)
  {
    node_stream<> ns(nodes);

    count_priority_queue_t partial_sums;
    partial_sums.hook_meta_stream(nodes);

    // Take root out and put its children into the priority queue
    // or count them immediately if they are sinks
    node_t root = ns.pull();

    uint64_t result = count_skipped_layers && min_label < label_of(root)
      ? 1u << (label_of(root) - min_label)
      : 0u;

    count_resolve_request<count_skipped_layers>(partial_sums, result, sink_pred,
                                                root.low, label_of(root), max_label);
    count_resolve_request<count_skipped_layers>(partial_sums, result, sink_pred,
                                                root.high, label_of(root), max_label);

    // Take out the rest of the nodes and process them one by one
    while (ns.can_pull()) {
      node_t n = ns.pull();

      if (partial_sums.current_layer() != label_of(n)) {
        partial_sums.setup_next_layer();
      }

      // Sum all ingoing arcs
      uint64_t sum = 0u;
      while (partial_sums.can_pull() && partial_sums.top().uid == n.uid) {
        sum += partial_sums.pull().sum;
      }

      // Put children of the current node into the priority queue or count them if they are sinks
      count_resolve_request<count_skipped_layers>(partial_sums, result, sink_pred,
                                                  n.low, label_of(n), max_label, sum);
      count_resolve_request<count_skipped_layers>(partial_sums, result, sink_pred,
                                                  n.high, label_of(n), max_label, sum);
    }

    return result;
  }

  uint64_t bdd_pathcount(const node_file &nodes, const sink_pred &sink_pred)
  {
    if (is_sink(nodes)) {
      return 0u;
    }

    return count<false>(nodes, 0, MAX_LABEL, sink_pred);
  }

  uint64_t bdd_satcount(const node_file& nodes, const sink_pred& sink_pred)
  {
    if (is_sink(nodes)) {
      return 0u;
    }

    return count<true>(nodes, min_label(nodes), max_label(nodes), sink_pred);
  }

  uint64_t bdd_pathcount(const node_file &nodes,
                         label_t minimum_label,
                         label_t maximum_label,
                         const sink_pred& sink_pred)
  {
    if (is_sink(nodes)) {
      return 0u;
    }

    coom_assert(minimum_label <= min_label(nodes),
                "given minimum_label should be smaller than the present root label");

    coom_assert(max_label(nodes) <= maximum_label,
                "given maximum_label should be greater than the largest label in obdd");

    return count<true>(nodes, minimum_label, maximum_label, sink_pred);
  }
}

#endif // COOM_COUNT_CPP
