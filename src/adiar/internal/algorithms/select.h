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
#include <adiar/internal/data_types/convert.h>
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
  // Priority queue functions
  struct select_arc : public arc
  { // TODO: replace with request class
    select_arc() = default;

    select_arc(const select_arc &) = default;

    select_arc(const arc &a) : arc(a)
    { }

    select_arc& operator= (const select_arc &a) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The level at which this nodes target belongs to.
    ////////////////////////////////////////////////////////////////////////////
    arc::label_type level() const
    { return target().label(); }
  };

  template<size_t look_ahead, memory_mode mem_mode>
  using select_priority_queue_t =
    levelized_node_priority_queue<select_arc, arc_target_lt,
                                  look_ahead,
                                  mem_mode>;

  struct select_rec_output { node out; };
  struct select_rec_skipto { ptr_uint64 child; };

  using select_rec = std::variant<select_rec_output, select_rec_skipto>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename select_policy, typename select_assignment_mgr>
  inline select_rec select_apply_assignment(const assignment &a,
                                                    const node &n,
                                                    select_assignment_mgr &amgr)
  {
    switch (a) {
    case assignment::False:
      return select_policy::fix_false(n, amgr);
    case assignment::True:
      return select_policy::fix_true(n, amgr);
    default:
    case assignment::None:
      return select_policy::keep_node(n, amgr);
    }
  }

  template<typename pq_t>
  inline void __select_resolve_request(const arc &request,
                                           pq_t &pq,
                                           arc_writer &aw)
  {
    if(request.target().is_terminal()) {
      aw.push_terminal(request);
    } else {
      pq.push(request);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename select_policy, typename select_assignment_mgr, typename pq_t>
  typename select_policy::__dd_type
  __select(const exec_policy &ep,
               const typename select_policy::dd_type &dd,
               select_assignment_mgr &amgr,
               const size_t pq_memory,
               const size_t pq_max_size)
  {
    // Set up to run the substitution algorithm
    node_stream<> ns(dd);
    node n = ns.pull();

    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    out_arcs->max_1level_cut = 0;

    pq_t select_pq({dd}, pq_memory, pq_max_size, stats_select.lpq);

    bool output_changes = false;

    // process the root and create initial recursion requests
    {
      const typename select_policy::label_type level = n.label();
      assignment a = amgr.assignment_for_level(level);

      const select_rec rec_res = select_apply_assignment<select_policy>(a, n, amgr);

      if (std::holds_alternative<select_rec_output>(rec_res)) {
        const node n_res = std::get<select_rec_output>(rec_res).out;

        output_changes |= n_res != n;

        __select_resolve_request(low_arc_of(n_res), select_pq, aw);
        __select_resolve_request(high_arc_of(n_res), select_pq, aw);

        aw.push(level_info(level, 1));
      } else { // std::holds_alternative<select_rec_skipto>(n_res)
        output_changes = true;

        const ptr_uint64 rec_child = std::get<select_rec_skipto>(rec_res).child;;

        if(rec_child.is_terminal()) {
          return select_policy::terminal(rec_child.value(), amgr);
        }

        select_pq.push(arc(ptr_uint64::nil(), rec_child));
      }
    }

    // process all to-be-visited nodes in topological order
    while(!select_pq.empty()) {
      // Set up next level
      select_pq.setup_next_level();

      const typename select_policy::label_type level = select_pq.current_level();
      typename select_policy::id_type level_size = 0;

      const assignment a = amgr.assignment_for_level(level);

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, select_pq.size());

      // Process entire level
      while (!select_pq.empty_level()) {
        // seek requested node
        while (n.uid() < select_pq.top().target()) {
          n = ns.pull();
        }

        // process node and forward information
        const select_rec rec_res = select_apply_assignment<select_policy>(a, n, amgr);

        if(std::holds_alternative<select_rec_output>(rec_res)) {
          const node n_res = std::get<select_rec_output>(rec_res).out;
          output_changes |= n_res != n;

          // outgoing arcs
          __select_resolve_request(low_arc_of(n_res), select_pq, aw);
          __select_resolve_request(high_arc_of(n_res), select_pq, aw);

          // Ingoing arcs
          while(select_pq.can_pull() && select_pq.top().target() == n_res.uid()) {
            const arc parent_arc = select_pq.pull();

            if(!parent_arc.source().is_nil()) {
              aw.push_internal(parent_arc);
            }
          }

          level_size++;
        } else { // std::holds_alternative<select_rec_skipto>(rec_res)
          output_changes = true;

          const ptr_uint64 rec_child = std::get<select_rec_skipto>(rec_res).child;

          while(select_pq.can_pull() && select_pq.top().target() == n.uid()) {
            const arc parent_arc = select_pq.pull();
            const arc request = { parent_arc.source(), rec_child };

            if(rec_child.is_terminal() && parent_arc.source().is_nil()) {
              // we have restricted ourselves to a terminal
              return select_policy::terminal(rec_child.value(), amgr);
            }

            __select_resolve_request(request, select_pq, aw);
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

    return typename select_policy::__dd_type(out_arcs, ep);
  }

  template<typename select_policy>
  size_t __select_2level_upper_bound(const typename select_policy::dd_type &dd)
  {
    const safe_size_t max_2level_cut = dd.max_2level_cut(cut::Internal);
    return to_size(max_2level_cut + 2u);
  }

  template<typename select_policy, typename select_assignment_mgr>
  typename select_policy::__dd_type
  select(const exec_policy &ep,
             const typename select_policy::dd_type &dd,
             select_assignment_mgr &amgr)
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

    const size_t pq_bound = __select_2level_upper_bound<select_policy>(dd);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_select.lpq.unbucketed += 1u;
#endif
      return __select<select_policy, select_assignment_mgr,
                          select_priority_queue_t<0, memory_mode::Internal>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_select.lpq.internal += 1u;
#endif
      return __select<select_policy, select_assignment_mgr,
                          select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_select.lpq.external += 1u;
#endif
      return __select<select_policy, select_assignment_mgr,
                          select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>
        (ep, dd, amgr, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_SELECT_H
