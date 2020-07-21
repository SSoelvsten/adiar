#ifndef COOM_EVALUATE_CPP
#define COOM_EVALUATE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "evaluate.h"

namespace coom
{
  bool evaluate(tpie::file_stream<node> &nodes,
                tpie::file_stream<bool> &assignment)
  {
    //reset streams
    nodes.seek(0, tpie::file_stream_base::end);
    assignment.seek(0); 

    //få fat i roden
    node v = nodes.read_back();
    bool x = assignment.read();
    int label = label_of(v);

    //check om træet kun er en sink
    if(is_sink_node(v)) {
      return value_of(v);
    }

    //gennemløb af træet
    while (true) {

      //find det rigtige assignment
      while(label_of(v) > label) {
        x = assignment.read();
        label = label + 1;
      }

      //check værdien for variablen
      if(x) {

        //find high for noden og gem
        uint64_t high = v.high;

        //check om high er en sink og returner
        if(is_sink(high)) {
          return value_of(high);
        }
        
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
          return value_of(low);
        }
        
        //søg efter low
        while(v.node_ptr < low) {
            v = nodes.read_back();
        }
      }
    }
  }
}

#endif // COOM_EVALUATE_CPP
