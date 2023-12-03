#ifndef ADIAR_INTERNAL_ALGORITHMS_SELECT_H
#define ADIAR_INTERNAL_ALGORITHMS_SELECT_H

#include <variant>

#include <adiar/exec_policy.h>
#include <adiar/types.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  Select Algorithm
  // ==================
  //
  // Given a Decision Diagram, a subset of the levels are removed and bridged
  // over in either the 'high' or the 'low' direction.
  /*
  //          _( )_     ---- xi                _( )_        ---- xi
  //         /     \                          /     \
  //        ( )   ( )   ---- xj     =>        \      \
  //        / \   / \                          \      \
  //        a b   c d                          b      d
  */
  // Examples of uses are `bdd_restrict` and `zdd_onset` / `zdd_offset`
  //
  // As such, this is merely a niche-case of the Intercut algorithm, where the
  // *onset* levels are removed and bridged over in a uniform direction (and the
  // edges crossing said level only have suppressed nodes added). Yet compared
  // to the Intercut algorithm, this implementation is optimised both in terms
  // of time and space.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::select_t stats_select;

  //////////////////////////////////////////////////////////////////////////////
  // Data Structures
  using select_request = request_data<1, with_parent>;

  template<size_t look_ahead, memory_mode mem_mode>
  using select_priority_queue_t =
    levelized_node_priority_queue<select_request, request_data_first_lt<select_request>,
                                  look_ahead,
                                  mem_mode,
                                  1u,
                                  0u>;

  /// Return a desired `node` to be output or a `node::pointer_type` to skip to
  /// one of its children.
  using select_rec = std::variant<node, node::pointer_type>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename PriorityQueue>
  inline void
  __select_recurse_out(PriorityQueue &pq, arc_writer &aw,
                       const ptr_uint64 &source,
                       const ptr_uint64 &target)
  {
    if(target.is_terminal()) {
      aw.push_terminal({ source, target });
    } else {
      pq.push({ {target}, {}, {source} });
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename SelectPolicy, typename AssignmentMgr, typename PriorityQueue>
  typename SelectPolicy::__dd_type
  __select(const exec_policy &ep,
               const typename SelectPolicy::dd_type &dd,
               AssignmentMgr &amgr,
               const size_t pq_memory,
               const size_t pq_max_size)
  {
    // Set up outputs
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    out_arcs->max_1level_cut = 0;
    bool output_changes = false;

    // Set up inputs
    node_stream<> ns(dd);

    // Set up priority queue with initial request to the root
    PriorityQueue select_pq({dd}, pq_memory, pq_max_size, stats_select.lpq);
    {
      const node root = ns.peek();
      select_pq.push({{ root.uid() }, {}, { SelectPolicy::pointer_type::nil() } });
    }

    // Process all to-be-visited nodes in topological order
    while(!select_pq.empty()) {
      // Set up next level
      select_pq.setup_next_level();

      const typename SelectPolicy::label_type level = select_pq.current_level();
      typename SelectPolicy::id_type level_size = 0;

      const assignment a = amgr.assignment_for_level(level);

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, select_pq.size());

      // Process entire level
      while (!select_pq.empty_level()) {
        // Seek requested node
        const node n = ns.seek(select_pq.top().target[0]);

        adiar_assert(select_pq.top().target == n.uid());
        adiar_assert(n.uid().label() == level);

        // Process node
        // TODO: Move the switch on `a` into Selection Policy?
        const select_rec rec = a == assignment::False   ? SelectPolicy::fix_false(n, amgr)
                             : a == assignment::True    ? SelectPolicy::fix_true(n, amgr)
                             : /*a == assignment::None:*/ SelectPolicy::keep_node(n, amgr);

        // Output/Forward resulting node
        if(std::holds_alternative<typename SelectPolicy::node_type>(rec)) {
          const node rec_node = std::get<typename SelectPolicy::node_type>(rec);

          output_changes |= rec_node != n;

          // Output/Forward outgoing arcs
          __select_recurse_out(select_pq, aw, n.uid().as_ptr(false), rec_node.low());
          __select_recurse_out(select_pq, aw, n.uid().as_ptr(true),  rec_node.high());

          // Output ingoing arcs
          while(select_pq.can_pull() && select_pq.top().target == rec_node.uid()) {
            const select_request request_pq = select_pq.pull();

            if(!request_pq.data.source.is_nil()) {
              aw.push_internal({ request_pq.data.source, rec_node.uid() });
            }
          }

          level_size++;
        } else { // std::holds_alternative<node::pointer_type>(rec_res)
          const typename SelectPolicy::pointer_type rec_target =
            std::get<typename SelectPolicy::pointer_type>(rec);

          output_changes = true;

          // Output/Forward extension of arc
          while(select_pq.can_pull() && select_pq.top().target == n.uid()) {
            const typename SelectPolicy::pointer_type source = select_pq.pull().data.source;

            if(rec_target.is_terminal() && source.is_nil()) {
              // Edge-case: restriction to a terminal
              return SelectPolicy::terminal(rec_target.value(), amgr);
            }

            __select_recurse_out(select_pq, aw, source, rec_target);
          }
        }
      }

      // Push meta data about this level
      if (level_size > 0) {
        aw.push(level_info(level, level_size));
      }
    }

    if (!output_changes) {
      return dd;
    }

    return typename SelectPolicy::__dd_type(out_arcs, ep);
  }

  template<typename SelectPolicy>
  size_t __select_2level_upper_bound(const typename SelectPolicy::dd_type &dd)
  {
    const safe_size_t max_2level_cut = dd.max_2level_cut(cut::Internal);
    return to_size(max_2level_cut + 2u);
  }

  template<typename SelectPolicy, typename AssignmentMgr>
  typename SelectPolicy::__dd_type
  select(const exec_policy &ep,
             const typename SelectPolicy::dd_type &dd,
             AssignmentMgr &amgr)
  {
    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const tpie::memory_size_type aux_available_memory = memory_available()
      - node_stream<>::memory_usage() - arc_writer::memory_usage();

    const tpie::memory_size_type pq_memory_fits =
      select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(aux_available_memory);

    const bool internal_only = ep.memory_mode() == exec_policy::memory::Internal;
    const bool external_only = ep.memory_mode() == exec_policy::memory::External;

    const size_t pq_bound = __select_2level_upper_bound<SelectPolicy>(dd);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_select.lpq.unbucketed += 1u;
#endif
      return __select<SelectPolicy, AssignmentMgr,
                      select_priority_queue_t<0, memory_mode::Internal>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_select.lpq.internal += 1u;
#endif
      return __select<SelectPolicy, AssignmentMgr,
                      select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_select.lpq.external += 1u;
#endif
      return __select<SelectPolicy, AssignmentMgr,
                      select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_SELECT_H
