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
        || (a.cut_at == b.cut_at && a.target < b.target);
    }
  };

  // TODO: Should this be a levelized_priority queue? This only makes sense, if
  // the end-user is changing EVERY level in the decision diagram. If we made it
  // into one, then it probably needs way larger a lookahead than just one
  // level.
  typedef tpie::priority_queue<arc_cut, arc_cut_lt>
  intercut_priority_queue_2_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions

  inline void intercut_forward_edge(arc_writer &aw,
                                    intercut_priority_queue_1_t &pq_1,
                                    intercut_priority_queue_2_t &pq_2,
                                    const ptr_t source, const ptr_t target,
                                    const label_t curr_level, const label_t next_cut)
  {
    if (is_sink(target)) {
      // TODO: allow policy to chose what sink values to cut on and which not to.
      if (!value_of(target) || next_cut <= curr_level) {
        aw.unsafe_push_sink({ source, target });
      } else { // value_of(target) && next_cut > curr_level
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

  //////////////////////////////////////////////////////////////////////////////

  struct intercut_rec_output {
    ptr_t low;
    ptr_t high;
  };

  struct intercut_rec_skipto {
    ptr_t tgt;
  };

  typedef std::variant<intercut_rec_output, intercut_rec_skipto> intercut_rec;

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

    // We need to insert nodes before the root. The levelized priority queue
    // does not support pushing to the level of the root, so we'll manually
    // deal with these.
    while (l < label_of(n)) {
      out_label = l;
      const uid_t out_uid = create_node_uid(out_label, 0);

      const bool more_labels = ls.can_pull();
      if (more_labels) { l = ls.pull(); }

      const intercut_rec_output r = intercut_policy::hit_cut(n.uid);

      // Cut the edge, if need be
      const label_t next_label = more_labels ? std::min(label_of(n), l) : label_of(n);
      const ptr_t next_node = create_node_ptr(next_label, 0);

      aw.unsafe_push(arc { out_uid, is_sink(r.low) ? r.low : next_node });
      aw.unsafe_push(arc { flag(out_uid), is_sink(r.high) ? r.high : next_node });
      aw.unsafe_push(create_level_info(out_label, 1));

      // Already done with all labels?
      if (out_label == l) { break; }
    }

    out_label = label_of(n);

    if (l == out_label) {
      // Let l be the next label to hit (if any)
      if (ls.can_pull()) { l = ls.pull(); }

      intercut_rec r = intercut_policy::hit_existing(n);

      if (std::holds_alternative<intercut_rec_skipto>(r)) {
        const intercut_rec_skipto rs = std::get<intercut_rec_skipto>(r);

        // We don't need to check the sink value here, since rs.tgt is the old
        // 'high' value (and hence cannot be false).
        if (is_sink(rs.tgt)) { return intercut_policy::sink(value_of(rs.tgt)); }

        intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, NIL, rs.tgt, out_label, l);
      } else { // std::holds_alternative<intercut_rec_output>(n)
        const intercut_rec_output ro = std::get<intercut_rec_output>(r);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);
        intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);
      }
    } else { // l > label_of(n)
      const intercut_rec_output ro = intercut_policy::miss_existing(n);
      const uid_t out_uid = create_node_uid(out_label, out_id++);

      intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);
      intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);
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

          // Will the hit kill the node?
          if (is_sink(rs.tgt) && is_nil(intercut_pq_1.top().source)) {
            return intercut_policy::sink(value_of(rs.tgt));
          }

          while (intercut_pq_1.can_pull() && intercut_pq_1.top().target == n.uid) {
            intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2,
                                    intercut_pq_1.pull().source, rs.tgt,
                                    out_label, l);
          }
        } else {
          const intercut_rec_output ro = std::get<intercut_rec_output>(r);
          const uid_t out_uid = create_node_uid(out_label, out_id++);

          intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);
          intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

          // Output ingoing arcs
          while (intercut_pq_1.can_pull() && intercut_pq_1.top().target == n.uid) {
            const arc_t parent_arc = intercut_pq_1.pull();
            if (!is_nil(parent_arc.source)) {
              aw.unsafe_push_node({ parent_arc.source, out_uid });
            }
          }
        }
      }

      // Resolve requests in intercut_pq_2 to cut arcs on this level
      while (!intercut_pq_2.empty() && intercut_pq_2.top().cut_at == out_label) {
        adiar_invariant(out_label <= l,
                        "the last iteration with this queue is for the very last label to cut on");

        const arc_cut request = intercut_pq_2.top();
        const intercut_rec_output ro = intercut_policy::hit_cut(request.target);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, out_uid, ro.low, out_label, l);
        intercut_forward_edge(aw, intercut_pq_1, intercut_pq_2, flag(out_uid), ro.high, out_label, l);

        // Output ingoing arcs
        while (!intercut_pq_2.empty()
               && intercut_pq_2.top().cut_at == out_label
               && intercut_pq_2.top().target == request.target) {
          const arc_cut parent = intercut_pq_2.top();
          intercut_pq_2.pop();

          if (!is_nil(parent.source)) {
            aw.unsafe_push_node({ parent.source, out_uid });
          }
        }
      }
    }

    // Push the level of the very last iteration
    if (out_id > 0) {
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    return out_arcs;
  }
}

#endif // ADIAR_INTERNAL_INTERCUT_H
