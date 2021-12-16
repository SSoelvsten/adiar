#ifndef ADIAR_INTERNAL_SUBSTITUTION_H
#define ADIAR_INTERNAL_SUBSTITUTION_H

#include <variant>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/levelized_priority_queue.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  typedef levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt>
  substitute_priority_queue_t;

  struct substitute_rec_output { node_t out; };
  struct substitute_rec_skipto { ptr_t child; };

  typedef std::variant<substitute_rec_output, substitute_rec_skipto> substitute_rec;

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

  template<typename substitution_policy>
  typename substitution_policy::unreduced_t substitute(const typename substitution_policy::reduced_t &dd,
                                                       const typename substitution_policy::substitution_t &na)
  {
    if (na.size() == 0 || is_sink(dd, is_any) || substitution_policy::disjoint_labels(na, dd)) {
      return dd;
    }

    typename substitution_policy::substitution_mgr smgr(na);

    node_stream<> ns(dd);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    substitute_priority_queue_t substitute_pq({dd});

    label_t level = label_of(n);
    size_t level_size = 0;

    // process the root and create initial recursion requests
    {
      smgr.setup_for_level(level);
      const substitute_rec rec_res = smgr.resolve_node(n, level);

      if (std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node_t n_res = std::get<substitute_rec_output>(rec_res).out;

        level_size = 1;

        substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);
      } else { // std::holds_alternative<substitute_rec_skipto>(n_res)
        const ptr_t rec_child = std::get<substitute_rec_skipto>(rec_res).child;;

        if(is_sink(rec_child)) {
          return substitution_policy::sink(value_of(rec_child));
        }

        substitute_pq.push({ NIL, rec_child });
      }
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

        smgr.setup_for_level(level);
      }

      // seek requested node
      while (n.uid < substitute_pq.top().target) {
        n = ns.pull();
      }

      // process node and forward information
      const substitute_rec rec_res = smgr.resolve_node(n, level);

      if(std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node_t n_res = std::get<substitute_rec_output>(rec_res).out;

        // outgoing arcs
        substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);

        // Ingoing arcs
        while(substitute_pq.can_pull() && substitute_pq.top().target == n_res.uid) {
          const arc_t parent_arc = substitute_pq.pull();

          if(!is_nil(parent_arc.source)) {
            aw.unsafe_push_node(parent_arc);
          }
        }

        level_size++;
      } else { // std::holds_alternative<substitute_rec_skipto>(rec_res)
        const ptr_t rec_child = std::get<substitute_rec_skipto>(rec_res).child;

        while(substitute_pq.can_pull() && substitute_pq.top().target == n.uid) {
          const arc_t parent_arc = substitute_pq.pull();
          const arc_t request = { parent_arc.source, rec_child };

          if(is_sink(rec_child) && is_nil(parent_arc.source)) {
            // we have restricted ourselves to a sink
            return substitution_policy::sink(value_of(rec_child));
          }

          substitute_resolve_request(request, substitute_pq, aw);
        }
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
