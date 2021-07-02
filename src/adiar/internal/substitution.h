#ifndef ADIAR_INTERNAL_SUBSTITUTION_H
#define ADIAR_INTERNAL_SUBSTITUTION_H

#include <variant>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  typedef levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt> substitute_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void substitute_resolve_request(const arc_t &request,
                                       substitute_priority_queue_t &pq,
                                       arc_writer &aw)
  {
    if(is_sink(request.target)) {
      aw.unsafe_push_sink(request);
    } else {
      pq.push(request);
    }
  }

  template<typename substitution_policy, typename in_t, typename out_t>
  out_t substitute(const in_t &in, const assignment_file &assignment)
  {
    if (assignment.size() == 0) {
      return substitution_policy::resolve_empty_assignment(in);
    }
    if (is_sink(in, is_any)) {
      return substitution_policy::resolve_sink_root(in, assignment);
    }
    if (disjoint_labels<assignment_file, assignment_stream<>>(assignment, in)) {
      return substitution_policy::resolve_disjoint_labels(in, assignment);
    }

    assignment_stream<> as(assignment);
    assignment_t a = as.pull();

    node_stream<> ns(in);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    substitute_priority_queue_t substitute_pq({in});

    label_t level = label_of(n);
    size_t level_size = 0;

    // find the next assignment
    while(as.can_pull() && label_of(n) > label_of(a)) {
      a = as.pull();
    }

    // process the root and create initial recursion requests
    if(label_of(a) == label_of(n)) {
      ptr_t rec_child;
      out_t maybe_resolved = substitution_policy::resolve_root_assign(n, a, in, assignment, rec_child);

      if (!(std::holds_alternative<no_file>(maybe_resolved._union))) {
        return maybe_resolved;
      }

      substitute_pq.push({ NIL, rec_child });
    } else {
      level_size = 1;

      substitute_resolve_request(low_arc_of(n), substitute_pq, aw);
      substitute_resolve_request(high_arc_of(n), substitute_pq, aw);
    }

    // process all to-be-visited nodes in topological order
    while(!substitute_pq.empty()) {
      if (substitute_pq.empty_level()) {
        if (level_size > 0) {
          aw.unsafe_push(create_meta(level, level_size));
        }
        substitute_pq.setup_next_level();

        level_size = 0;
        level = substitute_pq.current_level();

        // seek assignment
        while(as.can_pull() && level > label_of(a)) {
          a = as.pull();
        }
      }

      // seek requested node
      while (n.uid < substitute_pq.top().target) {
        n = ns.pull();
      }

      // process node and forward information
      if(label_of(a) == label_of(n)) {
        ptr_t rec_child = value_of(a) ? n.high : n.low;

        while(substitute_pq.can_pull() && substitute_pq.top().target == n.uid) {
          arc_t parent_arc = substitute_pq.pull();
          arc_t request = { parent_arc.source, rec_child };

          if(is_sink(rec_child) && is_nil(parent_arc.source)) {
            // we have restricted ourselves to a sink
            return substitution_policy::sink(value_of(rec_child));
          }

          substitute_resolve_request(request, substitute_pq, aw);
        }
      } else {
        // outgoing arcs
        substitute_resolve_request(low_arc_of(n), substitute_pq, aw);
        substitute_resolve_request(high_arc_of(n), substitute_pq, aw);

        // Ingoing arcs
        while(substitute_pq.can_pull() && substitute_pq.top().target == n.uid) {
          arc_t parent_arc = substitute_pq.pull();

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

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Generalised BDD 'composition'
}

#endif // H_ADIAR_INTERNAL_SUBSTITUTION_H
