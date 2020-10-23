#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include "restrict.h"

#include <tpie/sort.h>

#include "data.h"

#include "reduce.h"

#include "priority_queue.cpp"
#include "util.cpp"

#include "assert.h"

namespace coom
{
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
        reduce_sink_arcs.write(low_arc);
      } else {
        resD.push(low_arc);
      }

      arc_t high_arc = high_arc_of(n);
      if(is_sink_ptr(n.high)) {
        latest_good_sink_arc = high_arc;
        reduce_sink_arcs.write(high_arc);
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

      // seek requested node
      while (n.uid != resD.top().target) {
        n = in_nodes.read_back();
      }

      // process node and forward information
      if(a.label == label_of(n)) {
        ptr_t rec_child = a.value ? n.high : n.low;

        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();
          arc_t request = { parent_arc.source, rec_child };

          if(is_sink_ptr(rec_child)) {
            if(is_nil(parent_arc.source)) {
              // we have restricted ourselves to a sink
              out_nodes.write(create_sink(value_of(rec_child)));
              return;
            } else if (reduce_sink_arcs.size() > 0 && restrict_sink_lt(request, latest_good_sink_arc)) {
              reduce_sink_arcs.write(request);
            } else {
              reduce_sink_arcs.write(request);
              latest_good_sink_arc = request;
            }
          } else {
            resD.push(request);
          }
        }
      } else {
        // outgoing arcs
        arc_t low_arc = low_arc_of(n);

        if(is_sink_ptr(n.low)) {
          reduce_sink_arcs.write(low_arc);
          latest_good_sink_arc = low_arc;
        } else {
          resD.push(low_arc);
        }

        arc_t high_arc = high_arc_of(n);
        if(is_sink_ptr(n.high)) {
          reduce_sink_arcs.write(high_arc);
          latest_good_sink_arc = high_arc;
        } else {
          resD.push(high_arc);
        }

        // Ingoing arcs
        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();

          if(!is_nil(parent_arc.source)) {
            reduce_node_arcs.write(parent_arc);
          }
        }
      }
    }

    tpie::progress_indicator_null pi;
    tpie::sort(reduce_sink_arcs, restrict_sink_lt, pi);
  }

  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta)
  {
    assert::is_valid_input_stream(in_nodes);
    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

    if (in_assignment.size() == 0) {
      copy(in_nodes, out_nodes);
      copy(in_meta, out_meta);
      return;
    }

    if (is_sink(in_nodes, is_any)) {
      node_t n = in_nodes.can_read_back() ? in_nodes.read_back() : in_nodes.read();
      out_nodes.write(n);
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
    }
  }
}

#endif // COOM_RESTRICT_CPP
