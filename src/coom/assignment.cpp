#ifndef COOM_ASSIGNMENT_CPP
#define COOM_ASSIGNMENT_CPP

#include "assignment.h"

#include <tpie/sort.h>

#include "assert.h"

#include "debug.h"
#include "debug_data.h"
#include "debug_assignment.h"

namespace coom
{
  inline assignment_t create_assignment(label_t label, bool value)
  {
#if COOM_ASSERT
    assert (label <= MAX_LABEL);
#endif
    return {label, value};
  }

  bool operator< (const assignment& a, const assignment& b)
  {
    return a.label < b.label || (a.label == b.label && a.value < b.value);
  }

  bool operator> (const assignment& a, const assignment& b)
  {
    return a.label > b.label || (a.label == b.label && a.value > b.value);
  }

  bool operator== (const assignment& a, const assignment& b)
  {
    return a.label == b.label && a.value == b.value;
  }

  bool operator!= (const assignment& a, const assignment& b)
  {
    return !(a==b);
  }

  template<typename pred_t = std::less<assignment_t>>
  bool get_assignment(tpie::file_stream<node_t> &in_nodes,
                      const sink_pred& sink_pred,
                      tpie::file_stream<assignment_t> &out_assignment,
                      const pred_t pred)
  {
    debug::println_algorithm_start("GET ASSIGNMENT");

    assert::is_valid_input_stream(in_nodes);
    debug::println_file_stream(in_nodes, "in_nodes");

    assert::is_valid_output_stream(out_assignment);

    in_nodes.seek(0);

    if (is_sink(in_nodes.peek())) {
      return true;
    }

    // Find the first node, that satisfies the predicate (if any)
    node_t prior_node;
    do {
      prior_node = in_nodes.read();
    } while (in_nodes.can_read() &&
             (!is_sink_ptr(prior_node.low) || !sink_pred(prior_node.low)) &&
             (!is_sink_ptr(prior_node.high) || !sink_pred(prior_node.high)));

    if ((!is_sink_ptr(prior_node.low) || !sink_pred(prior_node.low)) &&
        (!is_sink_ptr(prior_node.high) || !sink_pred(prior_node.high))) {
      return false;
    }

    label_t label = label_of(prior_node);
    bool value = is_sink_ptr(prior_node.high) && sink_pred(prior_node.high);

    out_assignment.write({ label, value });

    // Output first-seen nodes that can lead to this node
    while (in_nodes.can_read()) {
      node_t parent_node;
      do {
        parent_node = in_nodes.read();
      } while (parent_node.low != prior_node.uid && parent_node.high != prior_node.uid);

      label_t label = label_of(parent_node);
      bool value = parent_node.high == prior_node.uid;

      out_assignment.write({label, value});
      prior_node = parent_node;
    }

    tpie::progress_indicator_null pi;
    tpie::sort(out_assignment, pred, pi);

    debug::println_file_stream(out_assignment, "out_assignment");
    debug::println_algorithm_end("GET ASSIGNMENT");

    return true;
  }
}

#endif // COOM_ASSIGNMENT_CPP
