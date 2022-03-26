#ifndef ADIAR_INTERNAL_INTERCUT_H
#define ADIAR_INTERNAL_INTERCUT_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/levelized_priority_queue.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::intercut_t stats_intercut;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queues

  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using intercut_priority_queue_1_t =
    levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt,
                                  sorter_template, priority_queue_template,
                                  1u,
                                  0u>;

  struct arc_cut : arc
  {
    label_t cut_at;
  };

  struct arc_cut_label
  {
    static label_t label_of(const arc_cut& a)
    {
      return a.cut_at;
    }
  };

  struct arc_cut_lt
  {
    bool operator()(const arc_cut &a, const arc_cut &b)
    {
      return a.cut_at < b.cut_at
        || (a.cut_at == b.cut_at && a.target < b.target)
#ifndef NDEBUG
        || (a.cut_at == b.cut_at && a.target == b.target && a.source < b.source)
#endif
           ;
    }
  };

  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using intercut_priority_queue_2_t =
    levelized_label_priority_queue<arc_cut, arc_cut_label, arc_cut_lt,
                                   sorter_template, priority_queue_template,
                                   1u,
                                   0u>;

  //////////////////////////////////////////////////////////////////////////////

  struct intercut_rec_output
  {
    ptr_t low;
    ptr_t high;
  };

  struct intercut_rec_skipto
  {
    ptr_t tgt;
  };

  typedef std::variant<intercut_rec_output, intercut_rec_skipto> intercut_rec;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename intercut_policy>
  bool cut_sink(const label_t curr_level, const label_t cut_level, const bool sink_value)
  {
    return curr_level < cut_level
      && (!sink_value || intercut_policy::cut_true_sink)
      && (sink_value || intercut_policy::cut_false_sink);
  }

  template<typename intercut_policy>
  class intercut_out__pq
  {
  public:
    static constexpr bool ignore_nil = false;

    template<typename pq_1_t, typename pq_2_t>
    static inline void forward(arc_writer &aw,
                               pq_1_t &pq_1,
                               pq_2_t &pq_2,
                               const ptr_t source, const ptr_t target,
                               const label_t curr_level, const label_t next_cut)
    {
      if (is_sink(target)) {
        if (!cut_sink<intercut_policy>(curr_level, next_cut, value_of(target))) {
          aw.unsafe_push_sink({ source, target });
        } else {
          pq_2.push({ source, target, next_cut });
        }
      } else { // is_node(target)
        if (curr_level < next_cut && next_cut < label_of(target)) {
          pq_2.push({ source, target, next_cut });
        } else {
          pq_1.push({ source, target });
        }
      }
    }
  };

  class intercut_out__writer
  {
  public:
    static constexpr bool ignore_nil = true;

    template<typename pq_1_t, typename pq_2_t>
    static inline void forward(arc_writer &aw,
                               pq_1_t &/*pq_1*/,
                               pq_2_t &/*pq_2*/,
                               const ptr_t source, const ptr_t target,
                               const label_t /*curr_level*/, const label_t /*next_cut*/)
    {
      aw.unsafe_push_node({ source, target });
    }
  };

  template<typename intercut_policy, typename in_policy,
           typename pq_1_t, typename pq_2_t>
  inline void intercut_in__pq_1(arc_writer &aw,
                                pq_1_t &pq_1,
                                pq_2_t &pq_2,
                                const label_t out_label,
                                const ptr_t pq_target,
                                const ptr_t out_target,
                                const label_t l)
  {
    adiar_debug(out_label <= label_of(out_target),
                "should forward/output a node on this level or ahead.");

    while (pq_1.can_pull() && pq_1.top().target == pq_target) {
      const arc_t parent = pq_1.pull();

      if (in_policy::ignore_nil && is_nil(parent.source)) { continue; }
      in_policy::forward(aw, pq_1, pq_2, parent.source, out_target, out_label, l);
    }
  }

  template<typename intercut_policy, typename in_policy,
           typename pq_1_t, typename pq_2_t>
  inline void intercut_in__pq_2(arc_writer &aw,
                                pq_1_t &pq_1,
                                pq_2_t &pq_2,
                                const label_t out_label,
                                const ptr_t pq_target,
                                const ptr_t out_target,
                                const label_t l)
  {
    adiar_debug(out_label <= label_of(out_target),
                "should forward/output a node on this level or ahead.");

    while (pq_2.can_pull()
           && pq_2.top().cut_at == out_label
           && pq_2.top().target == pq_target) {
      const arc_cut parent = pq_2.top();
      pq_2.pop();

      if (in_policy::ignore_nil && is_nil(parent.source)) { continue; }
      in_policy::forward(aw, pq_1, pq_2, parent.source, out_target, out_label, l);
    }
  }

  template<typename intercut_policy, typename pq_1_t, typename pq_2_t>
  typename intercut_policy::unreduced_t __intercut (const typename intercut_policy::reduced_t &dd,
                                                    const label_file &labels,
                                                    const size_t pq_1_memory, const size_t pq_2_memory,
                                                    const size_t max_pq_1_size, const size_t max_pq_2_size)
  {
    node_stream<> in_nodes(dd);
    node_t n = in_nodes.pull();

    if (is_sink(n)) {
      return intercut_policy::on_sink_input(value_of(n), dd, labels);
    }

    label_stream<> ls(labels);
    label_t l = ls.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_1_t intercut_pq_1({dd}, pq_1_memory, max_pq_1_size);
    pq_2_t intercut_pq_2({labels}, pq_2_memory, max_pq_2_size);

    // Add request for root in the relevant queue
    label_t out_label = std::min(l, label_of(n));
    id_t out_id = 0;

    if (l < label_of(n)) {
      intercut_pq_2.push({ NIL, n.uid, out_label });
    } else {
      intercut_pq_1.push({ NIL, n.uid });
    }

    size_t max_1level_cut = 0;

    // Process nodes of the decision diagram in topological order
    while (!intercut_pq_1.empty() || !intercut_pq_2.empty()) {
      if (out_id > 0) { aw.unsafe_push(create_level_info(out_label, out_id)); }

      out_label = std::min(intercut_pq_1.has_next_level() ? intercut_pq_1.next_level() : MAX_LABEL,
                           intercut_pq_2.has_next_level() ? intercut_pq_2.next_level() : MAX_LABEL);

      if (intercut_pq_1.has_next_level()) {
        intercut_pq_1.setup_next_level(out_label);
      }
      if (intercut_pq_2.has_next_level()) {
        intercut_pq_2.setup_next_level(out_label);
      }

      max_1level_cut = std::max(max_1level_cut, intercut_pq_1.size() + intercut_pq_2.size());

      adiar_debug(intercut_pq_1.can_pull() || intercut_pq_2.can_pull(),
                  "should be at a level of at least one of the two queues.");

      out_id = 0;

      const bool hit_level = out_label == l;

      // Forward to next label to cut on after this level
      while (ls.can_pull() && l <= out_label) { l = ls.pull(); }

      // Resolve requests in intercut_pq_1 for this level
      while (intercut_pq_1.can_pull()) {
        adiar_invariant(intercut_pq_1.current_level() == out_label,
                        "first priority queue should be at current level");

        while (n.uid < intercut_pq_1.top().target) { n = in_nodes.pull(); }
        adiar_debug(n.uid == intercut_pq_1.top().target, "should always find desired node");

        const intercut_rec r = hit_level
          ? intercut_policy::hit_existing(n)
          : intercut_policy::miss_existing(n);

        if (intercut_policy::may_skip && std::holds_alternative<intercut_rec_skipto>(r)) {
          const intercut_rec_skipto rs = std::get<intercut_rec_skipto>(r);

          if (is_sink(rs.tgt)
              && is_nil(intercut_pq_1.top().source)
              && !cut_sink<intercut_policy>(out_label, l, value_of(rs.tgt))) {
            return intercut_policy::sink(value_of(rs.tgt));
          }
          // TODO: The 'is_sink(rs.tgt) && cut_sink(...)' case can be handled even
          //       better with 'intercut_policy::on_sink_input' but where the
          //       label file are only of the remaining labels.

          intercut_in__pq_1<intercut_policy, intercut_out__pq<intercut_policy>>
            (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, rs.tgt, l);
        } else {
          const intercut_rec_output ro = std::get<intercut_rec_output>(r);
          const uid_t out_uid = create_node_uid(out_label, out_id++);

          intercut_out__pq<intercut_policy>::forward
            (aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);

          intercut_out__pq<intercut_policy>::forward
            (aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

          intercut_in__pq_1<intercut_policy, intercut_out__writer>
            (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, out_uid, l);
        }
      }

      // Resolve requests in intercut_pq_2 to cut arcs on this level
      while (intercut_pq_2.can_pull()) {
        adiar_invariant(out_label <= l,
                        "the last iteration with this queue is for the very last label to cut on");

        adiar_invariant(intercut_pq_2.current_level() == out_label
                        && intercut_pq_2.top().cut_at == out_label,
                        "second priority queue should be at current level");

        const arc_cut request = intercut_pq_2.top();
        const intercut_rec_output ro = intercut_policy::hit_cut(request.target);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        intercut_out__pq<intercut_policy>::forward
          (aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);

        intercut_out__pq<intercut_policy>::forward
          (aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

        intercut_in__pq_2<intercut_policy, intercut_out__writer>
          (aw, intercut_pq_1, intercut_pq_2, out_label, request.target, out_uid, l);
      }
    }

    // Push the level of the very last iteration
    if (out_id > 0) {
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    out_arcs._file_ptr->max_1level_cut = max_1level_cut;
    return out_arcs;
  }

  template<typename intercut_policy>
  size_t __intercut_size_based_upper_bound_1(const typename intercut_policy::reduced_t &dd)
  {
    return 2 * dd.file_ptr()->size() + 2;
  }

  template<typename intercut_policy>
  size_t __intercut_size_based_upper_bound_2(const typename intercut_policy::reduced_t &dd)
  {
    // Can the size_bound computation overflow?
    const size_t number_of_nodes = dd.file_ptr()->size();
    const bits_approximation input_bits(number_of_nodes);

    const bits_approximation bound_bits = (input_bits * 2) + 2;

    if(bound_bits.may_overflow()) {
      return std::numeric_limits<size_t>::max();
    } else {
      return (2 * number_of_nodes) + 2;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // TODO: If nothing changes (no new nodes are added and the current are left
  //       as-is) then one can return the input once more.

  template<typename intercut_policy>
  typename intercut_policy::unreduced_t intercut(const typename intercut_policy::reduced_t &dd,
                                                 const label_file &labels)
  {
    if (labels.size() == 0) {
      return intercut_policy::on_empty_labels(dd);
    }

    // Derive an upper bound on the size of auxiliary data structures and check
    // whether we can run them with a faster internal memory variant.
    const tpie::memory_size_type aux_available_memory = tpie::get_memory_manager().available()
      // Input stream
      - node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    const size_t size_bound_1 = __intercut_size_based_upper_bound_1<intercut_policy>(dd);
    const size_t size_bound_2 = __intercut_size_based_upper_bound_2<intercut_policy>(dd);

    constexpr size_t data_structures_in_lpq_1 =
      intercut_priority_queue_1_t<internal_sorter, internal_priority_queue>::BUCKETS + 1;

    constexpr size_t data_structures_in_lpq_2 =
      intercut_priority_queue_2_t<internal_sorter, internal_priority_queue>::BUCKETS + 1;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_lpq_1 + data_structures_in_lpq_2)) * data_structures_in_lpq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_1_memory_fits =
      intercut_priority_queue_1_t<internal_sorter, internal_priority_queue>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      intercut_priority_queue_2_t<internal_sorter, internal_priority_queue>::memory_fits(pq_2_internal_memory);

    if(size_bound_1 <= pq_1_memory_fits && size_bound_2 <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_intercut.lpq_internal++;
#endif
      return __intercut<intercut_policy,
                        intercut_priority_queue_1_t<external_sorter, external_priority_queue>,
                        intercut_priority_queue_2_t<external_sorter, external_priority_queue>>
        (dd, labels, pq_1_internal_memory, pq_2_internal_memory, size_bound_1, size_bound_2);
    } else {
#ifdef ADIAR_STATS
      stats_intercut.lpq_external++;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = aux_available_memory - pq_1_memory;

      return __intercut<intercut_policy,
                        intercut_priority_queue_1_t<external_sorter, external_priority_queue>,
                        intercut_priority_queue_2_t<external_sorter, external_priority_queue>>
        (dd, labels, pq_1_memory, pq_2_memory, size_bound_1, size_bound_2);
    }
  }
}

#endif // ADIAR_INTERNAL_INTERCUT_H
