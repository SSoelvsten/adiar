#include "restrict.h"

#include <adiar/data.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/util.h>

#include <adiar/bdd/build.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  typedef levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt> restrict_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void restrict_resolve_request(const arc_t &request,
                                       restrict_priority_queue_t &pq,
                                       arc_writer &aw)
  {
    if(is_sink(request.target)) {
      aw.unsafe_push_sink(request);
    } else {
      pq.push(request);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &bdd, const assignment_file &assignment)
  {
    if (assignment.size() == 0
        || is_sink(bdd, is_any)
        || disjoint_labels<assignment_file, assignment_stream<>>(assignment, bdd)) {
      return bdd;
    }

    node_stream<> ns(bdd);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    assignment_stream<> as(assignment);
    assignment_t a = as.pull();

    restrict_priority_queue_t restrict_pq({bdd});

    label_t level = label_of(n);
    size_t level_size = 0;

    // find the next assignment
    while(as.can_pull() && label_of(a) < label_of(n)) {
      a = as.pull();
    }

    // process the root and create initial recursion requests
    if(label_of(a) == label_of(n)) {
      ptr_t rec_child = value_of(a) ? n.high : n.low;

      if(is_sink(rec_child)) {
        return bdd_sink(value_of(rec_child));
      }

      restrict_pq.push({ NIL, rec_child });
    } else {
      level_size = 1;

      restrict_resolve_request(low_arc_of(n), restrict_pq, aw);
      restrict_resolve_request(high_arc_of(n), restrict_pq, aw);
    }

    // process all to-be-visited nodes in topological order
    while(!restrict_pq.empty()) {
      if (restrict_pq.empty_level()) {
        if (level_size > 0) {
          aw.unsafe_push(create_meta(level, level_size));
        }
        restrict_pq.setup_next_level();

        level_size = 0;
        level = restrict_pq.current_level();

        // seek assignment
        while(as.can_pull() && level > label_of(a)) {
          a = as.pull();
        }
      }

      // seek requested node
      while (n.uid != restrict_pq.top().target) {
        n = ns.pull();
      }

      // process node and forward information
      if(label_of(a) == label_of(n)) {
        ptr_t rec_child = value_of(a) ? n.high : n.low;

        while(restrict_pq.can_pull() && restrict_pq.top().target == n.uid) {
          arc_t parent_arc = restrict_pq.pull();
          arc_t request = { parent_arc.source, rec_child };

          if(is_sink(rec_child) && is_nil(parent_arc.source)) {
            // we have restricted ourselves to a sink
            return bdd_sink(value_of(rec_child));
          }

          restrict_resolve_request(request, restrict_pq, aw);
        }
      } else {
        // outgoing arcs
        restrict_resolve_request(low_arc_of(n), restrict_pq, aw);
        restrict_resolve_request(high_arc_of(n), restrict_pq, aw);

        // Ingoing arcs
        while(restrict_pq.can_pull() && restrict_pq.top().target == n.uid) {
          arc_t parent_arc = restrict_pq.pull();

          if(!is_nil(parent_arc.source)) {
            aw.unsafe_push_node(parent_arc);
          }
        }

        level_size++;
      }
    }

    // Push the level of the very last iteration
    if (level_size > 0) {
      aw.unsafe_push(create_meta(level, level_size));
    }

    return out_arcs;
  }
}
