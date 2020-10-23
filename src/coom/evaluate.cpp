#ifndef COOM_EVALUATE_CPP
#define COOM_EVALUATE_CPP

#include "evaluate.h"

#include <coom/assert.h>

namespace coom
{
  bool evaluate(tpie::file_stream<node_t> &nodes,
                tpie::file_stream<bool> &assignment)
  {
    assert::is_valid_input_stream(nodes);

    nodes.seek(0, tpie::file_stream_base::end);
    node_t current_node = nodes.read_back();

    assignment.seek(0);
    bool assignment_value = assignment.read();
    label_t assignment_label = 0;

    if(is_sink(current_node)) {
      return value_of(current_node);
    }

    while (true) {
      while(label_of(current_node) > assignment_label) {
        assignment_value = assignment.read();
        assignment_label++;
      }

      ptr_t next_ptr = unflag(assignment_value ? current_node.high : current_node.low);

      if(is_sink_ptr(next_ptr)) {
        return value_of(next_ptr);
      }

      while(current_node.uid < next_ptr) {
        current_node = nodes.read_back();
      }
    }
  }
}

#endif // COOM_EVALUATE_CPP
