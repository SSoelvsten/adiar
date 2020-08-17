#ifndef COOM_EVALUATE_CPP
#define COOM_EVALUATE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "evaluate.h"

#include "assert.h"
#include "debug.h"

namespace coom
{
  namespace debug
  {
    inline void print_evaluate_assignment([[maybe_unused]] const uint64_t label,
                                          [[maybe_unused]] bool value)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "| " << label << " := " << value << " ==> ";
#endif
    }

    inline void println_evaluate_return([[maybe_unused]] const uint64_t sink)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "return [ " << value_of(sink) << " ]" << std::endl;
#endif
    }

    inline void println_evaluate_seek([[maybe_unused]] const uint64_t node_ptr)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "seek   [ "
                       << label_of(node_ptr) << ", " << id_of(node_ptr)
                       << " ]" << std::endl;
#endif
    }
  }

  bool evaluate(tpie::file_stream<node> &nodes,
                tpie::file_stream<bool> &assignment)
  {
    debug::println_algorithm_start("EVALUATE");

    assert::is_valid_input_stream(nodes);
    debug::println_file_stream(nodes, "nodes");

    nodes.seek(0, tpie::file_stream_base::end);
    node current_node = nodes.read_back();

    assignment.seek(0);
    bool assignment_value = assignment.read();
    uint64_t assignment_label = 0;

    if(is_sink_node(current_node)) {
      debug::println_evaluate_return(current_node.node_ptr);
      debug::println_algorithm_end("EVALUATE");

      return value_of(current_node);
    }

    while (true) {
      while(label_of(current_node) > assignment_label) {
        assignment_value = assignment.read();
        assignment_label++;
      }

      debug::print_evaluate_assignment(assignment_label, assignment_value);

      uint64_t next_node_ptr = assignment_value ? current_node.high : current_node.low;

      if(is_sink(next_node_ptr)) {
        debug::println_evaluate_return(next_node_ptr);
        debug::println_algorithm_end("EVALUATE");

        return value_of(next_node_ptr);
      }

      debug::println_evaluate_seek(next_node_ptr);
      while(current_node.node_ptr < next_node_ptr) {
        current_node = nodes.read_back();
      }
    }
  }
}

#endif // COOM_EVALUATE_CPP
