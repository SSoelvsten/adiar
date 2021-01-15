#ifndef ADIAR_RESTRICT_CPP
#define ADIAR_RESTRICT_CPP

#include "restrict.h"

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/priority_queue.h>
#include <adiar/reduce.h>
#include <adiar/util.h>

#include <adiar/bdd/build.h>

namespace adiar
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
      return adiar::label_of(a.target);
    }
  };

  typedef node_priority_queue<arc_t, restrict_queue_label, restrict_queue_lt> restrict_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void restrict_resolve_request(const arc_t &request,
                                       restrict_priority_queue_t &pq,
                                       arc_writer &aw)
  {
    if(is_sink_ptr(request.target)) {
      aw.unsafe_push_sink(request);
    } else {
      pq.push(request);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &bdd, const assignment_file &assignment)
  {
    if (assignment.size() == 0 || is_sink(bdd, is_any)) {
      return bdd;
    }

    node_stream<> ns(bdd);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    assignment_stream<> as(assignment);
    assignment_t a = as.pull();

    restrict_priority_queue_t resD;
    resD.hook_meta_stream(bdd);

    // find the next assignment
    while(as.can_pull() && label_of(n) > a.label) {
      a = as.pull();
    }

    // process the root and create initial recursion requests
    if(a.label == label_of(n)) {
      ptr_t rec_child = a.value ? n.high : n.low;

      if(is_sink_ptr(rec_child)) {
        return bdd_sink(value_of(rec_child));
      }

      resD.push({ NIL, rec_child });
    } else {
      aw.unsafe_push(meta_t {label_of(n)});

      restrict_resolve_request(low_arc_of(n), resD, aw);
      restrict_resolve_request(high_arc_of(n), resD, aw);
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
            return bdd_sink(value_of(rec_child));
          }

          restrict_resolve_request(request, resD, aw);

        }
      } else {
        // outgoing arcs
        restrict_resolve_request(low_arc_of(n), resD, aw);
        restrict_resolve_request(high_arc_of(n), resD, aw);

        // Ingoing arcs
        while(resD.can_pull() && resD.top().target == n.uid) {
          arc_t parent_arc = resD.pull();

          if(!is_nil(parent_arc.source)) {
            aw.unsafe_push_node(parent_arc);
          }
        }
      }
    }

    // TODO: Add bool variable to check whether we really do need to sort.
    aw.sort_sinks();

    return out_arcs;
  }
}

#endif // ADIAR_RESTRICT_CPP
