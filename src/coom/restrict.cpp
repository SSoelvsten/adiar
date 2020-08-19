#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include <tpie/priority_queue.h>

#include "data.h"
#include "pred.h"
#include "reduce.h"

#include "util.cpp"

#include "assert.h"

#include "debug.h"
#include "debug_data.h"

#include "restrict.h"

namespace coom
{
  struct restrict_lt {
    bool operator ()(const arc& a, const arc& b) {
      return a.target < b.target || (a.target == b.target &&
                                     (is_nil(a.source) ||
                                      (!is_nil(b.source) && a.source < b.source) ||
                                      (a.source == b.source && !a.is_high)));
    }
  };

  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes,
                tpie::file_stream<arc> &reduce_node_arcs,
                tpie::file_stream<arc> &reduce_sink_arcs)
  {
    tpie::priority_queue<arc, restrict_lt> resD;

    in_nodes.seek(0, tpie::file_stream_base::end);
    in_assignment.seek(0);

    auto v = in_nodes.read_back();
    auto a = in_assignment.read();

    // Find the next assignment
    while(in_assignment.can_read() && label_of(v) > a.label) {
      a = in_assignment.read();
    }

    //Process the root and create initial recursion requests
    if(a.label == label_of(v)) {
      uint64_t rec_child = a.value ? v.high : v.low;

      if(is_sink(rec_child)) {
        out_nodes.write(create_sink_node(value_of(rec_child)));
        return;
      }

      resD.push(create_arc(NIL, a.value, rec_child));
    } else {
      arc low_arc = low_arc_of_node(v);
      if(is_sink(v.low)) {
        reduce_sink_arcs.write(low_arc);
      } else {
        resD.push(low_arc);
      }

      arc high_arc = high_arc_of_node(v);
      if(is_sink(v.high)) {
        reduce_sink_arcs.write(high_arc);
      } else {
        resD.push(high_arc);
      }
    }

    //Process all to-be-visited nodes in topological order
    while(!resD.empty()) {
      v = in_nodes.read_back();

      // Seek assignment
      while(in_assignment.can_read() && label_of(v) > a.label) {
        a = in_assignment.read();
      }

      // Process node and forward information
      if(a.label == label_of(v)) {
        uint64_t rec_child = a.value ? v.high : v.low;

        while(!resD.empty() && resD.top().target == v.node_ptr) {
          arc parent_arc = resD.top();
          arc request = create_arc(parent_arc.source, parent_arc.is_high, rec_child);

          if(is_sink(rec_child)) {
            if(is_nil(parent_arc.source)) {
              // We have restricted ourselves to a sink
              out_nodes.write(create_sink_node(value_of(rec_child)));
              return;
            } else {
              reduce_sink_arcs.write(request);
            }
          } else {
            resD.push(request);
          }

          resD.pop();
        }
      } else {
        // Outgoing arcs
        arc low_arc = low_arc_of_node(v);
        if(is_sink(v.low)) {
          reduce_sink_arcs.write(low_arc);
        } else {
          resD.push(low_arc);
        }

        arc high_arc = high_arc_of_node(v);
        if(is_sink(v.high)) {
          reduce_sink_arcs.write(high_arc);
        } else {
          resD.push(high_arc);
        }

        // Ingoing arcs
        while(!resD.empty() && resD.top().target == v.node_ptr) {
          auto parent_arc = resD.top();
          if(!is_nil(parent_arc.source)) {
            reduce_node_arcs.write(parent_arc);
          }

          resD.pop();
        }
      }
    }
  }

  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes)
  {
    debug::println_algorithm_start("RESTRICT");

    assert::is_valid_input_stream(in_nodes);
    debug::println_file_stream(in_nodes, "in_nodes");

    assert::is_valid_output_stream(out_nodes);

    if (in_assignment.size() == 0) {
      copy(in_nodes, out_nodes);
      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("RESTRICT");
      return;
    }

    if (is_sink(in_nodes, is_any)) {
      node n = in_nodes.can_read_back() ? in_nodes.read_back() : in_nodes.read();
      out_nodes.write(n);
      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("RESTRICT");
      return;
    }

    tpie::file_stream<arc> reduce_node_arcs;
    reduce_node_arcs.open();

    tpie::file_stream<arc> reduce_sink_arcs;
    reduce_sink_arcs.open();

    restrict(in_nodes, in_assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

    if (out_nodes.size() == 0) {
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    } else {
      debug::println_file_stream(out_nodes, "out_nodes");
    }

    debug::println_algorithm_end("RESTRICT");
  }
}

#endif // COOM_RESTRICT_CPP
