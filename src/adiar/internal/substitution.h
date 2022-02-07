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

  enum substitute_act { KEEP, FIX_FALSE, FIX_TRUE };

  //////////////////////////////////////////////////////////////////////////////
  class substitute_assignment_act
  {
    assignment_stream<> as;
    assignment_t a;

  public:
    typedef assignment_file action_t;

    substitute_assignment_act(const action_t &af) : as(af)
    {
      a = as.pull();
    }

    substitute_act action_for_level(label_t level) {
      while (label_of(a) < level && as.can_pull()) {
        a = as.pull();
      }

      if (label_of(a) == level) {
        return value_of(a) ? substitute_act::FIX_TRUE : substitute_act::FIX_FALSE;
      } else {
        return substitute_act::KEEP;
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename substitute_policy, typename substitute_act_mgr>
  inline substitute_rec substitute_apply_act(const substitute_act &act,
                                             const node_t &n,
                                             substitute_act_mgr &amgr)
  {
    switch (act) {
    case substitute_act::FIX_FALSE:
      return substitute_policy::fix_false(n, amgr);
    case substitute_act::FIX_TRUE:
      return substitute_policy::fix_true(n, amgr);
    default:
    case substitute_act::KEEP:
      return substitute_policy::keep_node(n, amgr);
    }
  }

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

  template<typename substitute_policy, typename substitute_act_mgr>
  typename substitute_policy::unreduced_t substitute(const typename substitute_policy::reduced_t &dd,
                                                     substitute_act_mgr &amgr)
  {
    node_stream<> ns(dd);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    substitute_priority_queue_t substitute_pq({dd},
                                              tpie::get_memory_manager().available(),
                                              std::numeric_limits<size_t>::max());

    label_t level = label_of(n);
    size_t level_size = 0;

    substitute_act action = amgr.action_for_level(level);

    // process the root and create initial recursion requests
    {
      const substitute_rec rec_res = substitute_apply_act<substitute_policy>(action, n, amgr);

      if (std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node_t n_res = std::get<substitute_rec_output>(rec_res).out;

        level_size = 1;

        substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);
      } else { // std::holds_alternative<substitute_rec_skipto>(n_res)
        const ptr_t rec_child = std::get<substitute_rec_skipto>(rec_res).child;;

        if(is_sink(rec_child)) {
          return substitute_policy::sink(value_of(rec_child), amgr);
        }

        substitute_pq.push({ NIL, rec_child });
      }
    }

    size_t max_1level_cut = 0;

    // process all to-be-visited nodes in topological order
    while(!substitute_pq.empty()) {
      if (substitute_pq.empty_level()) {
        if (level_size > 0) {
          aw.unsafe_push(create_level_info(level, level_size));
        }
        substitute_pq.setup_next_level();

        level_size = 0;
        level = substitute_pq.current_level();

        action = amgr.action_for_level(level);

        max_1level_cut = std::max(max_1level_cut, substitute_pq.size());
      }

      // seek requested node
      while (n.uid < substitute_pq.top().target) {
        n = ns.pull();
      }

      // process node and forward information
      const substitute_rec rec_res = substitute_apply_act<substitute_policy>(action, n, amgr);

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
            return substitute_policy::sink(value_of(rec_child), amgr);
          }

          substitute_resolve_request(request, substitute_pq, aw);
        }
      }
    }

    // Push the level of the very last iteration
    if (level_size > 0) {
      aw.unsafe_push(create_level_info(level, level_size));
    }

    out_arcs._file_ptr->max_1level_cut = max_1level_cut;
    return out_arcs;
  }
}

#endif // H_ADIAR_INTERNAL_SUBSTITUTION_H
