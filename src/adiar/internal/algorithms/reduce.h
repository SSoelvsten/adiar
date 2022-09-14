#ifndef ADIAR_INTERNAL_ALGORITHMS_REDUCE_H
#define ADIAR_INTERNAL_ALGORITHMS_REDUCE_H

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/memory.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/convert.h>

#include <adiar/statistics.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern stats_t::reduce_t stats_reduce;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct mapping
  {
    uid_t old_uid;
    uid_t new_uid;
  };

  //////////////////////////////////////////////////////////////////////////////
  // For priority queue
  struct reduce_queue_lt
  {
    bool operator()(const arc_t &a, const arc_t &b)
    {
      // We want the high arc first, but that is already placed on the
      // least-significant bit on the source variable.
      return a.source > b.source;
    }
  };

  struct reduce_queue_label
  {
    static label_t label_of(const arc_t &a)
    {
      return adiar::label_of(a.source);
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Decorator on the levelized priority queue to also keep track of
  ///        the number of arcs to each terminal.
  ////////////////////////////////////////////////////////////////////////////
  template<size_t LOOK_AHEAD, memory::memory_mode mem_mode>
  class reduce_priority_queue : public levelized_arc_priority_queue<arc_t, reduce_queue_label,
                                                                    reduce_queue_lt, LOOK_AHEAD,
                                                                    mem_mode>
  {
  private:
    using inner_lpq = levelized_arc_priority_queue<arc_t, reduce_queue_label,
                                                   reduce_queue_lt, LOOK_AHEAD,
                                                   mem_mode>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t _terminals[2] = { 0u, 0u };

  public:
    reduce_priority_queue(const arc_file (&files) [1u], size_t memory_given, size_t max_size)
      : inner_lpq(files, memory_given, max_size, stats_reduce.lpq)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an arc into the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void push(const arc_t &a)
    {
      _terminals[false] += is_false(a.target);
      _terminals[true]  += is_true(a.target);

      inner_lpq::push(a);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top arc on the current level and remove it.
    ////////////////////////////////////////////////////////////////////////////
    arc_t pull()
    {
      arc_t a = inner_lpq::pull();

      _terminals[false] -= is_false(a.target);
      _terminals[true]  -= is_true(a.target);

      return a;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top arc on the current level.
    ////////////////////////////////////////////////////////////////////////////
    void pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    const size_t& terminals(const bool terminal_value) const
    {
      return _terminals[terminal_value];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of arcs (across all levels) ignoring terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t size_without_terminals()
    {
      return inner_lpq::size() - _terminals[false] - _terminals[true];
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Reduction Rule 2 sorting (and back again)
  struct reduce_node_children_lt
  {
    bool operator()(const node_t &a, const node_t &b)
    {
      // If adding attributed edges, i.e. complement edges:
      //     Use the 'flag' bit on children to mark attributed edges. Currently,
      //     we use this flag to mark whether Reduction Rule 1 was applied to
      //     some node across some arc.
      const ptr_t a_high = unflag(a.high);
      const ptr_t a_low = unflag(a.low);

      const ptr_t b_high = unflag(b.high);
      const ptr_t b_low = unflag(b.low);

      return a_high > b_high || (a_high == b_high && a_low > b_low)
#ifndef NDEBUG
        || (a_high == b_high && a_low == b_low && a.uid > b.uid)
#endif
        ;
    }
  };

  struct reduce_uid_lt
  {
    bool operator()(const mapping &a, const mapping &b)
    {
      return a.old_uid > b.old_uid;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Merging priority queue with terminal_arc stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename pq_t>
  inline arc_t __reduce_get_next(pq_t &reduce_pq, terminal_arc_stream<> &terminal_arcs)
  {
    if (!reduce_pq.can_pull()
        || (terminal_arcs.can_pull() && terminal_arcs.peek().source > reduce_pq.top().source)) {
      return terminal_arcs.pull();
    } else {
      return reduce_pq.pull();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Update a cut size with some number of arcs.
  //////////////////////////////////////////////////////////////////////////////
  inline void __reduce_cut_add(size_t (&cut)[4],
                               const size_t internal_arcs,
                               const size_t false_arcs,
                               const size_t true_arcs)
  {
    cut[cut_type::INTERNAL]       += internal_arcs;
    cut[cut_type::INTERNAL_FALSE] += internal_arcs + false_arcs;
    cut[cut_type::INTERNAL_TRUE]  += internal_arcs + true_arcs;
    cut[cut_type::ALL]            += internal_arcs + false_arcs + true_arcs;
  }

  inline void __reduce_cut_add(size_t (&cut)[4], const ptr_t target)
  {
    cut[cut_type::INTERNAL]       += is_node(target);
    cut[cut_type::INTERNAL_FALSE] += is_node(target) + is_false(target);
    cut[cut_type::INTERNAL_TRUE]  += is_node(target) + is_true(target);
    cut[cut_type::ALL]            += 1u;
  }

  template <typename dd_policy, typename pq_t, template<typename, typename> typename sorter_t>
  void __reduce_level(terminal_arc_stream<> &terminal_arcs,
                      node_arc_stream<> &node_arcs,
                      const label_t label,
                      pq_t &reduce_pq,
                      node_writer &out_writer,
                      cuts_t &global_1level_cut,
                      const size_t sorters_memory,
                      const size_t level_width)
  {
    // Temporary file for Reduction Rule 1 mappings (opened later if need be)
    tpie::file_stream<mapping> red1_mapping;

    // Sorter to find Reduction Rule 2 mappings
    sorter_t<node_t, reduce_node_children_lt>
      child_grouping(sorters_memory, level_width, 2);

    sorter_t<mapping, reduce_uid_lt>
      red2_mapping(sorters_memory, level_width, 2);

    // Pull out all nodes from reduce_pq and terminal_arcs for this level
    while ((terminal_arcs.can_pull() && label_of(terminal_arcs.peek().source) == label)
            || reduce_pq.can_pull()) {
      const arc_t e_high = __reduce_get_next(reduce_pq, terminal_arcs);
      const arc_t e_low = __reduce_get_next(reduce_pq, terminal_arcs);

      node_t n = node_of(e_low, e_high);

      // Apply Reduction rule 1
      ptr_t reduction_rule_ret = dd_policy::reduction_rule(n);
      if (reduction_rule_ret != n.uid) {
        // Open red1_mapping first (and create file on disk) when at least one
        // element is written to it.
        if (!red1_mapping.is_open()) { red1_mapping.open(); }
#ifdef ADIAR_STATS
        stats_reduce.removed_by_rule_1++;
#endif
        red1_mapping.write({ n.uid, reduction_rule_ret });
      } else {
        child_grouping.push(n);
      }
    }

    // Count number of arcs that cross this level
    cuts_t local_1level_cut = { 0u, 0u, 0u, 0u };

    __reduce_cut_add(local_1level_cut,
                     reduce_pq.size_without_terminals(),
                     reduce_pq.terminals(false) + terminal_arcs.unread(false),
                     reduce_pq.terminals(true) + terminal_arcs.unread(true));

    // Sort and apply Reduction rule 2
    child_grouping.sort();

    id_t out_id = MAX_ID;
    node_t out_node = { NIL, NIL, NIL };

    while (child_grouping.can_pull()) {
      const node_t next_node = child_grouping.pull();

      if (out_node.low != unflag(next_node.low) || out_node.high != unflag(next_node.high)) {
        out_node = create_node(label, out_id, unflag(next_node.low), unflag(next_node.high));
        out_writer.unsafe_push(out_node);

        adiar_debug(out_id > 0, "Has run out of ids");
        out_id--;

        __reduce_cut_add(is_flagged(next_node.low) ? global_1level_cut : local_1level_cut,
                         out_node.low);
        __reduce_cut_add(is_flagged(next_node.high) ? global_1level_cut : local_1level_cut,
                         out_node.high);
      } else {
#ifdef ADIAR_STATS
        stats_reduce.removed_by_rule_2++;
#endif
      }

      red2_mapping.push({ next_node.uid, out_node.uid });
    }

    if (!is_nil(out_node.uid)) {
      out_writer.unsafe_push(create_level_info(label, MAX_ID - out_id));
    }

    // Sort mappings for Reduction rule 2 back in order of node_arcs
    red2_mapping.sort();

    // Merging of red1_mapping and red2_mapping
    mapping next_red1 = {0, 0};
    bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
    if (has_next_red1) {
      red1_mapping.seek(0);
      next_red1 = red1_mapping.read();
    }

    mapping next_red2 = {0, 0};
    bool has_next_red2 = red2_mapping.can_pull();
    if (has_next_red2) {
      next_red2 = red2_mapping.pull();
    }

    // Pass all the mappings to Q
    while (has_next_red1 || has_next_red2) {
      // Find the mapping with largest old_uid
      bool is_red1_current = !has_next_red2 ||
                              (has_next_red1 && next_red1.old_uid > next_red2.old_uid);
      mapping current_map = is_red1_current ? next_red1 : next_red2;

      adiar_invariant(!node_arcs.can_pull() || current_map.old_uid == node_arcs.peek().target,
                      "Mapping forwarded in sync with node_arcs");

      // Find all arcs that have sources that match the current mapping's old_uid
      while (node_arcs.can_pull() && current_map.old_uid == node_arcs.peek().target) {
        // The is_high flag is included in arc_t.source pulled from node_arcs.
        ptr_t s = node_arcs.pull().source;

        // If Reduction Rule 1 was used, then tell the parents to add to the global cut.
        ptr_t t = is_red1_current ? flag(current_map.new_uid) : current_map.new_uid;

        reduce_pq.push({ s,t });
      }

      // Update the mapping that was used
      if (is_red1_current) {
        has_next_red1 = red1_mapping.can_read();
        if (has_next_red1) {
          next_red1 = red1_mapping.read();
        }
      } else {
        has_next_red2 = red2_mapping.can_pull();
        if (has_next_red2) {
          next_red2 = red2_mapping.pull();
        }
      }
    }

    // Move on to the next level
    red1_mapping.close();

    // Update with 1-level cut below current level
    out_writer.inc_1level_cut(local_1level_cut);

    if (!reduce_pq.empty()) {
      adiar_debug(!terminal_arcs.can_pull() || label_of(terminal_arcs.peek().source) < label,
                  "All terminal arcs for 'label' should be processed");

      adiar_debug(!node_arcs.can_pull() || label_of(node_arcs.peek().target) < label,
                  "All node arcs for 'label' should be processed");

      adiar_debug(reduce_pq.empty() || !reduce_pq.can_pull(),
                  "All forwarded arcs for 'label' should be processed");

      if (terminal_arcs.can_pull()) {
        reduce_pq.setup_next_level(label_of(terminal_arcs.peek().source));
      } else {
        reduce_pq.setup_next_level();
      }
    } else if (!out_writer.has_pushed()) {
      adiar_debug(!node_arcs.can_pull() && !terminal_arcs.can_pull(),
                  "All nodes should be processed at this point");

      adiar_debug(reduce_pq.empty(),
                  "Nothing has been pushed to a 'parent'");

      adiar_debug(!out_writer.has_pushed(),
                  "No nodes are pushed when it collapses to a terminal");

      const bool terminal_val = value_of(next_red1.new_uid);

      out_writer.unsafe_push(create_terminal(terminal_val));
      out_writer.set_number_of_terminals(!terminal_val, terminal_val);
    }
  }

  template<typename dd_policy, typename pq_t>
  typename dd_policy::reduced_t __reduce(const arc_file &in_file,
                                         const size_t lpq_memory, const size_t sorters_memory)
  {
#ifdef ADIAR_STATS
    stats_reduce.sum_node_arcs += in_file->_files[0].size();
    stats_reduce.sum_terminal_arcs += in_file->_files[1].size();
#endif

    node_arc_stream<> node_arcs(in_file);
    terminal_arc_stream<> terminal_arcs(in_file);
    level_info_stream<arc_t> level_info(in_file);

    // Set up output
    node_file out_file;
    out_file->canonical = true;

    out_file->max_1level_cut[cut_type::INTERNAL]       = 0u;
    out_file->max_1level_cut[cut_type::INTERNAL_FALSE] = 0u;
    out_file->max_1level_cut[cut_type::INTERNAL_TRUE]  = 0u;
    out_file->max_1level_cut[cut_type::ALL]            = 0u;

    node_writer out_writer(out_file);

    // Trivial single-node case
    if (!node_arcs.can_pull()) {
      const arc_t e_high = terminal_arcs.pull();
      const arc_t e_low = terminal_arcs.pull();

      // Apply reduction rule 1, if applicable
      const ptr_t reduction_rule_ret = dd_policy::reduction_rule(node_of(e_low,e_high));
      if (reduction_rule_ret != e_low.source) {
#ifdef ADIAR_STATS
        stats_reduce.removed_by_rule_1++;
#endif
        const bool terminal_val = value_of(reduction_rule_ret);
        const node_t out_node = create_terminal(terminal_val);
        out_writer.unsafe_push(out_node);

        out_writer.set_number_of_terminals(!terminal_val, terminal_val);
        __reduce_cut_add(out_file->max_1level_cut, 0u, !terminal_val, terminal_val);
      } else {
        const label_t label = label_of(e_low.source);

        out_writer.unsafe_push(create_node(label, MAX_ID, e_low.target, e_high.target));

        out_writer.unsafe_push(create_level_info(label,1u));

        out_file->max_1level_cut[cut_type::INTERNAL] = 1u;

        out_file->max_1level_cut[cut_type::INTERNAL_FALSE] =
          std::max(!value_of(e_low.target) + !value_of(e_high.target), 1);

        out_file->max_1level_cut[cut_type::INTERNAL_TRUE] =
          std::max(value_of(e_low.target) + value_of(e_high.target), 1);

        out_file->max_1level_cut[cut_type::ALL]      = 2u;
      }

      // Copy over 1-level cut to 2-level cut.
      for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
        out_file->max_2level_cut[ct] = out_file->max_1level_cut[ct];
      }

      return out_file;
    }

    // Cut for arcs that suddenly cross much further down than they did
    // initially in the input, e.g. when a node was removed due to Reduction
    // Rule 1.
    cuts_t global_1level_cut = { 0u, 0u, 0u, 0u };

    // Initialize (levelized) priority queue
    pq_t reduce_pq({in_file}, lpq_memory, in_file->max_1level_cut);

    const size_t internal_sorter_can_fit = internal_sorter<node_t>::memory_fits(sorters_memory / 2);

    // Process bottom-up each level
    while (terminal_arcs.can_pull() || !reduce_pq.empty()) {
      const level_info_t current_level_info = level_info.pull();
      const label_t label = label_of(current_level_info);

      adiar_invariant(!reduce_pq.has_current_level() || label == reduce_pq.current_level(),
                      "label and priority queue should be in sync");

      const size_t level_width = width_of(current_level_info);

      if(level_width <= internal_sorter_can_fit) {
        __reduce_level<dd_policy, pq_t, internal_sorter>
          (terminal_arcs, node_arcs, label, reduce_pq, out_writer, global_1level_cut, sorters_memory, level_width);
      } else {
        __reduce_level<dd_policy, pq_t, external_sorter>
          (terminal_arcs, node_arcs, label, reduce_pq, out_writer, global_1level_cut, sorters_memory, level_width);
      }
    }

    // Add global_1level_cut to the maximum 1-level cut already present in 'out_file'.
    for (size_t ct = 0u; ct < CUT_TYPES; ct++) {
      out_file->max_1level_cut[ct] += global_1level_cut[ct];
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based decision diagram.
  ///
  /// \param dd_policy Which includes the types and the reduction rule
  /// \param arc_file The unreduced bdd in its arc-based representation
  ///
  /// \return The reduced decision diagram in a node-based representation
  //////////////////////////////////////////////////////////////////////////////
  template<typename dd_policy>
  typename dd_policy::reduced_t reduce(const typename dd_policy::unreduced_t &input)
  {
    adiar_debug(!input.empty(), "Input for Reduce should always be non-empty");

    // Is it already reduced?
    if (input.template has<node_file>()) {
      return typename dd_policy::reduced_t(input.template get<node_file>(), input.negate);
    }

    // Get unreduced input
    const arc_file in_file = input.template get<arc_file>();

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory::available()
      // Input streams
      - node_arc_stream<>::memory_usage() - terminal_arc_stream<>::memory_usage() - level_info_stream<arc_t>::memory_usage()
      // Output streams
      - node_writer::memory_usage();

    const size_t pq_memory = aux_available_memory / 2;
    const size_t sorters_memory = aux_available_memory - pq_memory - tpie::file_stream<mapping>::memory_usage();

    const tpie::memory_size_type pq_memory_fits =
      reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>::memory_fits(pq_memory);

    const bool internal_only = memory::mode == memory::INTERNAL;
    const bool external_only = memory::mode == memory::EXTERNAL;

    const size_t pq_bound = in_file->max_1level_cut;

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.unbucketed++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>>
        (in_file, pq_memory, sorters_memory);
    } else if(!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.internal++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory::INTERNAL>>
        (in_file, pq_memory, sorters_memory);
    } else {
#ifdef ADIAR_STATS
      stats_reduce.lpq.external++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory::EXTERNAL>>
        (in_file, pq_memory, sorters_memory);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_REDUCE_H
