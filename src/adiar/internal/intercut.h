#ifndef ADIAR_INTERNAL_INTERCUT_H
#define ADIAR_INTERNAL_INTERCUT_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/levelized_priority_queue.h>

#include <adiar/assert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queues

  typedef levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt>
  intercut_priority_queue_1_t;

  struct arc_cut : arc
  {
    label_t cut_at;
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

  // TODO: Should this be a levelized_priority queue? This only makes sense, if
  // the end-user is changing EVERY level in the decision diagram. If we made it
  // into one, then it probably needs way larger a lookahead than just one
  // level.
  typedef tpie::priority_queue<arc_cut, arc_cut_lt>
  intercut_priority_queue_2_t;

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
  class intercut_out__pre_root
  {
  public:
    static constexpr bool ignore_nil = false;

    static inline void forward(arc_writer &aw,
                               intercut_priority_queue_1_t &/*pq_1*/,
                               intercut_priority_queue_2_t &pq_2,
                               const ptr_t source, const ptr_t target,
                               const label_t curr_level, const label_t next_cut)
    {
      if (is_sink(target)
          && !cut_sink<intercut_policy>(curr_level, next_cut, value_of(target))) {
        aw.unsafe_push_sink({ source, target });
      } else {
        const label_t __next_cut = (next_cut <= curr_level || next_cut > label_of(target))
          ? label_of(target) : next_cut;

        pq_2.push({ source, target, __next_cut });
      }
    }
  };

  template<typename intercut_policy>
  class intercut_out__post_root
  {
  public:
    static constexpr bool ignore_nil = false;

    static inline void forward(arc_writer &aw,
                               intercut_priority_queue_1_t &pq_1,
                               intercut_priority_queue_2_t &pq_2,
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

  class intercut_out__arc_writer
  {
  public:
    static constexpr bool ignore_nil = true;

    static inline void forward(arc_writer &aw,
                               intercut_priority_queue_1_t &/*pq_1*/,
                               intercut_priority_queue_2_t &/*pq_2*/,
                               const ptr_t source, const ptr_t target,
                               const label_t /*curr_level*/, const label_t /*next_cut*/)
    {
      aw.unsafe_push_node({ source, target });
    }
  };

  template<typename intercut_policy, typename in_policy>
  inline void intercut_in__pq_1(arc_writer &aw,
                                intercut_priority_queue_1_t &pq_1,
                                intercut_priority_queue_2_t &pq_2,
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

  template<typename intercut_policy, typename in_policy>
  inline void intercut_in__pq_2(arc_writer &aw,
                                intercut_priority_queue_1_t &pq_1,
                                intercut_priority_queue_2_t &pq_2,
                                const label_t out_label,
                                const ptr_t pq_target,
                                const ptr_t out_target,
                                const label_t l)
  {
    adiar_debug(out_label <= label_of(out_target),
                "should forward/output a node on this level or ahead.");

    while (!pq_2.empty()
           && pq_2.top().cut_at == out_label
           && pq_2.top().target == pq_target) {
      const arc_cut parent = pq_2.top();
      pq_2.pop();

      if (in_policy::ignore_nil && is_nil(parent.source)) { continue; }
      in_policy::forward(aw, pq_1, pq_2, parent.source, out_target, out_label, l);
    }
  }

  template<typename intercut_policy, typename out_policy>
  inline void intercut_resolve_hit_arcs(arc_writer &aw,
                                        intercut_priority_queue_1_t &pq_1,
                                        intercut_priority_queue_2_t &pq_2,
                                        const label_t out_label, size_t &out_id,
                                        const label_t l)
  {
    while (!pq_2.empty() && pq_2.top().cut_at == out_label) {
      adiar_invariant(out_label <= l,
                      "the last iteration with this queue is for the very last label to cut on");

      const arc_cut request = pq_2.top();
      const intercut_rec_output ro = intercut_policy::hit_cut(request.target);
      const uid_t out_uid = create_node_uid(out_label, out_id++);

      out_policy::forward
        (aw, pq_1, pq_2, out_uid, ro.low, out_label, l);

      out_policy::forward
        (aw, pq_1, pq_2, flag(out_uid), ro.high, out_label, l);

      intercut_in__pq_2<intercut_policy, intercut_out__arc_writer>
        (aw, pq_1, pq_2, out_label, request.target, out_uid, l);
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  template<typename intercut_policy>
  typename intercut_policy::unreduced_t intercut(const typename intercut_policy::reduced_t &dd,
                                                 const label_file &labels)
  {
    if (labels.size() == 0) {
      return dd;
    }

    node_stream<> in_nodes(dd);
    node_t n = in_nodes.pull();

    if (is_sink(n)) {
      return intercut_policy::on_sink_input(value_of(n), dd, labels);
    }

    label_stream<> ls(labels);
    label_t l = ls.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    intercut_priority_queue_1_t intercut_pq_1({dd}, available_memory / 2);
    intercut_priority_queue_2_t intercut_pq_2(available_memory / 2);

    // Process root and create initial recursion requests
    label_t out_label;
    id_t out_id = 0;

    // The levelized_priority_queue does not do well with pushing to a level
    // above or to the root level. Yet, we need to insert nodes before the root,
    // i.e. cutting the in-going arc to the root. So, we will deal with them
    // ourselves (using the secondary priority queue exclusively)

    intercut_pq_2.push({ NIL, n.uid, std::min(l, label_of(n)) });

    while (l < label_of(n)) {
      out_label = l;
      out_id = 0;

      const bool more_labels = ls.can_pull();
      if (more_labels) { l = ls.pull(); }

      intercut_resolve_hit_arcs<intercut_policy, intercut_out__pre_root<intercut_policy>>
        (aw, intercut_pq_1, intercut_pq_2, out_label, out_id, l);

      if (out_id > 0) { aw.unsafe_push(create_level_info(out_label, out_id)); }

      adiar_invariant(!intercut_pq_2.empty(),
                      "intercut_pq_2 should never be empty (at least root request)");

      // Already done with all labels?
      if (!more_labels) { break; }
    }

    adiar_debug(intercut_pq_2.top().target == n.uid && intercut_pq_2.top().cut_at == label_of(n),
                "The top-most request should be for the root");

    out_label = label_of(n);
    out_id = 0;

    { // Deal with the root
      const bool hit_level = l == out_label;

      // Let l be the next label to hit (if any)
      if (hit_level && ls.can_pull()) { l = ls.pull(); }

      intercut_rec r = hit_level
        ? intercut_policy::hit_existing(n)
        : intercut_policy::miss_existing(n);;

      if (std::holds_alternative<intercut_rec_skipto>(r)) {
        const intercut_rec_skipto rs = std::get<intercut_rec_skipto>(r);

        // TODO: nil value on source?
        if (is_sink(rs.tgt) && !cut_sink<intercut_policy>(out_label, l, value_of(rs.tgt))) {
          return intercut_policy::sink(value_of(rs.tgt));
        }
        // TODO: The 'is_sink(rs.tgt) && cut_sink(...)' case can be handled even
        //       better with 'intercut_policy::on_sink_input' but where the
        //       label file are only of the remaining labels.

        intercut_in__pq_2<intercut_policy, intercut_out__post_root<intercut_policy>>
          (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, rs.tgt, l);

      } else { // std::holds_alternative<intercut_rec_output>(n)
        const intercut_rec_output ro = std::get<intercut_rec_output>(r);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        intercut_out__post_root<intercut_policy>::forward
          (aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);

        intercut_out__post_root<intercut_policy>::forward
          (aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

        intercut_in__pq_2<intercut_policy, intercut_out__arc_writer>
          (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, out_uid, l);
      }

      intercut_resolve_hit_arcs<intercut_policy, intercut_out__post_root<intercut_policy>>
        (aw, intercut_pq_1, intercut_pq_2, out_label, out_id, l);
    }

    // Process nodes of the decision diagram in topological order
    while (intercut_pq_1.has_next_level() || !intercut_pq_2.empty()) {
      if (out_id > 0) { aw.unsafe_push(create_level_info(out_label, out_id)); }

      if (intercut_pq_1.has_next_level()) {
        if (!intercut_pq_2.empty()) {
          intercut_pq_1.setup_next_level(intercut_pq_2.top().cut_at);
        } else {
          intercut_pq_1.setup_next_level();
        }
        out_label = intercut_pq_1.current_level();
      } else {
        out_label = intercut_pq_2.top().cut_at;
      }

      out_id = 0;

      const bool hit_level = out_label == l;

      // Forward to next label to cut on after this level
      while (ls.can_pull() && l <= out_label) { l = ls.pull(); }

      // Resolve requests in intercut_pq_1 for this level
      while (intercut_pq_1.can_pull()) {
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

          intercut_in__pq_1<intercut_policy, intercut_out__post_root<intercut_policy>>
            (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, rs.tgt, l);
        } else {
          const intercut_rec_output ro = std::get<intercut_rec_output>(r);
          const uid_t out_uid = create_node_uid(out_label, out_id++);

          intercut_out__post_root<intercut_policy>::forward
            (aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);

          intercut_out__post_root<intercut_policy>::forward
            (aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

          intercut_in__pq_1<intercut_policy, intercut_out__arc_writer>
            (aw, intercut_pq_1, intercut_pq_2, out_label, n.uid, out_uid, l);
        }
      }

      // Resolve requests in intercut_pq_2 to cut arcs on this level
      intercut_resolve_hit_arcs<intercut_policy, intercut_out__post_root<intercut_policy>>
        (aw, intercut_pq_1, intercut_pq_2, out_label, out_id, l);
    }

    // Push the level of the very last iteration
    if (out_id > 0) {
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    return out_arcs;
  }
}

#endif // ADIAR_INTERNAL_INTERCUT_H
