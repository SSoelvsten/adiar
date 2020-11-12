#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include "restrict.h"

#include <tpie/sort.h>

#include <coom/reduce.h>
#include <coom/priority_queue.h>

#include <coom/util.h>
#include <coom/assert.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
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

  typedef node_priority_queue<arc_t, restrict_queue_label, restrict_queue_lt> restrict_priority_queue_t;
  typedef tpie::merge_sorter<arc_t, false, by_source_lt> restrict_sink_sorter_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void restrict_resolve_request(const arc_t &request,
                                       restrict_priority_queue_t &pq,
                                       restrict_sink_sorter_t &ss)
  {
    if(is_sink_ptr(request.target)) {
      ss.push(request);
    } else {
      pq.push(request);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_restrict(const node_file &bdd, const assignment_file &assignment)
  {
    node_or_arc_file out_union;

    if (assignment.size() == 0 || is_sink(bdd, is_any)) {
      out_union << bdd;
      return out_union;
    }

    restrict_priority_queue_t resD;
    resD.hook_meta_stream(bdd);

    node_stream<> ns(bdd);
    node_t n = ns.pull();

    arc_file out_arcs;

    arc_writer aw(out_arcs);

    restrict_sink_sorter_t sink_sorter;
    sink_sorter.set_available_memory(tpie::get_memory_manager().available());
    sink_sorter.begin();

    assignment_stream as(assignment);
    assignment_t a = as.pull();

    // find the next assignment
    while(as.can_pull() && label_of(n) > a.label) {
      a = as.pull();
    }

    // process the root and create initial recursion requests
    if(a.label == label_of(n)) {
      ptr_t rec_child = a.value ? n.high : n.low;

      if(is_sink_ptr(rec_child)) {
        node_file out_nodes;
        node_writer nw(out_nodes);
        nw.unsafe_push(create_sink(value_of(rec_child)));

        return out_union << out_nodes;
      }

      resD.push({ NIL, rec_child });
    } else {
      aw.unsafe_push(meta_t {label_of(n)});

      restrict_resolve_request(low_arc_of(n), resD, sink_sorter);
      restrict_resolve_request(high_arc_of(n), resD, sink_sorter);
    }

    // process all to-be-visited nodes in topological order
    while(!resD.empty()) {
      if (resD.empty_layer()) {
        resD.setup_next_layer();

        // seek assignment
        while(as.can_pull() && resD.current_layer() > a.label) {
          a = as.pull();
        }

        // will any nodes in this layer be outputted?
        if (a.label != resD.current_layer()) {
          aw.unsafe_push(meta_t { resD.current_layer() });
        }
      }

      // seek requested node
      while (n.uid != resD.top().target) {
        n = ns.pull();
      }

      // process node and forward information
      if(a.label == label_of(n)) {
        ptr_t rec_child = a.value ? n.high : n.low;

        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();
          arc_t request = { parent_arc.source, rec_child };

          if(is_sink_ptr(rec_child) && is_nil(parent_arc.source)) {
            // we have restricted ourselves to a sink
            node_file out_nodes;
            node_writer nw(out_nodes);
            nw.unsafe_push(create_sink(value_of(rec_child)));

            return out_union << out_nodes;
          }

          restrict_resolve_request(request, resD, sink_sorter);

        }
      } else {
        // outgoing arcs
        restrict_resolve_request(low_arc_of(n), resD, sink_sorter);
        restrict_resolve_request(high_arc_of(n), resD, sink_sorter);

        // Ingoing arcs
        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();

          if(!is_nil(parent_arc.source)) {
            aw.unsafe_push_node(parent_arc);
          }
        }
      }
    }

    sort_into_file(sink_sorter, out_arcs._file_ptr -> _files[1]);
    return out_union << out_arcs;
  }
}

#endif // COOM_RESTRICT_CPP
