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
  /// Struct to hold statistics
  extern stats_t::substitute_t stats_substitute;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using substitute_priority_queue_t =
    levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt,
                                  sorter_template, priority_queue_template>;

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

  template<typename pq_t>
  inline void __substitute_resolve_request(const arc_t &request,
                                         pq_t &pq,
                                         arc_writer &aw)
  {
    if(is_sink(request.target)) {
      aw.unsafe_push_sink(request);
    } else {
      pq.push(request);
    }
  }

  template<typename substitute_policy, typename substitute_act_mgr, typename pq_t>
  typename substitute_policy::unreduced_t __substitute
    (const typename substitute_policy::reduced_t &dd, node_stream<> &ns, node_t &n,
     substitute_act_mgr &amgr,
     arc_file &out_arcs, arc_writer &aw,
     const tpie::memory_size_type available_memory_lpq,
     const size_t max_pq_size)
  {
    pq_t substitute_pq({dd}, available_memory_lpq, max_pq_size);

    label_t level = label_of(n);
    size_t level_size = 0;

    substitute_act action = amgr.action_for_level(level);

    // process the root and create initial recursion requests
    {
      const substitute_rec rec_res = substitute_apply_act<substitute_policy>(action, n, amgr);

      if (std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node_t n_res = std::get<substitute_rec_output>(rec_res).out;

        level_size = 1;

        __substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        __substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);
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
        __substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        __substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);

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

          __substitute_resolve_request(request, substitute_pq, aw);
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

  template<typename substitute_policy>
  size_t __substitute_size_based_upper_bound(const typename substitute_policy::reduced_t &dd)
  {
    // Can the size_bound computation overflow?
    const size_t number_of_nodes = dd.file_ptr()->size();
    const bits_approximation input_bits(number_of_nodes);

    const bits_approximation bound_bits = input_bits + 2;

    if(bound_bits.may_overflow()) {
      return std::numeric_limits<size_t>::max();
    } else {
      return number_of_nodes + 2;
    }
  }

  template<typename substitute_policy, typename substitute_act_mgr>
  typename substitute_policy::unreduced_t substitute(const typename substitute_policy::reduced_t &dd,
                                                     substitute_act_mgr &amgr)
  {
    // Set up to run the substitution algorithm
    node_stream<> ns(dd);
    node_t n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    // Derive an upper bound on the size of auxiliary data structures and check
    // whether we can run them with a faster internal memory variant.
    const tpie::memory_size_type available_memory = tpie::get_memory_manager().available();
    const size_t size_bound = __substitute_size_based_upper_bound<substitute_policy>(dd);

    const tpie::memory_size_type lpq_memory_fits =
      substitute_priority_queue_t<internal_sorter, internal_priority_queue>::memory_fits(available_memory);

    if(size_bound <= lpq_memory_fits) {
#ifdef ADIAR_STATS
      stats_substitute.lpq_internal++;
#endif
      return __substitute<substitute_policy, substitute_act_mgr,
                          substitute_priority_queue_t<internal_sorter, internal_priority_queue>>
        (dd, ns, n, amgr, out_arcs, aw, available_memory, size_bound);
    } else {
#ifdef ADIAR_STATS
      stats_substitute.lpq_external++;
#endif
      return __substitute<substitute_policy, substitute_act_mgr,
                          substitute_priority_queue_t<external_sorter, external_priority_queue>>
        (dd, ns, n, amgr, out_arcs, aw, available_memory, size_bound);
    }
  }
}

#endif // H_ADIAR_INTERNAL_SUBSTITUTION_H
