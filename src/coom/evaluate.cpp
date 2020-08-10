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
    inline void print_evaluate_assignment(const uint64_t label, bool value)
    {
#if COOM_DEBUG
      tpie::log_info() << "| " << label << " := " << value << " ==> ";
#endif
    }

    inline void println_evaluate_return(const uint64_t sink)
    {
#if COOM_DEBUG
      tpie::log_info() << "return [ " << value_of(sink) << " ]" << std::endl;
#endif
    }

    inline void println_evaluate_seek(const uint64_t node_ptr)
    {
#if COOM_DEBUG
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

    //reset streams
    nodes.seek(0, tpie::file_stream_base::end);
    assignment.seek(0);

    //få fat i roden
    node v = nodes.read_back();
    bool x = assignment.read();
    uint64_t label = 0;

    //check om træet kun er en sink
    if(is_sink_node(v)) {
      debug::println_evaluate_return(v.node_ptr);
      debug::println_algorithm_end("EVALUATE");

      return value_of(v);
    }

    //gennemløb af træet
    while (true) {
      //find det rigtige assignment
      while(label_of(v) > label) {
        x = assignment.read();
        label = label + 1;
      }

      debug::print_evaluate_assignment(label, x);

      //check værdien for variablen
      if(x) {

        //find high for noden og gem
        uint64_t high = v.high;

        //check om high er en sink og returner
        if(is_sink(high)) {
          debug::println_evaluate_return(high);
          debug::println_algorithm_end("EVALUATE");

          return value_of(high);
        }

        debug::println_evaluate_seek(high);

        //søg efter high
        while(v.node_ptr < high) {
            v = nodes.read_back();
        }
      }

      else {

        //som high, bare low - find low for noden og gem
        uint64_t low = v.low;

        //check om low er en sink og returner
        if(is_sink(low)) {
          debug::println_evaluate_return(low);
          debug::println_algorithm_end("EVALUATE");

          return value_of(low);
        }

        debug::println_evaluate_seek(low);

        //søg efter low
        while(v.node_ptr < low) {
            v = nodes.read_back();
        }
      }
    }
  }
}

#endif // COOM_EVALUATE_CPP
