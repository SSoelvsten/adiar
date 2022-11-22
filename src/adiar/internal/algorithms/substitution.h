#ifndef ADIAR_INTERNAL_ALGORITHMS_SUBSTITUTION_H
#define ADIAR_INTERNAL_ALGORITHMS_SUBSTITUTION_H

#include <variant>

#include <adiar/assignment.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/dd.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/convert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::substitute_t stats_substitute;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<size_t LOOK_AHEAD, memory::memory_mode mem_mode>
  using substitute_priority_queue_t =
    levelized_node_priority_queue<arc, arc_target_label,
                                  arc_target_lt, LOOK_AHEAD,
                                  mem_mode>;

  struct substitute_rec_output { node out; };
  struct substitute_rec_skipto { ptr_uint64 child; };

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

    substitute_act action_for_level(assignment_t::label_t level) {
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
                                             const node &n,
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
  inline void __substitute_resolve_request(const arc &request,
                                         pq_t &pq,
                                         arc_writer &aw)
  {
    if(request.target().is_terminal()) {
      aw.unsafe_push_terminal(request);
    } else {
      pq.push(request);
    }
  }

  template<typename substitute_policy, typename substitute_act_mgr, typename pq_t>
  typename substitute_policy::unreduced_t __substitute(const typename substitute_policy::reduced_t &dd,
                                                       substitute_act_mgr &amgr,
                                                       const size_t pq_memory,
                                                       const size_t pq_max_size)
  {
    // Set up to run the substitution algorithm
    node_stream<> ns(dd);
    node n = ns.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_t substitute_pq({dd}, pq_memory, pq_max_size, stats_substitute.lpq);

    typename substitute_policy::label_t level = n.label();
    size_t level_size = 0;

    substitute_act action = amgr.action_for_level(level);

    // process the root and create initial recursion requests
    {
      const substitute_rec rec_res = substitute_apply_act<substitute_policy>(action, n, amgr);

      if (std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node n_res = std::get<substitute_rec_output>(rec_res).out;

        level_size = 1;

        __substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        __substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);
      } else { // std::holds_alternative<substitute_rec_skipto>(n_res)
        const ptr_uint64 rec_child = std::get<substitute_rec_skipto>(rec_res).child;;

        if(rec_child.is_terminal()) {
          return substitute_policy::terminal(rec_child.value(), amgr);
        }

        substitute_pq.push(arc(ptr_uint64::NIL(), rec_child));
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
      while (n.uid() < substitute_pq.top().target()) {
        n = ns.pull();
      }

      // process node and forward information
      const substitute_rec rec_res = substitute_apply_act<substitute_policy>(action, n, amgr);

      if(std::holds_alternative<substitute_rec_output>(rec_res)) {
        const node n_res = std::get<substitute_rec_output>(rec_res).out;

        // outgoing arcs
        __substitute_resolve_request(low_arc_of(n_res), substitute_pq, aw);
        __substitute_resolve_request(high_arc_of(n_res), substitute_pq, aw);

        // Ingoing arcs
        while(substitute_pq.can_pull() && substitute_pq.top().target() == n_res.uid()) {
          const arc parent_arc = substitute_pq.pull();

          if(!parent_arc.source().is_nil()) {
            aw.unsafe_push_node(parent_arc);
          }
        }

        level_size++;
      } else { // std::holds_alternative<substitute_rec_skipto>(rec_res)
        const ptr_uint64 rec_child = std::get<substitute_rec_skipto>(rec_res).child;

        while(substitute_pq.can_pull() && substitute_pq.top().target() == n.uid()) {
          const arc parent_arc = substitute_pq.pull();
          const arc request = { parent_arc.source(), rec_child };

          if(rec_child.is_terminal() && parent_arc.source().is_nil()) {
            // we have restricted ourselves to a terminal
            return substitute_policy::terminal(rec_child.value(), amgr);
          }

          __substitute_resolve_request(request, substitute_pq, aw);
        }
      }
    }

    // Push the level of the very last iteration
    if (level_size > 0) {
      aw.unsafe_push(create_level_info(level, level_size));
    }

    out_arcs->max_1level_cut = max_1level_cut;
    return out_arcs;
  }

  template<typename substitute_policy>
  size_t __substitute_2level_upper_bound(const typename substitute_policy::reduced_t &dd)
  {
    const safe_size_t max_2level_cut = dd.max_2level_cut(cut_type::INTERNAL);
    return to_size(max_2level_cut + 2u);
  }

  template<typename substitute_policy, typename substitute_act_mgr>
  typename substitute_policy::unreduced_t substitute(const typename substitute_policy::reduced_t &dd,
                                                     substitute_act_mgr &amgr)
  {
    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const tpie::memory_size_type aux_available_memory = memory::available()
      - node_stream<>::memory_usage() - arc_writer::memory_usage();

    const tpie::memory_size_type pq_memory_fits =
      substitute_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::memory_fits(aux_available_memory);

    const bool internal_only = memory::mode == memory::INTERNAL;
    const bool external_only = memory::mode == memory::EXTERNAL;

    const size_t pq_bound = __substitute_2level_upper_bound<substitute_policy>(dd);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_substitute.lpq.unbucketed++;
#endif
      return __substitute<substitute_policy, substitute_act_mgr,
                          substitute_priority_queue_t<0, memory::INTERNAL>>
        (dd, amgr, aux_available_memory, max_pq_size);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_substitute.lpq.internal++;
#endif
      return __substitute<substitute_policy, substitute_act_mgr,
                          substitute_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>>
        (dd, amgr, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_substitute.lpq.external++;
#endif
      return __substitute<substitute_policy, substitute_act_mgr,
                          substitute_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory::EXTERNAL>>
        (dd, amgr, aux_available_memory, max_pq_size);
    }
  }
}

#endif // H_ADIAR_INTERNAL_ALGORITHMS_SUBSTITUTION_H
