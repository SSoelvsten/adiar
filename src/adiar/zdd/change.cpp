#include "change.h"

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/levelized_priority_queue.h>

#include <adiar/zdd/zdd.h>
#include <adiar/zdd/build.h>

#include <adiar/assert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Priority queues

  typedef levelized_node_priority_queue<arc_t, arc_target_label, arc_target_lt>
  change_priority_queue_1_t;

  struct change_cut : arc
  {
    label_t cut_at;
  };

  struct change_cut_queue_lt
  {
    bool operator()(const change_cut &a, const change_cut &b)
    {
      return a.cut_at < b.cut_at
        || (a.cut_at == b.cut_at && a.target < b.target);
    }
  };

  // TODO: Should this be a levelized_priority queue? This only makes sense, if
  // the end-user is changing EVERY level in the ZDD. If we made it into one,
  // then it probably needs way larger a lookahead than just one level.
  typedef tpie::priority_queue<change_cut, change_cut_queue_lt>
  change_priority_queue_2_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions

  zdd zdd_change_true_chain(const label_file &labels)
  {
    label_stream<true> ls(labels);

    const ptr_t low = create_sink_ptr(false);
    ptr_t high = create_sink_ptr(true);

    node_file out;
    node_writer nw(out);

    while (ls.can_pull()) {
      const uid_t uid = create_node_uid(ls.pull(), MAX_ID);
      nw << create_node(uid, low, high);
      high = uid;
    }
    return out;
  }

  void zdd_change_forward_edge(arc_writer &aw,
                               change_priority_queue_1_t &pq_1,
                               change_priority_queue_2_t &pq_2,
                               const ptr_t source, const ptr_t target,
                               const label_t curr_level, const label_t next_cut)
  {
    if (is_sink(target)) {
      if (!value_of(target) || next_cut <= curr_level) {
        // TODO: what is HERE the difference between curr_level and label_of(source)?
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

  struct change_rec_output {
    ptr_t low;
    ptr_t high;
  };

  struct change_rec_skipto {
    ptr_t tgt;
  };

  typedef std::variant<change_rec_output, change_rec_skipto> change_rec;

  change_rec zdd_change_flip_existing(const node_t &n)
  {
    if (is_sink(n.low) && !value_of(n.low)) {
      return change_rec_skipto { n.high };
    }

    return change_rec_output { n.high, n.low };
  }

  change_rec_output zdd_change_flip_cut(const ptr_t target)
  {
    return change_rec_output { create_sink_ptr(false), target };
  }

  change_rec_output zdd_change_keep_existing(const node_t &n)
  {
    return change_rec_output { n.low, n.high };
  }

  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_change(const zdd &dd, const label_file &labels)
  {
    if (labels.size() == 0) {
      return dd;
    }

    node_stream<> in_nodes(dd);
    node_t n = in_nodes.pull();

    if (is_sink(n)) {
      return value_of(n) ? zdd_change_true_chain(labels) : dd;
    }

    label_stream<> ls(labels);
    label_t l = ls.pull();

    arc_file out_arcs;
    arc_writer aw(out_arcs);

    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    change_priority_queue_1_t change_pq_1({dd}, available_memory / 2);
    change_priority_queue_2_t change_pq_2(available_memory / 2);

    // Process root and create initial recursion requests
    label_t out_label = MAX_LABEL+1;
    id_t out_id = 0;

    while (l < label_of(n)) {
      // We need to insert nodes before the root. The levelized priority queue
      // does not support pushing to the level of the root, so we'll manually
      // deal with these.

      // Already done with all labels?
      if (out_label == l) { break; }

      // TODO: Generalise
      out_label = l;
      const uid_t out_uid = create_node_uid(out_label, 0);

      const bool more_labels = ls.can_pull();
      if (more_labels) { l = ls.pull(); }

      const label_t next_label = more_labels ? std::min(label_of(n), l) : label_of(n);

      aw.unsafe_push_sink({ out_uid, create_sink_ptr(false) });
      aw.unsafe_push_node({ flag(out_uid), create_node_uid(next_label, 0) });
      aw.unsafe_push(create_meta(out_label, 1));
    }

    out_label = label_of(n);

    if (l == label_of(n)) {
      // Let l be the next label to flip (if any)
      if (ls.can_pull()) { l = ls.pull(); }

      change_rec r = zdd_change_flip_existing(n);

      if (std::holds_alternative<change_rec_skipto>(r)) {
        const change_rec_skipto rs = std::get<change_rec_skipto>(r);

        // We don't need to check the sink value here, since rs.tgt is the old
        // 'high' value (and hence cannot be false).
        if (is_sink(rs.tgt)) { return zdd_sink(value_of(rs.tgt)); }

        zdd_change_forward_edge(aw, change_pq_1, change_pq_2, NIL, rs.tgt, out_label, l);
      } else { // std::holds_alternative<change_rec_output>(n)
        const change_rec_output ro = std::get<change_rec_output>(r);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        zdd_change_forward_edge(aw, change_pq_1, change_pq_2, out_uid, ro.low, out_label, l);
        zdd_change_forward_edge(aw, change_pq_1, change_pq_2, flag(out_uid), ro.high, out_label, l);
      }
    } else { // l > label_of(n)
      const change_rec_output ro = zdd_change_keep_existing(n);
      const uid_t out_uid = create_node_uid(out_label, out_id++);

      zdd_change_forward_edge(aw, change_pq_1, change_pq_2, out_uid, ro.low, out_label, l);
      zdd_change_forward_edge(aw, change_pq_1, change_pq_2, flag(out_uid), ro.high, out_label, l);
    }

    // Process nodes of ZDD in topological order
    while (change_pq_1.has_next_level() || !change_pq_2.empty()) {
      if (out_id > 0) { aw.unsafe_push(create_meta(out_label, out_id)); }

      if (change_pq_1.has_next_level()) {
        if (!change_pq_2.empty()) {
          change_pq_1.setup_next_level(change_pq_2.top().cut_at);
        } else {
          change_pq_1.setup_next_level();
        }
        out_label = change_pq_1.current_level();
      } else {
        out_label = change_pq_2.top().cut_at;
      }

      out_id = 0;

      const bool change_level = out_label == l;

      // Forward to next label to cut on after this level
      while (ls.can_pull() && l <= out_label) { l = ls.pull(); }

      // Resolve requests in change_pq_1 for this level
      while (change_pq_1.can_pull()) {
        while (n.uid < change_pq_1.top().target) { n = in_nodes.pull(); }
        adiar_debug(n.uid == change_pq_1.top().target, "should always find desired node");

        const change_rec r = change_level
          ? zdd_change_flip_existing(n)
          : zdd_change_keep_existing(n);

        if (std::holds_alternative<change_rec_skipto>(r)) {
          const change_rec_skipto rs = std::get<change_rec_skipto>(r);

          // Will the flip kill the node?
          if (is_sink(rs.tgt) && is_nil(change_pq_1.top().source)) {
            return zdd_sink(value_of(rs.tgt));
          }

          while (change_pq_1.can_pull() && change_pq_1.top().target == n.uid) {
            zdd_change_forward_edge(aw, change_pq_1, change_pq_2,
                                    change_pq_1.pull().source, rs.tgt,
                                    out_label, l);
          }
        } else {
          const change_rec_output ro = std::get<change_rec_output>(r);
          const uid_t out_uid = create_node_uid(out_label, out_id++);

          zdd_change_forward_edge(aw, change_pq_1, change_pq_2, out_uid, ro.low, out_label, l);
          zdd_change_forward_edge(aw, change_pq_1, change_pq_2, flag(out_uid), ro.high, out_label, l);

          // Output ingoing arcs
          while (change_pq_1.can_pull() && change_pq_1.top().target == n.uid) {
            const arc_t parent_arc = change_pq_1.pull();
            if (!is_nil(parent_arc.source)) {
              aw.unsafe_push_node({ parent_arc.source, out_uid });
            }
          }
        }
      }

      // Resolve requests in change_pq_2 to cut arcs on this level
      while (!change_pq_2.empty() && change_pq_2.top().cut_at == out_label) {
        adiar_invariant(out_label <= l,
                        "the last iteration with this queue is for the very last label to cut on");

        const change_cut request = change_pq_2.top();
        const change_rec_output ro = zdd_change_flip_cut(request.target);
        const uid_t out_uid = create_node_uid(out_label, out_id++);

        zdd_change_forward_edge(aw, change_pq_1, change_pq_2, out_uid, ro.low, out_label, l);
        zdd_change_forward_edge(aw, change_pq_1, change_pq_2, flag(out_uid), ro.high, out_label, l);

        // Output ingoing arcs
        while (!change_pq_2.empty()
               && change_pq_2.top().cut_at == out_label
               && change_pq_2.top().target == request.target) {
          const change_cut parent = change_pq_2.top();
          change_pq_2.pop();

          if (!is_nil(parent.source)) {
            aw.unsafe_push_node({ parent.source, out_uid });
          }
        }
      }
    }

    // Push the level of the very last iteration
    if (out_id > 0) {
      aw.unsafe_push(create_meta(out_label, out_id));
    }

    return out_arcs;
  }
}
