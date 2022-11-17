#ifndef ADIAR_INTERNAL_ALGORITHMS_PRODUCT_CONSTRUCTION_H
#define ADIAR_INTERNAL_ALGORITHMS_PRODUCT_CONSTRUCTION_H

#include <variant>

#include <adiar/bool_op.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/bdd/bdd.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/decision_diagram.h>

#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>

#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::product_construction_t stats_product_construction;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct prod_tuple_1
  {
    tuple<ptr_uint64> target;
    ptr_uint64 source;
  };

  struct prod_tuple_1_lt
  {
    bool operator()(const prod_tuple_1 &a, const prod_tuple_1 &b)
    {
      return tuple_fst_lt<tuple<ptr_uint64>>()(a.target, b.target)
#ifndef NDEBUG
        || (!tuple_fst_lt<tuple<ptr_uint64>>()(b.target, a.target) && a.source < b.source)
#endif
        ;
    }
  };

  struct prod_tuple_1_label // TODO: remove by generalising to 'request' class
  {
    static inline ptr_uint64::label_t label_of(const prod_tuple_1 &t)
    {
      return t.target.fst().label();
    }
  };

  template<size_t LOOK_AHEAD, memory::memory_mode mem_mode>
  using prod_priority_queue_1_t =
    levelized_node_priority_queue<prod_tuple_1, prod_tuple_1_label,
                                  prod_tuple_1_lt, LOOK_AHEAD,
                                  mem_mode,
                                  2>;

  struct prod_tuple_2
  {
    tuple<ptr_uint64> target;
    ptr_uint64 data_low;  // <-- TODO: data array
    ptr_uint64 data_high; // <-- TODO: data array
    ptr_uint64 source;
  };

  struct prod_tuple_2_lt
  {
    bool operator()(const prod_tuple_2 &a, const prod_tuple_2 &b)
    {
      return tuple_snd_lt<tuple<ptr_uint64>>()(a.target, b.target)
#ifndef NDEBUG
        || (!tuple_snd_lt<tuple<ptr_uint64>>()(b.target, a.target) && a.source < b.source)
#endif
        ;
    }
  };

  template<memory::memory_mode mem_mode>
  using prod_priority_queue_2_t =
    priority_queue<mem_mode, prod_tuple_2, prod_tuple_2_lt>;

  // TODO: turn into 'tuple<tuple<ptr_uint64>>'
  struct prod_rec_output {
    tuple<ptr_uint64> low;
    tuple<ptr_uint64> high;
  };

  typedef tuple<ptr_uint64> prod_rec_skipto;

  typedef std::variant<prod_rec_output, prod_rec_skipto> prod_rec;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template<typename pq_1_t>
  inline void prod_recurse_out(pq_1_t &prod_pq_1, arc_writer &aw,
                               const bool_op &op,
                               ptr_uint64 source, tuple<ptr_uint64> target)
  {
    if (target[0].is_terminal() && target[1].is_terminal()) {
      arc out_arc = { source, op(target[0], target[1]) };
      aw.unsafe_push_terminal(out_arc);
    } else {
      adiar_debug(source.label() < std::min(target[0], target[1]).label(),
                  "should always push recursion for 'later' level");

      prod_pq_1.push({ target, source });
    }
  }

  template<typename out_policy, typename extra_arg, typename pq_1_t, typename pq_2_t>
  inline void prod_recurse_in(pq_1_t &prod_pq_1, pq_2_t &prod_pq_2,
                              arc_writer &aw,
                              const extra_arg &ea, ptr_uint64 t1, ptr_uint64 t2)
  {
    while (prod_pq_1.can_pull() && prod_pq_1.top().target[0] == t1 && prod_pq_1.top().target[1] == t2) {
      out_policy::go(prod_pq_1, aw, ea, prod_pq_1.pull().source);
    }

    while (!prod_pq_2.empty() && prod_pq_2.top().target[0] == t1 && prod_pq_2.top().target[1] == t2) {
      out_policy::go(prod_pq_1, aw, ea, prod_pq_2.top().source);
      prod_pq_2.pop();
    }
  }

  struct prod_recurse_in__output_node
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t& /*prod_pq_1*/, arc_writer &aw,
                          uid_t out_uid, ptr_uint64 source)
    {
      if (!source.is_nil()) {
        aw.unsafe_push_node({ source, out_uid });
      }
    }
  };

  struct prod_recurse_in__output_terminal
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t& /*prod_pq_1*/, arc_writer &aw,
                          ptr_uint64 out_terminal, ptr_uint64 source)
    {
      aw.unsafe_push_terminal({ source, out_terminal });
    }
  };

  struct prod_recurse_in__forward
  {
    template<typename pq_1_t>
    static inline void go(pq_1_t &prod_pq_1, arc_writer&,
                          const prod_rec_skipto &r, ptr_uint64 source)
    {
      prod_pq_1.push({ { r[0], r[1] }, source });
    }
  };

  inline node_file prod_terminal(ptr_uint64 t1, ptr_uint64 t2, const bool_op &op)
  {
    // TODO: Abuse that op(t1,t2) already is a pointer.
    return build_terminal(op(t1,t2).value());
  }

  inline bool prod_from_1(ptr_uint64 t1, ptr_uint64 t2)
  {
    return fst(t1,t2) == t1;
  }

  //////////////////////////////////////////////////////////////////////////////
  class prod_same_level_merger
  {
  public:
    static void merge_root(ptr_uint64 &low1, ptr_uint64 &high1, ptr_uint64 &low2, ptr_uint64 &high2,
                           node::label_t /* level */,
                           const node &v1, const node &v2)
    {
      low1 = v1.low();
      high1 = v1.high();

      low2 = v2.low();
      high2 = v2.high();
    }

  public:
    static void merge_data(ptr_uint64 &low1, ptr_uint64 &high1, ptr_uint64 &low2, ptr_uint64 &high2,
                           ptr_uint64 t1, ptr_uint64 t2, ptr_uint64 t_seek,
                           const node &v1, const node &v2,
                           ptr_uint64 data_low, ptr_uint64 data_high)
    {
      low1  = t1 < t_seek ? data_low  : v1.low();
      high1 = t1 < t_seek ? data_high : v1.high();

      low2  = t2 < t_seek ? data_low  : v2.low();
      high2 = t2 < t_seek ? data_high : v2.high();
    }
  };

  class prod_mixed_level_merger
  {
  private:
    static void __merge_root(const node &v, ptr_uint64::label_t level,
                             ptr_uint64 &low, ptr_uint64 &high)
    {
      if (!v.is_terminal() && v.label() == level) {
        low = v.low();
        high = v.high();
      } else {
        low = high = v.uid();
      }
    }

  public:
    static void merge_root(ptr_uint64 &low1, ptr_uint64 &high1, ptr_uint64 &low2, ptr_uint64 &high2,
                           ptr_uint64::label_t level,
                           const node &v1, const node &v2)
    {
      __merge_root(v1, level, low1, high1);
      __merge_root(v2, level, low2, high2);
    }

  public:
    static void merge_data(ptr_uint64 &low1, ptr_uint64 &high1, ptr_uint64 &low2, ptr_uint64 &high2,
                           ptr_uint64 t1, ptr_uint64 t2, ptr_uint64 t_seek,
                           const node &v1, const node &v2,
                           ptr_uint64 data_low, ptr_uint64 data_high)
    {
      if (t1.is_terminal() || t2.is_terminal() || t1.label() != t2.label()) {
        if (t1 < t2) { // ==> t1.label() < t2.label() || t2.is_terminal()
          low1 = v1.low();
          high1 = v1.high();
          low2 = high2 = t2;
        } else { // ==> t1.label() > t2.label() || t1.is_terminal()
          low1 = high1 = t1;
          low2 = v2.low();
          high2 = v2.high();
        }
      } else {
        prod_same_level_merger::merge_data(low1,high1, low2,high2,
                                           t1, t2, t_seek,
                                           v1, v2,
                                           data_low, data_high);
      }
    }
  };

  template<typename prod_policy, typename pq_1_t, typename pq_2_t>
  typename prod_policy::unreduced_t
  __product_construction(const typename prod_policy::reduced_t &in_1,
                         const typename prod_policy::reduced_t &in_2,
                         const bool_op &op,
                         const size_t pq_1_memory, const size_t max_pq_1_size,
                         const size_t pq_2_memory, const size_t max_pq_2_size)
  {
    node_stream<> in_nodes_1(in_1);
    node_stream<> in_nodes_2(in_2);

    node v1 = in_nodes_1.pull();
    node v2 = in_nodes_2.pull();

    if (v1.is_terminal() || v2.is_terminal()) {
      typename prod_policy::unreduced_t maybe_resolved = prod_policy::resolve_terminal_root(v1, in_1, v2, in_2, op);

      if (!(maybe_resolved.template has<no_file>())) {
        return maybe_resolved;
      }
    }

    // Set-up for Product Construction Algorithm
    arc_file out_arcs;
    arc_writer aw(out_arcs);

    pq_1_t prod_pq_1({in_1, in_2}, pq_1_memory, max_pq_1_size, stats_product_construction.lpq);
    pq_2_t prod_pq_2(pq_2_memory, max_pq_2_size);

    // Process root and create initial recursion requests
    typename prod_policy::label_t out_label = fst(v1.uid(), v2.uid()).label();
    typename prod_policy::id_t out_id = 0;

    typename prod_policy::ptr_t low1, low2, high1, high2;
    prod_policy::merge_root(low1,high1, low2,high2, out_label, v1, v2);

    // Shortcut the root (maybe)
    {
      prod_policy::compute_cofactor(v1.on_level(out_label), low1, high1);
      prod_policy::compute_cofactor(v2.on_level(out_label), low2, high2);

      prod_rec root_rec = prod_policy::resolve_request(op, low1, low2, high1, high2);

      if (std::holds_alternative<prod_rec_output>(root_rec)) {
        prod_rec_output r = std::get<prod_rec_output>(root_rec);
        const uid_t out_uid(out_label, out_id++);

        prod_recurse_out(prod_pq_1, aw, op, out_uid, r.low);
        prod_recurse_out(prod_pq_1, aw, op, flag(out_uid), r.high);
      } else { // std::holds_alternative<prod_rec_skipto>(root_rec)
        prod_rec_skipto r = std::get<prod_rec_skipto>(root_rec);

        if (r[0].is_terminal() && r[1].is_terminal()) {
          return prod_terminal(r[0], r[1], op);
        } else {
          prod_pq_1.push({ { r[0], r[1] }, ptr_uint64::NIL() });
        }
      }
    }

    size_t max_1level_cut = 0;

    // Process nodes in topological order of both BDDs
    while (!prod_pq_1.empty() || !prod_pq_2.empty()) {
      if (prod_pq_1.empty_level() && prod_pq_2.empty()) {
        if (prod_policy::no_skip || out_id > 0) {
          // Only output level_info information on prior level, if output
          aw.unsafe_push(create_level_info(out_label, out_id));
        }

        prod_pq_1.setup_next_level();
        out_label = prod_pq_1.current_level();
        out_id = 0;

        max_1level_cut = std::max(max_1level_cut, prod_pq_1.size());
      }

      ptr_uint64 source, t1, t2; // TODO: merge t1,t2 in a tuple
      bool with_data = false;
      ptr_uint64 data_low = ptr_uint64::NIL(), data_high = ptr_uint64::NIL();

      // Merge requests from  prod_pq_1 or prod_pq_2
      if (prod_pq_1.can_pull() && (prod_pq_2.empty() ||
                                   prod_pq_1.top().target.fst() < prod_pq_2.top().target.snd())) {
        source = prod_pq_1.top().source;
        t1 = prod_pq_1.top().target[0];
        t2 = prod_pq_1.top().target[1];
      } else {
        source = prod_pq_2.top().source;
        t1 = prod_pq_2.top().target[0];
        t2 = prod_pq_2.top().target[1];

        with_data = true;
        data_low = prod_pq_2.top().data_low;
        data_high = prod_pq_2.top().data_high;
      }

      adiar_invariant(t1.is_terminal() || out_label <= t1.label(),
                      "Request should never level-wise be behind current position");
      adiar_invariant(t2.is_terminal() || out_label <= t2.label(),
                      "Request should never level-wise be behind current position");

      // Seek request partially in stream
      ptr_uint64 t_seek = with_data ? snd(t1,t2) : fst(t1,t2);
      while (v1.uid() < t_seek && in_nodes_1.can_pull()) {
        v1 = in_nodes_1.pull();
      }
      while (v2.uid() < t_seek && in_nodes_2.can_pull()) {
        v2 = in_nodes_2.pull();
      }

      // Forward information across the level
      if (t1.is_node() && t2.is_node() && t1.label() == t2.label()
          && !with_data && (v1.uid() != t1 || v2.uid() != t2)) {
        node v0 = t1 == v1.uid() /*prod_from_1(t1,t2)*/ ? v1 : v2;

        while (prod_pq_1.can_pull() && prod_pq_1.top().target[0] == t1 && prod_pq_1.top().target[1] == t2) {
          source = prod_pq_1.pull().source;
          prod_pq_2.push({ { t1, t2 }, v0.low(), v0.high(), source });
        }
        continue;
      }

      // Resolve current node and recurse
      // remember from above: ptr low1, low2, high1, high2;
      prod_policy::merge_data(low1,high1, low2,high2,
                              t1, t2, t_seek,
                              v1, v2,
                              data_low, data_high);

      // Resolve request
      prod_policy::compute_cofactor(t1.on_level(out_label), low1, high1);
      prod_policy::compute_cofactor(t2.on_level(out_label), low2, high2);

      prod_rec rec_res = prod_policy::resolve_request(op, low1, low2, high1, high2);

      if (prod_policy::no_skip || std::holds_alternative<prod_rec_output>(rec_res)) {
        prod_rec_output r = std::get<prod_rec_output>(rec_res);

        adiar_debug(out_id < prod_policy::MAX_ID, "Has run out of ids");
        const uid_t out_uid(out_label, out_id++);

        prod_recurse_out(prod_pq_1, aw, op, out_uid, r.low);
        prod_recurse_out(prod_pq_1, aw, op, flag(out_uid), r.high);

        prod_recurse_in<prod_recurse_in__output_node>(prod_pq_1, prod_pq_2, aw, out_uid, t1, t2);

      } else { // std::holds_alternative<prod_rec_skipto>(root_rec)
        prod_rec_skipto r = std::get<prod_rec_skipto>(rec_res);
        if (r[0].is_terminal() && r[1].is_terminal()) {
          if (source.is_nil()) {
            // Skipped in both DAGs all the way from the root until a pair of terminals.
            return prod_terminal(r[0], r[1], op);
          }
          prod_recurse_in<prod_recurse_in__output_terminal>(prod_pq_1, prod_pq_2, aw, op(r[0], r[1]), t1, t2);
        } else {
          prod_recurse_in<prod_recurse_in__forward>(prod_pq_1, prod_pq_2, aw, r, t1, t2);
        }
      }
    }

    if (prod_policy::no_skip || out_id > 0) {
      // Push the level of the very last iteration
      aw.unsafe_push(create_level_info(out_label, out_id));
    }

    out_arcs->max_1level_cut = max_1level_cut;

    return out_arcs;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut based on the
  /// product of the maximum i-level cut of both inputs.
  //////////////////////////////////////////////////////////////////////////////
  template<typename prod_policy, typename cut, size_t const_size_inc>
  size_t __prod_ilevel_upper_bound(const typename prod_policy::reduced_t &in_1,
                                   const typename prod_policy::reduced_t &in_2,
                                   const bool_op &op)
  {
    // Cuts for left-hand side
    const safe_size_t left_cut_internal = cut::get(in_1, cut_type::INTERNAL);

    const cut_type left_ct = prod_policy::left_cut(op);
    const safe_size_t left_cut_terminals = cut::get(in_1, left_ct) - left_cut_internal;

    // Cuts for right-hand side
    const safe_size_t right_cut_internal = cut::get(in_2, cut_type::INTERNAL);

    const cut_type right_ct = prod_policy::right_cut(op);
    const safe_size_t right_cut_terminals = cut::get(in_2, right_ct) - right_cut_internal;

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
  template<typename prod_policy>
  size_t __prod_2level_upper_bound(const typename prod_policy::reduced_t &in_1,
                                   const typename prod_policy::reduced_t &in_2,
                                   const bool_op &op)
  {
    // Left-hand side
    const safe_size_t left_2level_cut = in_1.max_2level_cut(cut_type::INTERNAL);
    const safe_size_t left_1level_cut = in_1.max_1level_cut(cut_type::INTERNAL);

    const cut_type left_ct = prod_policy::left_cut(op);
    const safe_size_t left_terminal_vals = number_of_terminals(left_ct);

    const safe_size_t left_terminal_arcs =  in_1.max_1level_cut(left_ct) - left_1level_cut;

    // Right-hand side
    const safe_size_t right_2level_cut = in_2.max_2level_cut(cut_type::INTERNAL);
    const safe_size_t right_1level_cut = in_2.max_1level_cut(cut_type::INTERNAL);

    const cut_type right_ct = prod_policy::right_cut(op);
    const safe_size_t right_terminal_vals = number_of_terminals(right_ct);

    const safe_size_t right_terminal_arcs = in_2.max_1level_cut(right_ct) - right_1level_cut;

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
  template<typename prod_policy>
  size_t __prod_ilevel_upper_bound(const typename prod_policy::reduced_t &in_1,
                                   const typename prod_policy::reduced_t &in_2,
                                   const bool_op &op)
  {
    const cut_type left_ct = prod_policy::left_cut(op);
    const safe_size_t left_terminal_vals = number_of_terminals(left_ct);
    const safe_size_t left_size = in_1->size();

    const cut_type right_ct = prod_policy::right_cut(op);
    const safe_size_t right_terminal_vals = number_of_terminals(right_ct);
    const safe_size_t right_size = in_2->size();

    return to_size((left_size + left_terminal_vals) * (right_size + right_terminal_vals) + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Creates the product construction of the given two DAGs.
  ///
  /// Behaviour of the product construction is controlled with the 'prod_policy'
  /// class, which exposes static void strategy functions.
  ///
  /// - resolve_same_file:
  ///   Creates the output based on knowing both inputs refer to the same
  ///   underlying file.
  ///
  /// - resolve_terminal_root:
  ///   Resolves (if possible) the cases for one of the two DAGs only being a
  ///   terminal. Uses the _union in the 'out_t' to trigger an early termination. If
  ///   it holds an 'adiar::no_file', then the algorithm proceeds to the actual
  ///   product construction.
  ///
  /// - resolve_request:
  ///   Given all information collected for the two nodes (both children, if
  ///   they are on the same level. Otherwise, only the first-seen node),
  ///   returns whether (a) a node should be output and its two children to
  ///   recurse to or (b) no node should be output (i.e. it is skipped) and what
  ///   child to forward the request to.
  ///
  /// - no_skip:
  ///   Constexpr boolean whether the strategy guarantees never to skip a level.
  ///   This shortcuts some boolean conditions at compile-time.
  ///
  /// This 'prod_policy' also should inherit the general policy for the
  /// decision_diagram used (i.e. bdd_policy in bdd/bdd.h, zdd_policy in
  /// zdd/zdd.h and so on). This provides the following functions
  ///
  /// - compute_cofactor:
  ///   Used to change the low and high children retrieved from the input during
  ///   the product construction.
  ///
  /// Other parameters are:
  ///
  /// \param in_i  DAGs to combine into one.
  ///
  /// \param op    Binary boolean operator to be applied. See data.h for the
  ///              ones directly provided by Adiar.
  ///
  /// \return      A class that inherits from __decision_diagram and describes
  ///              the product of the two given DAGs.
  //////////////////////////////////////////////////////////////////////////////
  template<typename prod_policy>
  typename prod_policy::unreduced_t product_construction(const typename prod_policy::reduced_t &in_1,
                                                         const typename prod_policy::reduced_t &in_2,
                                                         const bool_op &op)
  {
    if (in_1.file_ptr() == in_2.file_ptr()) {
      return prod_policy::resolve_same_file(in_1, in_2, op);
    }

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory::available()
      // Input streams
      - 2*node_stream<>::memory_usage()
      // Output stream
      - arc_writer::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::DATA_STRUCTURES;

    constexpr size_t data_structures_in_pq_2 =
      prod_priority_queue_2_t<memory::INTERNAL>::DATA_STRUCTURES;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2)) * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_1_memory_fits =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::memory_fits(pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      prod_priority_queue_2_t<memory::INTERNAL>::memory_fits(pq_2_internal_memory);

    const bool internal_only = memory::mode == memory::INTERNAL;
    const bool external_only = memory::mode == memory::EXTERNAL;

    const size_t pq_1_bound = std::min({__prod_ilevel_upper_bound<prod_policy, get_2level_cut, 2u>(in_1, in_2, op),
                                        __prod_2level_upper_bound<prod_policy>(in_1, in_2, op),
                                        __prod_ilevel_upper_bound<prod_policy>(in_1, in_2, op)});

    const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __prod_ilevel_upper_bound<prod_policy, get_1level_cut, 0u>(in_1, in_2, op);

    const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if(!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_product_construction.lpq.unbucketed++;
#endif
      return __product_construction<prod_policy,
                                    prod_priority_queue_1_t<0, memory::INTERNAL>,
                                    prod_priority_queue_2_t<memory::INTERNAL>>
        (in_1, in_2, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if(!external_only && max_pq_1_size <= pq_1_memory_fits
                             && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_product_construction.lpq.internal++;
#endif
      return __product_construction<prod_policy,
                                    prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>,
                                    prod_priority_queue_2_t<memory::INTERNAL>>
        (in_1, in_2, op, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_product_construction.lpq.external++;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __product_construction<prod_policy,
                                    prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory::EXTERNAL>,
                                    prod_priority_queue_2_t<memory::EXTERNAL>>
        (in_1, in_2, op, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_PRODUCT_CONSTRUCTION_H
