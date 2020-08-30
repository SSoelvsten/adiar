#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include <tpie/priority_queue.h>

#include "assignment.h"
#include "data.h"
#include "pred.h"
#include "reduce.h"

#include "util.cpp"

#include "assert.h"

#include "debug.h"
#include "debug_data.h"
#include "debug_assignment.h"

#include "restrict.h"

namespace coom
{
  namespace debug {
    inline void println_restrict_request([[maybe_unused]] const arc& arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << std::endl << "| request: ";
      print_node_ptr(arc.target);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_restrict_position([[maybe_unused]] const node& v)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  current: " << std::endl << "|     | ";
      print_node(v);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_restrict_skip([[maybe_unused]] uint64_t out_node_uid)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  skip to: ";
      print_child(out_node_uid);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_restrict_keep()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  keep... " << std::endl;
#endif
    }

    inline void println_restrict_request([[maybe_unused]] arc& out_arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  request: ";
      debug::println_arc(out_arc);
#endif
    }

    inline void println_restrict_ingoing([[maybe_unused]] arc& in_arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  in: ";
      debug::println_arc(in_arc);
#endif
    }
  }


  struct restrict_lt {
    bool operator ()(const arc& a, const arc& b) {
      return a.target < b.target || (a.target == b.target &&
                                     (is_nil(a.source) ||
                                      (!is_nil(b.source) && a.source < b.source)));
    }
  };

  const auto restrict_sink_lt = [](const arc& a, const arc& b) -> bool {
    return a.source < b.source;
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

    node n = in_nodes.read_back();
    assignment a = in_assignment.read();

    // Find the next assignment
    while(in_assignment.can_read() && label_of(n) > a.label) {
      a = in_assignment.read();
    }

    tpie::file_stream<arc> good_sink_arcs;
    good_sink_arcs.open();

    tpie::file_stream<arc> bad_sink_arcs;
    bad_sink_arcs.open();

    arc latest_good_sink_arc = { NIL, NIL };

    // Process the root and create initial recursion requests
    if(a.label == label_of(n)) {
      uint64_t rec_child = a.value ? n.high : n.low;

      if(is_sink_ptr(rec_child)) {
        out_nodes.write(create_sink(value_of(rec_child)));
        return;
      }

      resD.push({ NIL, rec_child });
    } else {
      arc low_arc = low_arc_of(n);
      if(is_sink_ptr(n.low)) {
        latest_good_sink_arc = low_arc;
        good_sink_arcs.write(low_arc);
      } else {
        resD.push(low_arc);
      }

      arc high_arc = high_arc_of(n);
      if(is_sink_ptr(n.high)) {
        latest_good_sink_arc = high_arc;
        good_sink_arcs.write(high_arc);
      } else {
        resD.push(high_arc);
      }
    }

    // Process all to-be-visited nodes in topological order
    while(!resD.empty()) {
      debug::println_restrict_request(resD.top());

      // Seek requeted node
      while (n.uid != resD.top().target) {
        n = in_nodes.read_back();
      }

      debug::println_restrict_position(n);

      // Seek assignment
      while(in_assignment.can_read() && label_of(n) > a.label) {
        a = in_assignment.read();
      }

      // Process node and forward information
      if(a.label == label_of(n)) {
        uint64_t rec_child = a.value ? n.high : n.low;

        debug::println_restrict_skip(rec_child);

        while(!resD.empty() && resD.top().target == n.uid) {
          arc parent_arc = resD.top();
          arc request = { parent_arc.source, rec_child };

          debug::println_restrict_request(request);

          if(is_sink_ptr(rec_child)) {
            if(is_nil(parent_arc.source)) {
              // We have restricted ourselves to a sink
              out_nodes.write(create_sink(value_of(rec_child)));
              return;
            } else if (good_sink_arcs.size() > 0 && restrict_sink_lt(request, latest_good_sink_arc)) {
              bad_sink_arcs.write(request);
            } else {
              good_sink_arcs.write(request);
              latest_good_sink_arc = request;
            }
          } else {
            resD.push(request);
          }

          resD.pop();
        }
      } else {
        debug::println_restrict_keep();
        // Outgoing arcs
        arc low_arc = low_arc_of(n);
        debug::println_restrict_request(low_arc);

        if(is_sink_ptr(n.low)) {
          good_sink_arcs.write(low_arc);
          latest_good_sink_arc = low_arc;
        } else {
          resD.push(low_arc);
        }

        arc high_arc = high_arc_of(n);
        debug::println_restrict_request(high_arc);
        if(is_sink_ptr(n.high)) {
          good_sink_arcs.write(high_arc);
          latest_good_sink_arc = high_arc;
        } else {
          resD.push(high_arc);
        }

        // Ingoing arcs
        while(!resD.empty() && resD.top().target == n.uid) {
          auto parent_arc = resD.top();
          debug::println_restrict_ingoing(parent_arc);

          if(!is_nil(parent_arc.source)) {
            reduce_node_arcs.write(parent_arc);
          }

          resD.pop();
        }
      }
    }

    tpie::progress_indicator_null pi;
    tpie::sort(bad_sink_arcs, restrict_sink_lt, pi);

    debug::println_file_stream(good_sink_arcs, "good_sink_arcs");
    debug::println_file_stream(bad_sink_arcs, "bad_sink_arcs");

    good_sink_arcs.seek(0);
    bad_sink_arcs.seek(0);

    while (good_sink_arcs.can_read() || bad_sink_arcs.can_read()) {
      if (!bad_sink_arcs.can_read() ||
          restrict_sink_lt(good_sink_arcs.peek(), bad_sink_arcs.peek())) {
        reduce_sink_arcs.write(good_sink_arcs.read());
      } else {
        reduce_sink_arcs.write(bad_sink_arcs.read());
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

    debug::println_file_stream(in_assignment, "in_assignment");

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
