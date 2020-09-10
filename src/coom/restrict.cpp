#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include "data.h"
#include "priority_queue.cpp"

#include "assignment.h"
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
    inline void println_restrict_request([[maybe_unused]] const arc_t &arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << std::endl << "| request: ";
      print_node_ptr(arc.target);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_restrict_position([[maybe_unused]] const node_t &v)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  current: " << std::endl << "|     | ";
      print_node(v);
      tpie::log_info() << std::endl;
#endif
    }

    inline void println_restrict_skip([[maybe_unused]] uid_t out_node_uid)
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

    inline void println_restrict_request([[maybe_unused]] arc_t &out_arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  request: ";
      debug::println_arc(out_arc);
#endif
    }

    inline void println_restrict_ingoing([[maybe_unused]] arc_t &in_arc)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  in: ";
      debug::println_arc(in_arc);
#endif
    }
  }


  struct restrict_queue_lt {
    bool operator ()(const arc_t& a, const arc_t& b) {
      return a.target < b.target;
    }
  };

  struct restrict_queue_label {
    label_t label_of(const arc_t& a) {
      return coom::label_of(a.target);
    }
  };

  const auto restrict_sink_lt = [](const arc_t& a, const arc_t& b) -> bool {
    return a.source < b.source;
  };

  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                tpie::file_stream<assignment_t> &in_assignment,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<arc_t> &reduce_node_arcs,
                tpie::file_stream<arc_t> &reduce_sink_arcs,
                tpie::file_stream<meta_t> &reduce_meta)
  {
    priority_queue<arc_t, restrict_queue_label, restrict_queue_lt, 1, std::less<label_t>> resD;
    resD.hook_meta_stream(in_meta);

    in_nodes.seek(0, tpie::file_stream_base::end);
    node_t n = in_nodes.read_back();

    in_assignment.seek(0);
    assignment_t a = in_assignment.read();

    // find the next assignment
    while(in_assignment.can_read() && label_of(n) > a.label) {
      a = in_assignment.read();
    }

    tpie::file_stream<arc_t> good_sink_arcs;
    good_sink_arcs.open();

    tpie::file_stream<arc_t> bad_sink_arcs;
    bad_sink_arcs.open();

    arc_t latest_good_sink_arc = { NIL, NIL };

    // process the root and create initial recursion requests
    if(a.label == label_of(n)) {
      ptr_t rec_child = a.value ? n.high : n.low;

      if(is_sink_ptr(rec_child)) {
        out_nodes.write(create_sink(value_of(rec_child)));
        return;
      }

      resD.push({ NIL, rec_child });
    } else {
      reduce_meta.write({label_of(n)});

      arc_t low_arc = low_arc_of(n);
      if(is_sink_ptr(n.low)) {
        latest_good_sink_arc = low_arc;
        good_sink_arcs.write(low_arc);
      } else {
        resD.push(low_arc);
      }

      arc_t high_arc = high_arc_of(n);
      if(is_sink_ptr(n.high)) {
        latest_good_sink_arc = high_arc;
        good_sink_arcs.write(high_arc);
      } else {
        resD.push(high_arc);
      }
    }

    // process all to-be-visited nodes in topological order
    while(!resD.empty()) {
      if (resD.empty_layer()) {
        resD.setup_next_layer();

        // seek assignment
        while(in_assignment.can_read() && resD.current_layer() > a.label) {
          a = in_assignment.read();
        }

        // will any nodes in this layer be outputted?
        if (a.label != resD.current_layer()) {
          reduce_meta.write({ resD.current_layer() });
        }
      }

      debug::println_restrict_request(resD.top());

      // seek requested node
      while (n.uid != resD.top().target) {
        n = in_nodes.read_back();
      }

      debug::println_restrict_position(n);

      // process node and forward information
      if(a.label == label_of(n)) {
        ptr_t rec_child = a.value ? n.high : n.low;

        debug::println_restrict_skip(rec_child);

        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();
          arc_t request = { parent_arc.source, rec_child };

          debug::println_restrict_request(request);

          if(is_sink_ptr(rec_child)) {
            if(is_nil(parent_arc.source)) {
              // we have restricted ourselves to a sink
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
        }
      } else {
        debug::println_restrict_keep();
        // outgoing arcs
        arc_t low_arc = low_arc_of(n);
        debug::println_restrict_request(low_arc);

        if(is_sink_ptr(n.low)) {
          good_sink_arcs.write(low_arc);
          latest_good_sink_arc = low_arc;
        } else {
          resD.push(low_arc);
        }

        arc_t high_arc = high_arc_of(n);
        debug::println_restrict_request(high_arc);
        if(is_sink_ptr(n.high)) {
          good_sink_arcs.write(high_arc);
          latest_good_sink_arc = high_arc;
        } else {
          resD.push(high_arc);
        }

        // Ingoing arcs
        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();
          debug::println_restrict_ingoing(parent_arc);

          if(!is_nil(parent_arc.source)) {
            reduce_node_arcs.write(parent_arc);
          }
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

  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta)
  {
    debug::println_algorithm_start("RESTRICT");

    assert::is_valid_input_stream(in_nodes);
    debug::println_file_stream(in_nodes, "in_nodes");

    debug::println_file_stream(in_assignment, "in_assignment");

    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

    if (in_assignment.size() == 0) {
      copy(in_nodes, out_nodes);
      copy(in_meta, out_meta);
      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("RESTRICT");
      return;
    }

    if (is_sink(in_nodes, is_any)) {
      node_t n = in_nodes.can_read_back() ? in_nodes.read_back() : in_nodes.read();
      out_nodes.write(n);
      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("RESTRICT");
      return;
    }

    tpie::file_stream<arc_t> reduce_node_arcs;
    reduce_node_arcs.open();

    tpie::file_stream<arc_t> reduce_sink_arcs;
    reduce_sink_arcs.open();

    tpie::file_stream<meta_t> reduce_meta;
    reduce_meta.open();

    restrict(in_nodes, in_meta, in_assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs, reduce_meta);

    if (out_nodes.size() == 0) {
      reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta, out_nodes, out_meta);
    } else {
      debug::println_file_stream(out_nodes, "out_nodes");
    }

    debug::println_algorithm_end("RESTRICT");
  }
}

#endif // COOM_RESTRICT_CPP
