#ifndef COOM_COUNT_CPP
#define COOM_COUNT_CPP

#include "count.h"

#include "priority_queue.cpp"

#include "assert.h"

namespace coom
{
  struct partial_sum
  {
    uid_t uid;
    uint64_t sum;
  };

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

  inline uint64_t count(tpie::file_stream<node_t> &nodes,
                        tpie::file_stream<meta_t> &meta,
                        const sink_pred &sink_pred,
                        const bool count_skipped_layers)
  {
    assert::is_valid_input_stream(nodes);

    nodes.seek(0);
    if (is_sink(nodes.peek())) {
      return 0u;
    }

    label_t biggest_label = label_of(nodes.read());

    nodes.seek(0, tpie::file_stream_base::end);

    priority_queue<partial_sum, count_queue_label, count_queue_lt, 1> partial_sums;
    partial_sums.hook_meta_stream(meta);

    // Take root out and put its children into the priority queue
    // or count them immediately if they are sinks
    node_t root = nodes.read_back();
    uint64_t result = 0u;

    uint64_t sum = 0u;
    if (is_sink_ptr(root.low)) {
      if (sink_pred(root.low)) {
        sum = count_skipped_layers
          ? 1u << (biggest_label - label_of(root))
          : 1u;

        result = result + sum;
      }
    } else {
      sum = count_skipped_layers
        ? 1u << (label_of(root.low) - label_of(root) - 1)
        : 1u;

      partial_sums.push({root.low, sum});
    }

    sum = 0u;
    if (is_sink_ptr(root.high)) {
      if (sink_pred(root.high)) {
        sum = count_skipped_layers
          ? 1u << (biggest_label - label_of(root))
          : 1u;

        result = result + sum;
      }
    } else {
      sum = count_skipped_layers
        ? 1u << (label_of(root.high) - label_of(root) - 1)
        : 1u;

      partial_sums.push({root.high, sum});
    }

    // Take out the rest of the nodes and process them one by one
    while (nodes.can_read_back()) {
      node_t current_node = nodes.read_back();
      sum = 0u;

      if (partial_sums.current_layer() != label_of(current_node)) {
        partial_sums.setup_next_layer();
      }

      // Sum all ingoing arcs
      while (partial_sums.can_pull() && partial_sums.top().uid == current_node.uid) {
        sum += partial_sums.pull().sum;
      }

      // Put children of the current node into the priority queue or count them if they are sinks
      uint64_t sum_low = 0u;
      if (is_sink_ptr(current_node.low)) {
        if (sink_pred(current_node.low)) {
          sum_low = sum * (count_skipped_layers
                           ? 1u << (biggest_label - label_of(current_node))
                           : 1u);

          result += sum_low;
        }
      } else {
        sum_low = sum * (count_skipped_layers
                         ? 1u << (label_of(current_node.low) - label_of(current_node) - 1u)
                         : 1u);

        partial_sums.push({current_node.low, sum_low});
      }

      uint64_t sum_high = 0u;
      if (is_sink_ptr(current_node.high)) {
        if (sink_pred(current_node.high)) {
          sum_high = sum * (count_skipped_layers
                            ? 1u << (biggest_label - label_of(current_node))
                            : 1u);

          result += sum_high;
        }
      } else {
        sum_high = sum * (count_skipped_layers
                          ? 1u << (label_of(current_node.high) - label_of(current_node) - 1u)
                          : 1u);

        partial_sums.push({current_node.high, sum_high});
      }
    }

    return result;
  }

  uint64_t count_paths(tpie::file_stream<node_t> &nodes,
                       tpie::file_stream<meta_t> &meta,
                       const sink_pred &sink_pred)
  {
    return count(nodes, meta, sink_pred, false);
  }

  uint64_t count_assignments(tpie::file_stream<node_t>& nodes,
                             tpie::file_stream<meta_t> &meta,
                             const sink_pred& sink_pred)
  {
    return count(nodes, meta, sink_pred, true);
  }
}

#endif // COOM_COUNT_CPP
