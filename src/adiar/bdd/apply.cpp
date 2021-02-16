#ifndef ADIAR_APPLY_CPP
#define ADIAR_APPLY_CPP

#include "apply.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/priority_queue.h>
#include <adiar/tuple.h>

#include <adiar/bdd/build.h>
#include <adiar/bdd/negate.h>

#include <adiar/assert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct apply_tuple : tuple
  {
    ptr_t source;
  };

  struct apply_tuple_data : tuple_data
  {
    ptr_t source;
  };

  typedef node_priority_queue<apply_tuple, tuple_label, tuple_fst_lt, std::less<>, 2> apply_priority_queue_t;
  typedef tpie::priority_queue<apply_tuple_data, tuple_snd_lt> apply_data_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  inline void apply_init_request(node_stream<> &in_nodes, node_t &v, label_t out_label,
                                 ptr_t &low, ptr_t &high)
  {
    if (!is_sink(v) && label_of(v) == out_label) {
      low = v.low;
      high = v.high;

      if (in_nodes.can_pull()) { v = in_nodes.pull(); }
    } else {
      low = high = v.uid;
    }
  }

  inline void apply_resolve_request(apply_priority_queue_t &appD,
                                    arc_writer &aw,
                                    const bool_op &op,
                                    ptr_t source, ptr_t r1, ptr_t r2)
  {
    if (is_sink(r1) && is_sink(r2)) {
      arc_t out_arc = { source, op(r1, r2) };
      aw.unsafe_push_sink(out_arc);
    } else if (is_sink(r1) && can_left_shortcut(op, r1)) {
      arc_t out_arc = { source, op(r1, create_sink_ptr(true)) };
      aw.unsafe_push_sink(out_arc);
    } else if (is_sink(r2) && can_right_shortcut(op, r2)) {
      arc_t out_arc = { source, op(create_sink_ptr(true), r2) };
      aw.unsafe_push_sink(out_arc);
    } else {
      appD.push({ r1, r2, source });
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &bdd_1,
                  const bdd &bdd_2,
                  const bool_op &op)
  {
    // Resolve being given the same underlying input
    if (bdd_1.file._file_ptr == bdd_2.file._file_ptr) {
      ptr_t sink_1_F = create_sink_ptr(bdd_1.negate);
      ptr_t sink_2_F = create_sink_ptr(bdd_2.negate);

      // Compute the results on all children.
      ptr_t op_F = op(sink_1_F, sink_2_F);
      ptr_t op_T = op(negate(sink_1_F), negate(sink_2_F));

      // Does it collapse to a sink?
      if (op_F == op_T) {
        return bdd_sink(value_of(op_F));
      }

      return op_F == sink_1_F ? bdd_1 : ~bdd_1;
    }

    node_stream<> in_nodes_1(bdd_1);
    node_stream<> in_nodes_2(bdd_2);

    node_t v1 = in_nodes_1.pull();
    node_t v2 = in_nodes_2.pull();

    // Resolve sink shortcutting the result
    if (is_sink(v1) && is_sink(v2)) {
      ptr_t p = op(v1.uid, v2.uid);
      return bdd_sink(value_of(p));
    } else if (is_sink(v1)) {
      if (can_left_shortcut(op, v1.uid)) {
        ptr_t p =  op(v1.uid, create_sink_ptr(false));
        return bdd_sink(value_of(p));
      }
      if (is_left_irrelevant(op, v1.uid)) {
        return bdd_2;
      }
      if (is_left_negating(op, v1.uid)) {
        return bdd_not(bdd_2);
      }
    } else if (is_sink(v2)) {
      if (can_right_shortcut(op, v2.uid)) {
        ptr_t p = op(create_sink_ptr(false), v2.uid);
        return bdd_sink(value_of(p));
      }
      if (is_right_irrelevant(op, v2.uid)) {
        return bdd_1;
      }
      if (is_right_negating(op, v2.uid)) {
        return bdd_not(bdd_1);
      }
    }

    // Set-up for Apply Algorithm
    arc_file out_arcs;
    arc_writer aw(out_arcs);

    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    // TODO: Do statistics on size of the 2 priority queues
    apply_priority_queue_t appD({bdd_1,bdd_2}, available_memory / 2);
    apply_data_priority_queue_t appD_data(calc_tpie_pq_factor(available_memory / 2));

    // Process root and create initial recursion requests
    label_t out_label = label_of(fst(v1.uid, v2.uid));
    id_t out_id = 0;

    aw.unsafe_push(meta_t { out_label });

    ptr_t low1, low2, high1, high2;
    apply_init_request(in_nodes_1, v1, out_label, low1, high1);
    apply_init_request(in_nodes_2, v2, out_label, low2, high2);

    // Shortcut the root
    uid_t out_uid = create_node_uid(out_label, out_id);
    apply_resolve_request(appD, aw, op, out_uid, low1, low2);
    apply_resolve_request(appD, aw, op, flag(out_uid), high1, high2);

    // Process nodes in topological order of both BDDs
    while (appD.can_pull() || appD.has_next_layer() || !appD_data.empty()) {
      if (!appD.can_pull() && appD_data.empty()) {
        appD.setup_next_layer();
        out_label = appD.current_layer();
        aw.unsafe_push(meta_t { out_label });
        out_id = 0;
      }

      ptr_t source, t1, t2;
      bool with_data = false;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from  appD or appD_data
      if (appD.can_pull() && (appD_data.empty() ||
                              fst(appD.top().t1, appD.top().t2) <
                              snd(appD_data.top().t1, appD_data.top().t2))) {
        source = appD.top().source;
        t1 = appD.top().t1;
        t2 = appD.top().t2;

        appD.pop();
      } else {
        source = appD_data.top().source;
        t1 = appD_data.top().t1;
        t2 = appD_data.top().t2;

        with_data = true;
        data_low = appD_data.top().data_low;
        data_high = appD_data.top().data_high;

        appD_data.pop();
      }

      adiar_invariant(is_sink(t1) || out_label <= label_of(t1),
                      "Request should never level-wise be behind current position");
      adiar_invariant(is_sink(t2) || out_label <= label_of(t2),
                      "Request should never level-wise be behind current position");

      // Seek request partially in stream
      ptr_t t_seek = with_data ? snd(t1,t2) : fst(t1,t2);
      while (v1.uid < t_seek && in_nodes_1.can_pull()) {
        v1 = in_nodes_1.pull();
      }
      while (v2.uid < t_seek && in_nodes_2.can_pull()) {
        v2 = in_nodes_2.pull();
      }

      // Forward information across the level

      // TODO: We don't need to forward something, if there are no more nodes to
      // pull from that stream.
      if (is_node(t1) && is_node(t2) && label_of(t1) == label_of(t2)
          && !with_data && (v1.uid != t1 || v2.uid != t2)) {
        node_t v0 = v1.uid == t1 ? v1 : v2;

        appD_data.push({ t1, t2, v0.low, v0.high, source });

        while (appD.can_pull() && appD.top().t1 == t1 && appD.top().t2 == t2) {
          source = appD.pull().source;
          appD_data.push({ t1, t2, v0.low, v0.high, source });
        }
        continue;
      }

      // Resolve current node and recurse
      // remember from above: ptr_t low1, low2, high1, high2;

      if (is_sink(t1) || is_sink(t2) || label_of(t1) != label_of(t2)) {
        if (t1 < t2) { // ==> label_of(t1) < label_of(t2) || is_sink(t2)
          low1 = v1.low;
          high1 = v1.high;
          low2 = high2 = t2;
        } else { // ==> label_of(t1) > label_of(t2) || is_sink(t1)
          low1 = high1 = t1;
          low2 = v2.low;
          high2 = v2.high;
        }
      } else {
        low1  = t1 < t_seek ? data_low  : v1.low;
        high1 = t1 < t_seek ? data_high : v1.high;

        low2  = t2 < t_seek ? data_low  : v2.low;
        high2 = t2 < t_seek ? data_high : v2.high;
      }

      // Resolve request
      adiar_debug(out_id < MAX_ID, "Has run out of ids");
      out_uid = create_node_uid(out_label, out_id++);

      apply_resolve_request(appD, aw, op, out_uid, low1, low2);
      apply_resolve_request(appD, aw, op, flag(out_uid), high1, high2);

      // Output ingoing arcs
      while (true) {
        arc_t out_arc = { source, out_uid };
        aw.unsafe_push_node(out_arc);

        if (appD.can_pull() && appD.top().t1 == t1 && appD.top().t2 == t2) {
          source = appD.pull().source;
        } else if (!appD_data.empty() && appD_data.top().t1 == t1 && appD_data.top().t2 == t2) {
          source = appD_data.top().source;
          appD_data.pop();
        } else {
          break;
        }
      }
    }

    return out_arcs;
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, and_op);
  }

  __bdd bdd_nand(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, nand_op);
  }

  __bdd bdd_or(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, or_op);
  }

  __bdd bdd_nor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, nor_op);
  }

  __bdd bdd_xor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, xor_op);
  }

  __bdd bdd_xnor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, xnor_op);
  }

  __bdd bdd_imp(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, imp_op);
  }

  __bdd bdd_invimp(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, invimp_op);
  }

  __bdd bdd_equiv(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, equiv_op);
  }

  __bdd bdd_diff(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, diff_op);
  }

  __bdd bdd_less(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, less_op);
  }
}

#endif // ADIAR_APPLY_CPP
