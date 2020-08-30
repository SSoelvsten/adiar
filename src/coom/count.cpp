#ifndef COOM_COUNT_CPP
#define COOM_COUNT_CPP

#include <tpie/file_stream.h>
#include <tpie/priority_queue.h>

#include "data.h"

#include "debug.h"
#include "debug_data.h"

#include "assert.h"

#include "count.h"

namespace coom
{
  namespace debug
  {
    inline void println_count_low_sum([[maybe_unused]] const uid_t uid,
                                      [[maybe_unused]] const uint64_t count)
    {
#if COOM_DEBUG >= 2
      debug::print_node_ptr(uid);
      tpie::log_info() << " | low : " << count;
#endif
    }

    inline void println_count_high_sum([[maybe_unused]] const uid_t uid,
                                       [[maybe_unused]] const uint64_t count)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << " | high : " << count << std::endl;
#endif
    }

    inline void println_count_result([[maybe_unused]] const uint64_t count)
    {
#if COOM_DEBUG
      tpie::log_info() << std::endl << "total : " << count << std::endl;
#endif
    }

  }

  struct partial_sum
  {
    uint64_t uid;
    uint64_t sum;
  };

  struct count_queue_lt
  {
    bool operator()(const partial_sum &a, const partial_sum &b)
    {
      return a.uid < b.uid;
    }
  };

  inline uint64_t count(tpie::file_stream<node> &nodes,
                        const sink_pred &sink_pred,
                        const bool count_skipped_layers)
  {
    debug::println_algorithm_start("COUNT");

    assert::is_valid_input_stream(nodes);
    debug::println_file_stream(nodes, "nodes");

    nodes.seek(0);
    if (is_sink(nodes.peek())) {
      return 0u;
    }

    uint64_t biggest_label = label_of(nodes.read());

    nodes.seek(0, tpie::file_stream_base::end);
    tpie::priority_queue<partial_sum, count_queue_lt> partial_sums;

    // Take root out and put its children into the priority queue
    // or count them immediately if they are sinks
    node root = nodes.read_back();
    uint64_t result = 0u;

    uint64_t sum = 0u;
    if (is_sink_ptr(root.low)) {
      if (sink_pred(root.low)) {
        sum = count_skipped_layers
          ? 1 << (biggest_label - label_of(root))
          : 1;

        result = result + sum;
      }
    } else {
      sum = count_skipped_layers
        ?  1 << (label_of(root.low) - label_of(root) - 1)
        : 1;

      partial_sums.push({root.low, sum});
    }

    debug::println_count_low_sum(root.uid, sum);

    sum = 0u;
    if (is_sink_ptr(root.high)) {
      if (sink_pred(root.high)) {
        sum = count_skipped_layers
          ? 1 << (biggest_label - label_of(root))
          : 1;

        result = result + sum;
      }
    } else {
      sum = count_skipped_layers
        ? 1 << (label_of(root.high) - label_of(root) - 1)
        : 1;

      partial_sums.push({root.high, sum});
    }

    debug::println_count_high_sum(root.uid, sum);

    // Take out the rest of the nodes and process them one by one
    while (nodes.can_read_back()) {
      node current_node = nodes.read_back();
      sum = 0u;

      // Sum all ingoing arcs
      while (!partial_sums.empty() && partial_sums.top().uid == current_node.uid) {
        sum += partial_sums.top().sum;
        partial_sums.pop();
      }

      // Put children of the current node into the priority queue or count them if they are sinks
      uint64_t sum_low = 0;
      if (is_sink_ptr(current_node.low)) {
        if (sink_pred(current_node.low)) {
          sum_low = sum * (count_skipped_layers
                           ? 1 << (biggest_label - label_of(current_node))
                           : 1);

          result += sum_low;
        }
      } else {
        sum_low = sum * (count_skipped_layers
                         ? 1 << (label_of(current_node.low) - label_of(current_node) - 1)
                         : 1);

        partial_sums.push({current_node.low, sum_low});
      }
      debug::println_count_low_sum(current_node.uid, sum_low);

      uint64_t sum_high = 0u;
      if (is_sink_ptr(current_node.high)) {
        if (sink_pred(current_node.high)) {
          sum_high = sum * (count_skipped_layers
                            ? 1 << (biggest_label - label_of(current_node))
                            : 1);

          result += sum_high;
        }
      } else {
        sum_high = sum * (count_skipped_layers
                          ? 1 << (label_of(current_node.high) - label_of(current_node) - 1)
                          : 1);

        partial_sums.push({current_node.high, sum_high});
      }
      debug::println_count_high_sum(current_node.uid, sum_high);
    }

    debug::println_count_result(result);
    debug::println_algorithm_end("COUNT");
    return result;
  }

  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred = is_any)
  {
    return count(nodes, sink_pred, false);
  }

  uint64_t count_assignments(tpie::file_stream<node>& nodes,
                             const sink_pred& sink_pred = is_true)
  {
    return count(nodes, sink_pred, true);
  }
} // namespace coom

#endif // COOM_COUNT_CPP
