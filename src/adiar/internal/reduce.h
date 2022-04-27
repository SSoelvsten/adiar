#ifndef ADIAR_INTERNAL_REDUCE_H
#define ADIAR_INTERNAL_REDUCE_H

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/sorter.h>

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
  ///        the number of arcs to each sink.
  ////////////////////////////////////////////////////////////////////////////
  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  class reduce_priority_queue : public levelized_arc_priority_queue<arc_t, reduce_queue_label, reduce_queue_lt,
                                                                      sorter_template, priority_queue_template>
  {
  private:
    using inner_lpq = levelized_arc_priority_queue<arc_t, reduce_queue_label, reduce_queue_lt,
                                                   sorter_template, priority_queue_template>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of sinks (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t _sinks[2] = { 0u, 0u };

  public:
    reduce_priority_queue(const arc_file (&files) [1u], size_t memory_given, size_t max_size)
      : inner_lpq(files, memory_given, max_size)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an arc into the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void push(const arc_t &a)
    {
      _sinks[false] += is_false(a.target);
      _sinks[true]  += is_true(a.target);

      inner_lpq::push(a);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top arc on the current level and remove it.
    ////////////////////////////////////////////////////////////////////////////
    arc_t pull()
    {
      arc_t a = inner_lpq::pull();

      _sinks[false] -= is_false(a.target);
      _sinks[true]  -= is_true(a.target);

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
    /// \brief Number of sinks (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    const size_t& sinks(const bool sink_value) const
    {
      return _sinks[sink_value];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of arcs (across all levels) ignoring sinks.
    ////////////////////////////////////////////////////////////////////////////
    size_t size_without_sinks()
    {
      return inner_lpq::size() - _sinks[false] - _sinks[true];
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // For sorting for Reduction Rule 2 (and back again)
  struct reduce_node_children_lt
  {
    bool operator()(const node_t &a, const node_t &b)
    {
      return a.high > b.high ||
        (a.high == b.high && a.low > b.low) ||
        (a.high == b.high && a.low == b.low && a.uid > b.uid);
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
  /// \brief Merging priority queue with sink_arc stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename pq_t>
  inline arc_t __reduce_get_next(pq_t &reduce_pq, sink_arc_stream<> &sink_arcs)
  {
    if (!reduce_pq.can_pull()
        || (sink_arcs.can_pull() && sink_arcs.peek().source > reduce_pq.top().source)) {
      return sink_arcs.pull();
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

  template <typename dd_policy, typename pq_t, template<typename, typename> typename sorter_t>
  void __reduce_level(sink_arc_stream<> &sink_arcs,
                      node_arc_stream<> &node_arcs,
                      const label_t label,
                      pq_t &reduce_pq,
                      node_writer &out_writer,
                      const size_t sorters_memory,
                      const size_t level_width)
  {
    // Temporary file for Reduction Rule 1 mappings (opened later if need be)
    tpie::file_stream<mapping> red1_mapping;

    size_t red1_sinks[2] = { 0u, 0u };

    // Sorter to find Reduction Rule 2 mappings
    sorter_t<node_t, reduce_node_children_lt>
      child_grouping(sorters_memory, level_width, 2);

    sorter_t<mapping, reduce_uid_lt>
      red2_mapping(sorters_memory, level_width, 2);

    // Pull out all nodes from reduce_pq and sink_arcs for this level
    while ((sink_arcs.can_pull() && label_of(sink_arcs.peek().source) == label)
            || reduce_pq.can_pull()) {
      const arc_t e_high = __reduce_get_next(reduce_pq, sink_arcs);
      const arc_t e_low = __reduce_get_next(reduce_pq, sink_arcs);

      node_t n = node_of(e_low, e_high);

      // Apply Reduction rule 1
      ptr_t reduction_rule_ret = dd_policy::reduction_rule(n);
      if (reduction_rule_ret != n.uid) {
        // Open red1_mapping first (and create file on disk) when at least one
        // element is written to it.
        if (!red1_mapping.is_open()) { red1_mapping.open(); }
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_1++;
#endif
        red1_sinks[false] += is_false(reduction_rule_ret);
        red1_sinks[true] += is_true(reduction_rule_ret);

        red1_mapping.write({ n.uid, reduction_rule_ret });
      } else {
        child_grouping.push(n);
      }
    }

    // Count number of arcs that cross this level
    size_t one_level_cut[4] = { 0u, 0u, 0u, 0u };

    const size_t red1_internal = red1_mapping.is_open()
      ? red1_mapping.size() - red1_sinks[false] - red1_sinks[true]
      : 0u;

    __reduce_cut_add(one_level_cut,
                     reduce_pq.size_without_sinks() + red1_internal,
                     reduce_pq.sinks(false) + red1_sinks[false] + sink_arcs.unread(false),
                     reduce_pq.sinks(true) + red1_sinks[true] + sink_arcs.unread(true));

    // Sort and apply Reduction rule 2
    child_grouping.sort();

    id_t out_id = MAX_ID;
    node_t out_node = { NIL, NIL, NIL };

    while (child_grouping.can_pull()) {
      node_t next_node = child_grouping.pull();

      if (out_node.low != next_node.low || out_node.high != next_node.high) {
        out_node = create_node(label, out_id, next_node.low, next_node.high);
        out_writer.unsafe_push(out_node);

        adiar_debug(out_id > 0, "Has run out of ids");
        out_id--;

        __reduce_cut_add(one_level_cut,
                         is_node(out_node.low) + is_node(out_node.high),
                         is_false(out_node.low) + is_false(out_node.high),
                         is_true(out_node.low) + is_true(out_node.high));
      } else {
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_2++;
#endif
      }

      red2_mapping.push({ next_node.uid, out_node.uid });
    }

    if (!is_nil(out_node.uid)) {
      out_writer.unsafe_push(create_level_info(label, MAX_ID - out_id));
    }

    // Update with 1-level cut below current level
    out_writer.inc_1level_cut(one_level_cut);

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
        // The is_high flag is included in arc_t.source
        arc_t new_arc = { node_arcs.pull().source, current_map.new_uid };
        reduce_pq.push(new_arc);
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

    if (!reduce_pq.empty()) {
      adiar_debug(!sink_arcs.can_pull() || label_of(sink_arcs.peek().source) < label,
                  "All sink arcs for 'label' should be processed");

      adiar_debug(!node_arcs.can_pull() || label_of(node_arcs.peek().target) < label,
                  "All node arcs for 'label' should be processed");

      adiar_debug(reduce_pq.empty() || !reduce_pq.can_pull(),
                  "All forwarded arcs for 'label' should be processed");

      if (sink_arcs.can_pull()) {
        reduce_pq.setup_next_level(label_of(sink_arcs.peek().source));
      } else {
        reduce_pq.setup_next_level();
      }
    } else if (!out_writer.has_pushed()) {
      adiar_debug(!node_arcs.can_pull() && !sink_arcs.can_pull(),
                  "All nodes should be processed at this point");

      adiar_debug(reduce_pq.empty(),
                  "Nothing has been pushed to a 'parent'");

      adiar_debug(!out_writer.has_pushed(),
                  "No nodes are pushed when it collapses to a sink");

      out_writer.push({ next_red1.new_uid, NIL, NIL });
    }
  }

  template<typename dd_policy, typename pq_t>
  typename dd_policy::reduced_t __reduce(const arc_file &in_file,
                                         const size_t lpq_memory, const size_t sorters_memory)
  {
#ifdef ADIAR_STATS
    stats_reduce.sum_node_arcs += in_file->_files[0].size();
    stats_reduce.sum_sink_arcs += in_file->_files[1].size();
#endif

    node_arc_stream<> node_arcs(in_file);
    sink_arc_stream<> sink_arcs(in_file);
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
      const arc_t e_high = sink_arcs.pull();
      const arc_t e_low = sink_arcs.pull();

      // Apply reduction rule 1, if applicable
      const ptr_t reduction_rule_ret = dd_policy::reduction_rule(node_of(e_low,e_high));
      if (reduction_rule_ret != e_low.source) {
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_1++;
#endif
        const bool sink_val = value_of(reduction_rule_ret);
        const node_t out_node = create_sink(sink_val);
        out_writer.push(out_node);

        __reduce_cut_add(out_file->max_1level_cut, 0u, !sink_val, sink_val);
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

    // Initialize (levelized) priority queue
    pq_t reduce_pq({in_file}, lpq_memory, in_file->max_1level_cut);

    const size_t internal_sorter_can_fit = internal_sorter<node_t>::memory_fits(sorters_memory / 2);

    // Process bottom-up each level
    while (sink_arcs.can_pull() || !reduce_pq.empty()) {
      const level_info_t current_level_info = level_info.pull();
      const label_t label = label_of(current_level_info);

      adiar_invariant(!reduce_pq.has_current_level() || label == reduce_pq.current_level(),
                      "label and priority queue should be in sync");

      const size_t level_width = width_of(current_level_info);

      if(level_width <= internal_sorter_can_fit) {
        __reduce_level<dd_policy, pq_t, internal_sorter>
          (sink_arcs, node_arcs, label, reduce_pq, out_writer, sorters_memory, level_width);
      } else {
        __reduce_level<dd_policy, pq_t, external_sorter>
          (sink_arcs, node_arcs, label, reduce_pq, out_writer, sorters_memory, level_width);
      }
    }

    // Compute final maximum i-level cuts
    const size_t number_of_nodes = out_writer.size();

    // Upper bound for any cut based on number of internal nodes.
    const cut_t max_cut = number_of_nodes < MAX_CUT ? number_of_nodes + 1 : MAX_CUT;

    // Upper bound on just 'all arcs'
    const size_t number_of_arcs = 2u * number_of_nodes;
    const bool noa_overflow = number_of_nodes <= MAX_CUT / 2u;

    const size_t number_of_false = out_file->number_of_sinks[false];
    const size_t number_of_true = out_file->number_of_sinks[true];

    const cuts_t all_arcs_cut = {
      noa_overflow ? number_of_arcs - number_of_false - number_of_true : MAX_CUT,
      noa_overflow ? number_of_arcs - number_of_true                   : MAX_CUT,
      noa_overflow ? number_of_arcs - number_of_false                  : MAX_CUT,
      noa_overflow ? number_of_arcs                                    : MAX_CUT
    };

    // 1-level cuts
    if (is_sink(out_file)) { // Sink case
      const bool sink_val = value_of(out_file);

      out_file->max_1level_cut[cut_type::INTERNAL]       = 0u;
      out_file->max_1level_cut[cut_type::INTERNAL_FALSE] = !sink_val;
      out_file->max_1level_cut[cut_type::INTERNAL_TRUE]  = sink_val;
      out_file->max_1level_cut[cut_type::ALL]            = 1u;
    } else if (out_writer.size() == 1) { // Single node case
      out_file->max_1level_cut[cut_type::INTERNAL] = 1u;

      out_file->max_1level_cut[cut_type::INTERNAL_FALSE] =
        std::max(out_file->number_of_sinks[false], 1lu);

      out_file->max_1level_cut[cut_type::INTERNAL_TRUE] =
        std::max(out_file->number_of_sinks[true], 1lu);

      out_file->max_1level_cut[cut_type::ALL] = 2u;
    } else { // General case
      for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
        // Use smallest sound upper bound.
        out_file->max_1level_cut[ct] = std::min({
            out_file->max_1level_cut[ct], max_cut, all_arcs_cut[ct]
          });
      }
    }

    // 2-level cuts
    if (is_sink(out_file) || out_writer.size() == 1) { // Single vertex cases
      for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
        out_file->max_2level_cut[ct] = out_file->max_1level_cut[ct];
      }
    } else { // General case
      for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
        // Upper bound based on 1-level cut
        const cut_t ub_from_1level_cut = out_file->max_1level_cut[ct] < MAX_CUT / 3u && ct == cut_type::INTERNAL
          ? (out_file->max_1level_cut[cut_type::INTERNAL] * 3) / 2
          : out_file->max_1level_cut[ct] < MAX_CUT / 2u
          ? out_file->max_1level_cut[ct] + out_file->max_1level_cut[cut_type::INTERNAL]
          : MAX_CUT;

        // TODO: If the number of nodes match the number of levels, then it is
        //       exactly the 1-level cut.

        // Use smallest sound upper bound.
        out_file->max_2level_cut[ct] = std::min({
            ub_from_1level_cut, max_cut, all_arcs_cut[ct]
          });
      }
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
      - node_arc_stream<>::memory_usage() - sink_arc_stream<>::memory_usage() - level_info_stream<arc_t>::memory_usage()
      // Output streams
      - node_writer::memory_usage();

    const size_t lpq_memory = aux_available_memory / 2;
    const size_t sorters_memory = aux_available_memory - lpq_memory - __tpie_file_stream_memory_usage<mapping>();

    const size_t max_cut = in_file->max_1level_cut;
    const tpie::memory_size_type lpq_memory_fits =
      reduce_priority_queue<internal_sorter, internal_priority_queue>::memory_fits(lpq_memory);

    if(max_cut <= lpq_memory_fits) {
#ifdef ADIAR_STATS
        stats_reduce.lpq_internal++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<internal_sorter, internal_priority_queue>>
        (in_file, lpq_memory, sorters_memory);
    } else {
#ifdef ADIAR_STATS
        stats_reduce.lpq_external++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<external_sorter, external_priority_queue>>
        (in_file, lpq_memory, sorters_memory);
    }
  }
}

#endif // ADIAR_INTERNAL_REDUCE_H
