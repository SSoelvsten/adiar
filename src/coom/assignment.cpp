#ifndef COOM_ASSIGNMENT_CPP
#define COOM_ASSIGNMENT_CPP

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include "data.h"
#include "pred.h"

#include "assert.h"

#include "debug.h"
#include "debug_data.h"
#include "debug_assignment.h"

namespace coom
{
  inline assignment create_assignment(uint64_t label, bool value)
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

  template<typename pred_t = std::less<assignment>>
  void get_assignment(tpie::file_stream<node> &in_nodes,
                      const sink_pred& sink_pred,
                      tpie::file_stream<assignment> &out_assignment,
                      const pred_t pred)
  {
    debug::println_algorithm_start("GET ASSIGNMENT");

    assert::is_valid_input_stream(in_nodes);
    debug::println_file_stream(in_nodes, "in_nodes");

    assert::is_valid_output_stream(out_assignment);

    in_nodes.seek(0);

    if (!is_sink_node(in_nodes.peek())) {
      node prior_node = in_nodes.read();

      // Since this is the deepest node in the topological order, it has to only
      // have sinks as children. Assuming it is reduced, we also know they
      // cannot be the same sink.

      uint64_t label = label_of(prior_node);
      bool value = sink_pred(prior_node.high);

      out_assignment.write({label, value});

      while (in_nodes.can_read()) {
        // Find the first node, that has an outgoing arc to the prior node
        while (in_nodes.peek().low != prior_node.node_ptr &&
               in_nodes.peek().high != prior_node.node_ptr) {
          in_nodes.read();
        }

        node parent_node = in_nodes.read();

        uint64_t label = label_of(parent_node);
        bool value = parent_node.high == prior_node.node_ptr;

        out_assignment.write({label, value});
        prior_node = parent_node;
      }
    }

    tpie::progress_indicator_null pi;
    tpie::sort(out_assignment, pred, pi);

    debug::println_file_stream(out_assignment, "out_assignment");
    debug::println_algorithm_end("GET ASSIGNMENT");
  }
}

#endif // COOM_ASSIGNMENT_CPP
