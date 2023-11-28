#ifndef ADIAR_INTERNAL_ALGORITHMS_PROD2_H
#define ADIAR_INTERNAL_ALGORITHMS_PROD2_H

#include <variant>

#include <adiar/exec_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/node_random_access.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  2-ary Product Construction
  // ============================
  //
  // Given two Decision Diagrams construct the product of both.
  /*
  //          (a)          (b)               _(a,b)_
  //         /   \    X   /   \     =>      /       \
  //        a0   a1      b1   b2        (a0,b0)   (a1,b1)
  */
  // Examples of uses are `bdd_apply` and `zdd_binop`.
  //////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::prod2_t stats_prod2;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template<uint8_t nodes_carried>
  using prod2_request = request_data<2, with_parent, nodes_carried>;

  template<size_t look_ahead, memory_mode mem_mode>
  using prod_priority_queue_t =
    levelized_node_priority_queue<prod2_request<0>, request_data_lt<1, prod2_request<0>>,
                                  look_ahead,
                                  mem_mode,
                                  2,
                                  0>;

  template<size_t look_ahead, memory_mode mem_mode>
  using prod_priority_queue_1_t =
    levelized_node_priority_queue<prod2_request<0>, request_data_first_lt<prod2_request<0>>,
                                  look_ahead,
                                  mem_mode,
                                  2,
                                  0>;

  template<memory_mode mem_mode>
  using prod_priority_queue_2_t =
    priority_queue<mem_mode, prod2_request<1>, request_data_second_lt<prod2_request<1>>>;

  // TODO: turn into 'tuple<tuple<node::pointer_type>>'
  struct prod2_rec_output {
    tuple<dd::pointer_type> low;
    tuple<dd::pointer_type> high;
  };

  using prod2_rec_skipto = tuple<dd::pointer_type>;

  using prod2_rec = std::variant<prod2_rec_output, prod2_rec_skipto>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forward edge from `source` to `target`.
  ///
  /// \details If `target` is two terminals, the result terminal is computed,
  ///          and the edge output. Otherwise the edge is forwarded to be
  ///          processed later.
  ///
  /// \pre `source` level is strictly before `target`
  //////////////////////////////////////////////////////////////////////////////
  template<typename PriorityQueue_1>
  inline void __prod2_recurse_out(PriorityQueue_1 &prod_pq_1, arc_writer &aw,
                                  const bool_op &op,
                                  const ptr_uint64 &source,
                                  const tuple<ptr_uint64> &target)
  {
    if (target[0].is_terminal() && target[1].is_terminal()) {
      const arc out_arc = { source, op(target[0], target[1]) };
      aw.push_terminal(out_arc);
    } else {
      adiar_assert(source.label() < std::min(target[0], target[1]).label(),
                   "should always push recursion for 'later' level");

      prod_pq_1.push({ target, {}, {source} });
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Run `Policy::go(...)` as long as there are in-going arcs to
  ///        `target` in the Levelized Priority Queue.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename ExtraArg, typename PriorityQueue>
  inline void __prod2_recurse_in__1(PriorityQueue &prod_pq,
                                    arc_writer &aw,
                                    const ExtraArg &ea,
                                    const tuple<ptr_uint64> &target)
  {
    // TODO: merge with the per-level priority queue below. This requires adding
    //       `can_pull()` or similar to `internal::priority_queue`. Maybe it
    //       should have a prettier name like `has_top()`?
    while (!prod_pq.empty_level() && prod_pq.top().target == target) {
      Policy::go(prod_pq, aw, ea, prod_pq.pull().data.source);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Run `Policy::go(...)` as long as there are in-going arcs to
  ///        `target` in the Per-level Priority Queue.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename ExtraArg, typename PriorityQueue>
  inline void __prod2_recurse_in__2(PriorityQueue &prod_pq,
                                    arc_writer &aw,
                                    const ExtraArg &ea,
                                    const tuple<ptr_uint64> &target)
  {
    while (!prod_pq.empty() && prod_pq.top().target == target) {
      Policy::go(prod_pq, aw, ea, prod_pq.top().data.source);
      prod_pq.pop();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Run `Policy::go(...)` as long as there are in-going arcs to
  ///        `target` in the priority queue.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename ExtraArg, typename PriorityQueue>
  inline void __prod2_recurse_in(PriorityQueue &prod_pq,
                                 arc_writer &aw,
                                 const ExtraArg &ea,
                                 const tuple<ptr_uint64> &target)
  {
    // HACK for hiding '__1' and '__2' versions
    __prod2_recurse_in__1<Policy>(prod_pq, aw, ea, target);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Run `Policy::go(...)` as long as there are in-going arcs to
  ///        `target` in either of the priority queues.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename ExtraArg, typename PriorityQueue_1, typename PriorityQueue_2>
  inline void __prod2_recurse_in(PriorityQueue_1 &prod_pq_1, PriorityQueue_2 &prod_pq_2,
                                 arc_writer &aw,
                                 const ExtraArg &ea,
                                 const tuple<ptr_uint64> &target)
  {
    __prod2_recurse_in__1<Policy>(prod_pq_1, aw, ea, target);
    __prod2_recurse_in__2<Policy>(prod_pq_2, aw, ea, target);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `__prod2_recurse_in` for an arc to an internal node.
  //////////////////////////////////////////////////////////////////////////////
  struct __prod2_recurse_in__output_node
  {
    template<typename PriorityQueue>
    static inline void go(PriorityQueue& /*prod_pq_1*/, arc_writer &aw,
                          const node::uid_type &out_uid,
                          const node::pointer_type &source)
    {
      if (!source.is_nil()) {
        aw.push_internal({ source, out_uid });
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `__prod2_recurse_in` for an arc to a terminal.
  //////////////////////////////////////////////////////////////////////////////
  struct __prod2_recurse_in__output_terminal
  {
    template<typename PriorityQueue_1>
    static inline void go(PriorityQueue_1& /*prod_pq_1*/, arc_writer &aw,
                          const ptr_uint64 &out_terminal,
                          const ptr_uint64 &source)
    {
      aw.push_terminal({ source, out_terminal });
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `__prod2_recurse_in` for forwarding a request further,
  ///        i.e. skipping outputting an internal node.
  //////////////////////////////////////////////////////////////////////////////
  struct __prod2_recurse_in__forward
  {
    template<typename PriorityQueue>
    static inline void go(PriorityQueue &prod_pq, arc_writer&,
                          const prod2_rec_skipto &r,
                          const ptr_uint64 &source)
    {
      prod_pq.push({ { r[0], r[1] }, {}, {source} });
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Construct a terminal for the result of two pointers.
  //////////////////////////////////////////////////////////////////////////////
  template<typename DdPolicy>
  inline shared_levelized_file<node>
  __prod2_terminal(const tuple<typename DdPolicy::pointer_type> &rp, const bool_op &op)
  {
    // TODO: Abuse that op(tgt[0], tgt[1]) already is a pointer.
    return build_terminal<DdPolicy>(op(rp[0], rp[1]).value());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Common logic for reconstructing nodes forwarded across a level.
  ///
  /// \details The policy for `prod2` should provide a `merge` function which.
  ///          Inherit from this class, if you can guarantee all requests are
  ///          to pairs of nodes on the same level.
  ///
  /// \pre All requests are assumed to only be for nodes on the same level
  ///
  /// \see prod2_mixed_level_merger
  //////////////////////////////////////////////////////////////////////////////
  template<typename DdPolicy>
  class prod2_same_level_merger
  {
  public:
    template<typename Request>
    static tuple<typename DdPolicy::children_type>
    merge(const Request &r,
          const ptr_uint64 &t_seek,
          const node &v0,
          const node &v1)
    {
      const typename DdPolicy::children_type pair_0 =
        r.target[0] < t_seek ? r.node_carry[0] : v0.children();

      const typename DdPolicy::children_type pair_1 =
        r.target[1] < t_seek ? r.node_carry[0] : v1.children();

      return { pair_0, pair_1 };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logic for reconstructing nodes forwarded across a level.
  ///
  /// \details The policy for `prod2` should provide a `merge` function which.
  ///          Inherit from this class, if you cannot guarantee all requests are
  ///          to pairs of nodes on the same level.
  ///
  /// \pre Requests may be for nodes on different levels.
  ///
  /// \see prod2_same_level_merger
  //////////////////////////////////////////////////////////////////////////////
  template<typename DdPolicy>
  class prod2_mixed_level_merger
  {
  public:
    template<typename Request>
    static tuple<typename DdPolicy::children_type>
    merge(const Request &r,
          const typename DdPolicy::pointer_type &t_seek,
          const typename DdPolicy::node_type &v0,
          const typename DdPolicy::node_type &v1)
    {
      if (r.target[0].is_terminal() ||
          r.target[1].is_terminal() ||
          r.target[0].label() != r.target[1].label()) {

        adiar_assert(r.target[0] != r.target[1],
                     "Cannot have mismatching levels and be equal");

        // t.target[0].label() < r.target[1].label() || r.target[1].is_terminal() ?
        const typename DdPolicy::children_type pair_0 =
          r.target[0] < r.target[1]
            ? v0.children()
            : DdPolicy::reduction_rule_inv(r.target[0]);

        // r.target[1].label() < r.target[0].label() || r.target[0].is_terminal() ?
        const typename DdPolicy::children_type pair_1 =
          r.target[1] < r.target[0]
            ? v1.children()
            : DdPolicy::reduction_rule_inv(r.target[1]);

        return { pair_0, pair_1 };
      } else {
        return prod2_same_level_merger<DdPolicy>::merge(r, t_seek, v0, v1);
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief 2-ary Product Construction Algorithm where Random Access is used to
  ///        get nodes from the one of the two decision diagrams; this removes
  ///        the need for the secondary priority queue in `__prod2_pq`.
  ///
  /// \pre `in_ra` is the input to random access
  //////////////////////////////////////////////////////////////////////////////
  // TODO (Optimiation): Flip 'in_1' to be the one to do Random Access on. This
  //                     simplifies the request ordering by merging the `idx`
  //                     comparison with the final lexicographical tie-breaker.
  template<typename Policy, typename PriorityQueue_1>
  typename Policy::__dd_type
  __prod2_ra(const exec_policy &ep,
             const typename Policy::dd_type &in_ra,
             const typename Policy::dd_type &in_pq,
             const bool_op &op,
             const size_t pq_memory, const size_t max_pq_size)
  {
    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    // Set up input
    node_random_access<> in_nodes_ra(in_ra);
    node_stream<> in_nodes_pq(in_pq);

    node v_pq = in_nodes_pq.pull();

    // Set up cross-level priority queue
    PriorityQueue_1 prod_pq({in_ra, in_pq}, pq_memory, max_pq_size, stats_prod2.lpq);
    prod_pq.push({ { in_nodes_ra.root(), v_pq.uid() }, {}, { ptr_uint64::nil() } });

    out_arcs->max_1level_cut = prod_pq.size();

    // Process all requests
    while (!prod_pq.empty()){
      // Set up level
      prod_pq.setup_next_level();

      typename Policy::label_type out_label = prod_pq.current_level();
      typename Policy::id_type out_id = 0;

      in_nodes_ra.setup_next_level(out_label);

      // Update maximum 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, prod_pq.size());

      // Process all requests for this level
      while (!prod_pq.empty_level()) {
        const prod2_request<0> req = prod_pq.top();

        constexpr size_t ra_idx = 0;
        constexpr size_t pq_idx = 1;

        // Seek request partially in stream
        if (req.target[pq_idx].is_node() && req.target[pq_idx].label() == out_label) {
          while (v_pq.uid() < req.target[pq_idx] && in_nodes_pq.can_pull()) {
            v_pq = in_nodes_pq.pull();
          }

          adiar_assert(v_pq.uid() == req.target[pq_idx],
                       "Must have found correct node in `in_1`");
        }

        // Recreate/Obtain children of req.target (possibly of suppressed node)
        const typename Policy::children_type children_ra =
          req.target[ra_idx].on_level(out_label)
              ? in_nodes_ra.at(req.target[ra_idx]).children()
              : Policy::reduction_rule_inv(req.target[ra_idx]);

        const typename Policy::children_type children_pq =
          req.target[pq_idx].on_level(out_label)
              ? v_pq.children()
              : Policy::reduction_rule_inv(req.target[pq_idx]);

        // Create pairing of product children
        const tuple<typename Policy::pointer_type> rec_pair_0 =
          { children_ra[false], children_pq[false] };

        const tuple<typename Policy::pointer_type> rec_pair_1 =
          { children_ra[true], children_pq[true] };

        // Obtain new recursion targets
        const prod2_rec rec_res =
          Policy::resolve_request(op, rec_pair_0, rec_pair_1);

        // Forward recursion targets
        if (Policy::no_skip || std::holds_alternative<prod2_rec_output>(rec_res)) {
          const prod2_rec_output r = std::get<prod2_rec_output>(rec_res);

          adiar_assert(out_id < Policy::max_id, "Has run out of ids");
          const node::uid_type out_uid(out_label, out_id++);

          __prod2_recurse_out(prod_pq, aw, op, out_uid.with(false), r.low);
          __prod2_recurse_out(prod_pq, aw, op, out_uid.with(true),  r.high);

          __prod2_recurse_in<__prod2_recurse_in__output_node>
            (prod_pq, aw, out_uid, req.target);

        } else { // std::holds_alternative<prod2_rec_skipto>(root_rec)
          const prod2_rec_skipto r = std::get<prod2_rec_skipto>(rec_res);
          if (r[0].is_terminal() && r[1].is_terminal()) {
            if (req.data.source.is_nil()) {
              // Skipped in both DAGs all the way from the root until a pair of terminals.
              return __prod2_terminal<Policy>(r, op);
            }
            __prod2_recurse_in<__prod2_recurse_in__output_terminal>
              (prod_pq, aw, op(r[0], r[1]), req.target);
          } else {
            __prod2_recurse_in<__prod2_recurse_in__forward>
              (prod_pq, aw, r, req.target);
          }
        }
      }

      // Update meta information
      if (Policy::no_skip || out_id > 0) {
        aw.push(level_info(out_label, out_id));
      }
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                                  - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief 2-ary Product Construction where nodes are potentially forwarded
  ///        with a secondary priority queue such that they are accessible at
  ///        the same time.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename PriorityQueue_1, typename PriorityQueue_2>
  typename Policy::__dd_type
  __prod2_pq(const exec_policy &ep,
             const typename Policy::dd_type &in_0,
             const typename Policy::dd_type &in_1,
             const bool_op &op,
             const size_t pq_1_memory, const size_t max_pq_1_size,
             const size_t pq_2_memory, const size_t max_pq_2_size)
  {
    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_writer aw(out_arcs);

    out_arcs->max_1level_cut = 0;

    // Set up input
    node_stream<> in_nodes_0(in_0);
    node_stream<> in_nodes_1(in_1);

    node v0 = in_nodes_0.pull();
    node v1 = in_nodes_1.pull();

    // Set up cross-level priority queue
    PriorityQueue_1 prod_pq_1({in_0, in_1}, pq_1_memory, max_pq_1_size, stats_prod2.lpq);
    prod_pq_1.push({ { v0.uid(), v1.uid() }, {}, { ptr_uint64::nil() } });

    // Set up per-level priority queue
    PriorityQueue_2 prod_pq_2(pq_2_memory, max_pq_2_size);

    // Process requests in topological order of both BDDs
    while (!prod_pq_1.empty()) {
      // Set up next level
      prod_pq_1.setup_next_level();

      const typename Policy::label_type out_label = prod_pq_1.current_level();
      typename Policy::id_type out_id = 0;

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, prod_pq_1.size());

      // Process all requests for this level
      while (!prod_pq_1.empty_level() || !prod_pq_2.empty()) {
        prod2_request<1> req;

        // Merge requests from prod_pq_1 or prod_pq_2
        if (prod_pq_1.can_pull() && (prod_pq_2.empty() ||
                                     prod_pq_1.top().target.first() < prod_pq_2.top().target.second())) {
          req = { prod_pq_1.top().target,
                  {{ { node::pointer_type::nil(), node::pointer_type::nil() } }},
                  { prod_pq_1.top().data } };
        } else {
          req = prod_pq_2.top();
        }

        adiar_assert(req.target[0].is_terminal() || out_label <= req.target[0].label(),
                     "Request should never level-wise be behind current position");
        adiar_assert(req.target[1].is_terminal() || out_label <= req.target[1].label(),
                     "Request should never level-wise be behind current position");

        // Seek request partially in stream
        const typename Policy::pointer_type t_seek =
          req.empty_carry() ? req.target.first() : req.target.second();

        while (v0.uid() < t_seek && in_nodes_0.can_pull()) {
          v0 = in_nodes_0.pull();
        }
        while (v1.uid() < t_seek && in_nodes_1.can_pull()) {
          v1 = in_nodes_1.pull();
        }

        // Forward information across the level
        if (req.empty_carry()
            && req.target[0].is_node() && req.target[1].is_node()
            && req.target[0].label() == req.target[1].label()
            && (v0.uid() != req.target[0] || v1.uid() != req.target[1])) {
          const typename Policy::children_type children =
            (req.target[0] == v0.uid() ? v0 : v1).children();

          while (prod_pq_1.can_pull() && prod_pq_1.top().target == req.target) {
#ifdef ADIAR_STATS
            stats_prod2.pq.pq_2_elems += 1u;
#endif
            prod_pq_2.push({ req.target, { children }, prod_pq_1.pull().data });
          }
          continue;
        }

        // Recreate children of nodes for req.target
        const tuple<typename Policy::children_type> children =
          Policy::merge(req, t_seek, v0, v1);

        // Create pairing of product children
        const tuple<typename Policy::pointer_type> rec_pair_0 =
          { children[0][false], children[1][false] };

        const tuple<typename Policy::pointer_type> rec_pair_1 =
          { children[0][true], children[1][true] };

        // Obtain new recursion targets
        const prod2_rec rec_res =
          Policy::resolve_request(op, rec_pair_0, rec_pair_1);

        // Forward recursion targets
        if (Policy::no_skip || std::holds_alternative<prod2_rec_output>(rec_res)) {
          const prod2_rec_output r = std::get<prod2_rec_output>(rec_res);

          adiar_assert(out_id < Policy::max_id, "Has run out of ids");
          const node::uid_type out_uid(out_label, out_id++);

          __prod2_recurse_out(prod_pq_1, aw, op, out_uid.with(false), r.low);
          __prod2_recurse_out(prod_pq_1, aw, op, out_uid.with(true),  r.high);

          __prod2_recurse_in<__prod2_recurse_in__output_node>
            (prod_pq_1, prod_pq_2, aw, out_uid, req.target);

        } else { // std::holds_alternative<prod2_rec_skipto>(root_rec)
          const prod2_rec_skipto r = std::get<prod2_rec_skipto>(rec_res);
          if (r[0].is_terminal() && r[1].is_terminal()) {
            if (req.data.source.is_nil()) {
              // Skipped in both DAGs all the way from the root until a pair of terminals.
              return __prod2_terminal<Policy>(r, op);
            }
            __prod2_recurse_in<__prod2_recurse_in__output_terminal>
              (prod_pq_1, prod_pq_2, aw, op(r[0], r[1]), req.target);
          } else {
            __prod2_recurse_in<__prod2_recurse_in__forward>
              (prod_pq_1, prod_pq_2, aw, r, req.target);
          }
        }
      }

      // Push meta data about this level
      if (Policy::no_skip || out_id > 0) {
        aw.push(level_info(out_label, out_id));
      }
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                                  - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut based on the
  /// product of the maximum i-level cut of both inputs.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy, typename cut, size_t const_size_inc>
  size_t __prod2_ilevel_upper_bound(const typename Policy::dd_type &in_0,
                                    const typename Policy::dd_type &in_1,
                                    const bool_op &op)
  {
    // Cuts for left-hand side
    const safe_size_t left_cut_internal = cut::get(in_0, cut::type::Internal);

    const typename cut::type left_ct = Policy::left_cut(op);
    const safe_size_t left_cut_terminals = cut::get(in_0, left_ct) - left_cut_internal;

    // Cuts for right-hand side
    const safe_size_t right_cut_internal = cut::get(in_1, cut::type::Internal);

    const typename cut::type right_ct = Policy::right_cut(op);
    const safe_size_t right_cut_terminals = cut::get(in_1, right_ct) - right_cut_internal;

    // Compute cut, where we make sure not to pair terminals with terminals.
    return to_size(left_cut_internal * right_cut_internal
                   + left_cut_terminals * right_cut_internal
                   + left_cut_internal * right_cut_terminals
                   + const_size_inc);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum 2-level cut based on both
  /// using the max 1 and 2-level cuts and the number of relevant terminals.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy>
  size_t __prod2_2level_upper_bound(const typename Policy::dd_type &in_0,
                                    const typename Policy::dd_type &in_1,
                                    const bool_op &op)
  {
    // Left-hand side
    const safe_size_t left_2level_cut = in_0.max_2level_cut(cut::Internal);
    const safe_size_t left_1level_cut = in_0.max_1level_cut(cut::Internal);

    const cut left_ct = Policy::left_cut(op);
    const safe_size_t left_terminal_vals = left_ct.number_of_terminals();

    const safe_size_t left_terminal_arcs =  in_0.max_1level_cut(left_ct) - left_1level_cut;

    // Right-hand side
    const safe_size_t right_2level_cut = in_1.max_2level_cut(cut::Internal);
    const safe_size_t right_1level_cut = in_1.max_1level_cut(cut::Internal);

    const cut right_ct = Policy::right_cut(op);
    const safe_size_t right_terminal_vals = right_ct.number_of_terminals();

    const safe_size_t right_terminal_arcs = in_1.max_1level_cut(right_ct) - right_1level_cut;

    // Compute cut, where we count the product, the input-terminal pairings, and the
    // connection from the product to the input-terminal pairings separately.
    return to_size(left_2level_cut * right_2level_cut
                   + (right_1level_cut * left_terminal_arcs) + left_terminal_vals * right_2level_cut
                   + (left_1level_cut * right_terminal_arcs) + right_terminal_vals * left_2level_cut
                   + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Upper bound on i-level cut based on the maximum possible number of nodes
  /// in the output.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy>
  size_t __prod2_ilevel_upper_bound(const typename Policy::dd_type &in_0,
                                   const typename Policy::dd_type &in_1,
                                   const bool_op &op)
  {
    const cut left_ct = Policy::left_cut(op);
    const safe_size_t left_terminal_vals = left_ct.number_of_terminals();
    const safe_size_t left_size = in_0->size();

    const cut right_ct = Policy::right_cut(op);
    const safe_size_t right_terminal_vals = right_ct.number_of_terminals();
    const safe_size_t right_size = in_1->size();

    return to_size((left_size + left_terminal_vals) * (right_size + right_terminal_vals) + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy>
  typename Policy::__dd_type
  __prod2_ra(const exec_policy &ep,
             const typename Policy::dd_type &in_0,
             const typename Policy::dd_type &in_1,
             const bool_op &op)
  {
    adiar_assert(in_0->canonical || in_1->canonical, "At least one input must be canonical");

    const bool internal_only = ep.memory_mode() == exec_policy::memory::Internal;
    const bool external_only = ep.memory_mode() == exec_policy::memory::External;

    const size_t pq_1_bound = std::min({__prod2_ilevel_upper_bound<Policy, get_2level_cut, 2u>(in_0, in_1, op),
        __prod2_2level_upper_bound<Policy>(in_0, in_1, op),
        __prod2_ilevel_upper_bound<Policy>(in_0, in_1, op)});

    // Flip inputs such that 'ra_in_0' is the one to use random-access on.
    const bool ra_0 = in_0->canonical && (!in_1->canonical || in_0.width() <= in_1.width());
    const typename Policy::dd_type& ra_in_0 = ra_0 ? in_0 : in_1;
    const typename Policy::dd_type& ra_in_1 = ra_0 ? in_1 : in_0;
    const bool_op& ra_op = ra_0 ? op : flip(op);

#ifdef ADIAR_STATS
    stats_prod2.ra.used_narrowest +=
      static_cast<size_t>(ra_in_0.width() == std::min(in_0.width(), in_1.width()));

    stats_prod2.ra.acc_width += ra_in_0.width();
    stats_prod2.ra.min_width = std::min(stats_prod2.ra.min_width, ra_in_0.width());
    stats_prod2.ra.max_width = std::max(stats_prod2.ra.max_width, ra_in_0.width());
#endif

    const size_t pq_available_memory = memory_available()
      // Input stream
      - node_stream<>::memory_usage()
      // Random access
      - node_random_access<>::memory_usage(ra_in_0)
      // Output stream
      - arc_writer::memory_usage();

    const size_t pq_memory_fits =
      prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_available_memory);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_1_bound) : pq_1_bound;

    if(!external_only && max_pq_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2.lpq.unbucketed += 1u;
#endif
      return __prod2_ra<Policy,
                        prod_priority_queue_t<0, memory_mode::Internal>>
        (ep, ra_in_0, ra_in_1, ra_op, pq_available_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2.lpq.internal += 1u;
#endif
      return __prod2_ra<Policy,
                        prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>
        (ep, ra_in_0, ra_in_1, ra_op, pq_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2.lpq.external += 1u;
#endif
      return __prod2_ra<Policy,
                        prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>
        (ep, ra_in_0, ra_in_1, ra_op, pq_available_memory, max_pq_size);
    }
  }

  template<typename Policy>
  typename Policy::__dd_type
  __prod2_pq(const exec_policy &ep,
             const typename Policy::dd_type &in_0,
             const typename Policy::dd_type &in_1,
             const bool_op &op)
  {
    const bool internal_only = ep.memory_mode() == exec_policy::memory::Internal;
    const bool external_only = ep.memory_mode() == exec_policy::memory::External;

    const size_t pq_1_bound = std::min({__prod2_ilevel_upper_bound<Policy, get_2level_cut, 2u>(in_0, in_1, op),
        __prod2_2level_upper_bound<Policy>(in_0, in_1, op),
        __prod2_ilevel_upper_bound<Policy>(in_0, in_1, op)});

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory_available()
      // Input streams
      - 2*node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      prod_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_1_memory_fits =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      prod_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __prod2_ilevel_upper_bound<Policy, get_1level_cut, 0u>(in_0, in_1, op);

    const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if(!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2.lpq.unbucketed += 1u;
#endif
      return __prod2_pq<Policy,
                        prod_priority_queue_1_t<0, memory_mode::Internal>,
                        prod_priority_queue_2_t<memory_mode::Internal>>
        (ep, in_0, in_1, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if(!external_only && max_pq_1_size <= pq_1_memory_fits
              && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2.lpq.internal += 1u;
#endif
      return __prod2_pq<Policy,
                        prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>,
                        prod_priority_queue_2_t<memory_mode::Internal>>
        (ep, in_0, in_1, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2.lpq.external += 1u;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __prod2_pq<Policy,
                        prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>,
                        prod_priority_queue_2_t<memory_mode::External>>
        (ep, in_0, in_1, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  2-ary Product Construction algorithm
  ///
  /// \return A class that inherits from `__dd` and describes the product the
  ///         two given DAGs.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Policy>
  typename Policy::__dd_type
  prod2(const exec_policy &ep,
        const typename Policy::dd_type &in_0,
        const typename Policy::dd_type &in_1,
        const bool_op &op)
  {
    // -------------------------------------------------------------------------
    // Case: Same file, i.e. exactly the same DAG.
    if (in_0.file_ptr() == in_1.file_ptr()) {
#ifdef ADIAR_STATS
      stats_prod2.trivial_file += 1u;
#endif
      return Policy::resolve_same_file(in_0, in_1, op);
    }

    // -------------------------------------------------------------------------
    // Case: At least one terminal.
    if (dd_isterminal(in_0) || dd_isterminal(in_1)) {
      typename Policy::__dd_type maybe_resolved =
        Policy::resolve_terminal_root(in_0, in_1, op);

      if (!(maybe_resolved.template has<no_file>())) {
#ifdef ADIAR_STATS
        stats_prod2.trivial_terminal += 1u;
#endif
        return maybe_resolved;
      }
    }

    // -------------------------------------------------------------------------
    // Case: Do the product construction (with random access)
    //
    // TODO: Optimisation!
    //
    //   We do not need to check on the decision diagram being 'canonical'. It
    //   merely suffices, that it is 'indexable', i.e. the one half of being
    //   'canonical'.

    // Use random access if requested or the width fits into a single block
    const size_t nodes_per_block = get_block_size() / sizeof(typename Policy::node_type);

    const size_t width_0 = in_0->canonical ? in_0.width() : Policy::max_id;
    const size_t width_1 = in_1->canonical ? in_1.width() : Policy::max_id;
    const size_t min_width = std::min(width_0, width_1);

    if (// Use `__prod2_ra` if user has forced Random Access
        ep.access_mode() == exec_policy::access::Random_Access
        || (// Heuristically, if the narrowest canonical fits
            ep.access_mode() == exec_policy::access::Auto && (min_width <= nodes_per_block))) {
#ifdef ADIAR_STATS
      stats_prod2.ra.runs += 1u;
#endif
      return __prod2_ra<Policy>(ep, in_0, in_1, op);
    }

    // -------------------------------------------------------------------------
    // Case: Do the product construction (with priority queues)
#ifdef ADIAR_STATS
    stats_prod2.pq.runs += 1u;
#endif
    return __prod2_pq<Policy>(ep, in_0, in_1, op);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_PROD2_H
