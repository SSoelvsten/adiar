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
      : inner_lpq(files, memory_given, max_size, stats_reduce.lpq)
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
  struct node_with_indegree : public node_t
  {
    node_with_indegree() = default;

    node_with_indegree(const node_t &n)
      : node_t(n)
    { }

    node_with_indegree(const arc_t &e_low, const arc_t &e_high)
      : node_t(node_of(e_low, e_high))
    { }

    size_t in_degree = 0u;
  };

  struct reduce_node_children_lt
  {
    bool operator()(const node_t &a, const node_t &b)
    {
      return a.high > b.high ||
        (a.high == b.high && a.low > b.low) ||
        (a.high == b.high && a.low == b.low && a.uid > b.uid);
    }
  };

  struct mapping
  {
    uid_t old_uid;
    uid_t new_uid;
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
  /// \brief Sets a cut to be based off a given number of arcs.
  //////////////////////////////////////////////////////////////////////////////
  inline void __reduce_cut_set(cuts_t &cut,
                               const size_t internal_arcs,
                               const size_t false_arcs,
                               const size_t true_arcs)
  {
    cut[cut_type::INTERNAL]       = internal_arcs;
    cut[cut_type::INTERNAL_FALSE] = internal_arcs + false_arcs;
    cut[cut_type::INTERNAL_TRUE]  = internal_arcs + true_arcs;
    cut[cut_type::ALL]            = internal_arcs + false_arcs + true_arcs;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Update a cut size with the arcs from another cut
  //////////////////////////////////////////////////////////////////////////////
  inline void __reduce_cut_add(cuts_t &cut, const cuts_t &other_cut)
  {
    cut[cut_type::INTERNAL]       += other_cut[cut_type::INTERNAL];
    cut[cut_type::INTERNAL_FALSE] += other_cut[cut_type::INTERNAL_FALSE];
    cut[cut_type::INTERNAL_TRUE]  += other_cut[cut_type::INTERNAL_TRUE];
    cut[cut_type::ALL]            += other_cut[cut_type::ALL];
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Update a cut size with the arcs from another cut or at least the
  ///        given minimum value.
  //////////////////////////////////////////////////////////////////////////////
  inline void __reduce_cut_add(cuts_t &cut, const cuts_t &other_cut, const size_t min_val)
  {
    __reduce_cut_add(cut,
                     {
                       std::max(other_cut[0], min_val),
                       std::max(other_cut[1], min_val),
                       std::max(other_cut[2], min_val),
                       std::max(other_cut[3], min_val),
                     });
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a single level.
  ///
  /// \sa __reduce
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_policy, typename pq_t, template<typename, typename> typename sorter_t>
  void __reduce_level(sink_arc_stream<> &sink_arcs,
                      node_arc_stream<> &node_arcs_1,
                      node_arc_stream<> &node_arcs_2,
                      const label_t label,
                      pq_t &reduce_pq,
                      node_writer &out_writer,
                      const size_t sorters_memory,
                      const size_t level_width)
  {
    // Temporary file for Reduction Rule 1 mappings (opened later if need be)
    tpie::file_stream<mapping> red1_mapping;

    // Sorter to find Reduction Rule 2 mappings
    sorter_t<node_with_indegree, reduce_node_children_lt>
      child_grouping(sorters_memory, level_width, 2);

    sorter_t<mapping, reduce_uid_lt>
      red2_mapping(sorters_memory, level_width, 2);

    // Pull out all nodes from reduce_pq and sink_arcs for this level
    while ((sink_arcs.can_pull() && label_of(sink_arcs.peek().source) == label)
            || reduce_pq.can_pull()) {
      const arc_t e_high = __reduce_get_next(reduce_pq, sink_arcs);
      const arc_t e_low = __reduce_get_next(reduce_pq, sink_arcs);

      node_with_indegree n(e_low, e_high);

      // Apply Reduction rule 1
      ptr_t reduction_rule_ret = dd_policy::reduction_rule(n);
      if (reduction_rule_ret != n.uid) {
        // Open red1_mapping first (and create file on disk) when at least one
        // element is written to it.
        if (!red1_mapping.is_open()) { red1_mapping.open(); }
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_1++;
#endif
        red1_mapping.write({ n.uid, reduction_rule_ret });
      } else {
        // Accumulate the number of in-going arcs to 'n' for later 2-level cut
        while (node_arcs_1.can_pull() && n.uid <= node_arcs_1.peek().target) {
          if (n.uid == node_arcs_1.pull().target) { n.in_degree++; }
        }

        child_grouping.push(n);
      }
    }

    // Initialise 1-level cut
    cuts_t one_level_cut = { 0u, 0u, 0u, 0u };

    __reduce_cut_set(one_level_cut,
                     reduce_pq.size_without_sinks(),
                     reduce_pq.sinks(false) + sink_arcs.unread(false),
                     reduce_pq.sinks(true) + sink_arcs.unread(true));

    // Initialise 2-level cut where we copy over the number of arcs crossing
    // this level (from 'one_level_cut').
    cuts_t two_level_cut = { 0u, 0u, 0u, 0u };
    for (size_t ct = 0u; ct < CUT_TYPES; ct++) {
      two_level_cut[ct] = one_level_cut[ct];
    }

    // Sort and apply Reduction rule 2
    child_grouping.sort();

    id_t out_id = MAX_ID;
    node_t out_node = { NIL, NIL, NIL };

    size_t out_node_in_degree  = 0u;
    cuts_t out_node_1level_cut = { 0u, 0u, 0u, 0u };

    while (child_grouping.can_pull()) {
      node_with_indegree next_node = child_grouping.pull();

      if (out_node.low != next_node.low || out_node.high != next_node.high) {
        // Update cuts based on output node stored inside of 'out_node' (if any)
        if (!is_nil(out_node.uid)) {
          __reduce_cut_add(one_level_cut, out_node_1level_cut);
          __reduce_cut_add(two_level_cut, out_node_1level_cut, out_node_in_degree);
        }

        // Output next node
        out_node = create_node(label, out_id, next_node.low, next_node.high);
        out_writer.unsafe_push(out_node);

        out_node_in_degree = next_node.in_degree;

        adiar_debug(out_id > 0, "Has run out of ids");
        out_id--;

        // Store 1-level cut below this node for later
        __reduce_cut_set(out_node_1level_cut,
                         is_node(out_node.low) + is_node(out_node.high),
                         is_false(out_node.low) + is_false(out_node.high),
                         is_true(out_node.low) + is_true(out_node.high));
      } else {
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_2++;
#endif
        out_node_in_degree += next_node.in_degree;
      }

      red2_mapping.push({ next_node.uid, out_node.uid });
    }

    if (!is_nil(out_node.uid)) {
      // Output level information on non-empty level
      out_writer.unsafe_push(create_level_info(label, MAX_ID - out_id));

      // Update cuts from last node that was output above
      __reduce_cut_add(one_level_cut, out_node_1level_cut);
      __reduce_cut_add(two_level_cut, out_node_1level_cut, out_node_in_degree);
    }

    // Sort mappings for Reduction rule 2 back in order of node_arcs_2
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

      adiar_invariant(!node_arcs_2.can_pull() || current_map.old_uid == node_arcs_2.peek().target,
                      "Mapping forwarded in sync with node_arcs_2");

      // Find all arcs that have sources that match the current mapping's old_uid
      size_t indegree = 0u;
      while (node_arcs_2.can_pull() && current_map.old_uid == node_arcs_2.peek().target) {
        // The is_high flag is included in arc_t.source
        arc_t new_arc = { node_arcs_2.pull().source, current_map.new_uid };
        reduce_pq.push(new_arc);
        indegree++;
      }

      // Update the mapping that was used
      if (is_red1_current) {
        cuts_t red1_arc_cut = {};
        __reduce_cut_set(red1_arc_cut,
                         is_node(next_red1.new_uid) * indegree,
                         is_false(next_red1.new_uid) * indegree,
                         is_true(next_red1.new_uid) * indegree);

        __reduce_cut_add(one_level_cut, red1_arc_cut);
        __reduce_cut_add(two_level_cut, red1_arc_cut);

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

    // Update with 1-level cut below current level and 2-level cut around current level
    out_writer.inc_1level_cut(one_level_cut);
    out_writer.inc_2level_cut(two_level_cut);

    if (!reduce_pq.empty()) {
      adiar_debug(!sink_arcs.can_pull() || label_of(sink_arcs.peek().source) < label,
                  "All sink arcs for 'label' should be processed");

      adiar_debug(!node_arcs_2.can_pull() || label_of(node_arcs_2.peek().target) < label,
                  "All node arcs for 'label' should be processed");

      adiar_debug(reduce_pq.empty() || !reduce_pq.can_pull(),
                  "All forwarded arcs for 'label' should be processed");

      if (sink_arcs.can_pull()) {
        reduce_pq.setup_next_level(label_of(sink_arcs.peek().source));
      } else {
        reduce_pq.setup_next_level();
      }
    } else if (!out_writer.has_pushed()) {
      adiar_debug(!node_arcs_2.can_pull() && !sink_arcs.can_pull(),
                  "All nodes should be processed at this point");

      adiar_debug(reduce_pq.empty(),
                  "Nothing has been pushed to a 'parent'");

      adiar_debug(!out_writer.has_pushed(),
                  "No nodes are pushed when it collapses to a sink");

      const bool sink_val = value_of(next_red1.new_uid);

      out_writer.unsafe_push(create_sink(sink_val));
      out_writer.set_number_of_sinks(!sink_val, sink_val);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given decision diagram with a semi-transposed arc
  ///        representation (in an <tt>arc_file</tt>).
  //////////////////////////////////////////////////////////////////////////////
  template<typename dd_policy, typename pq_t>
  typename dd_policy::reduced_t __reduce(const arc_file &in_file,
                                         const size_t lpq_memory, const size_t sorters_memory)
  {
#ifdef ADIAR_STATS
    stats_reduce.sum_node_arcs += in_file->_files[0].size();
    stats_reduce.sum_sink_arcs += in_file->_files[1].size();
#endif

    node_arc_stream<> node_arcs_1(in_file);
    node_arc_stream<> node_arcs_2(in_file);
    sink_arc_stream<> sink_arcs(in_file);
    level_info_stream<arc_t> level_info(in_file);

    // Set up output
    node_file out_file;
    out_file->canonical = true;

    out_file->max_1level_cut[cut_type::INTERNAL]       = 0u;
    out_file->max_1level_cut[cut_type::INTERNAL_FALSE] = 0u;
    out_file->max_1level_cut[cut_type::INTERNAL_TRUE]  = 0u;
    out_file->max_1level_cut[cut_type::ALL]            = 0u;

    out_file->max_2level_cut[cut_type::INTERNAL]       = 0u;
    out_file->max_2level_cut[cut_type::INTERNAL_FALSE] = 0u;
    out_file->max_2level_cut[cut_type::INTERNAL_TRUE]  = 0u;
    out_file->max_2level_cut[cut_type::ALL]            = 0u;

    node_writer out_writer(out_file);

    // Trivial single-node case
    if (!node_arcs_2.can_pull()) {
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
        out_writer.unsafe_push(out_node);

        out_writer.set_number_of_sinks(!sink_val, sink_val);

        __reduce_cut_set(out_file->max_1level_cut, 0u, !sink_val, sink_val);
        __reduce_cut_set(out_file->max_2level_cut, 0u, !sink_val, sink_val);
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
          (sink_arcs, node_arcs_1, node_arcs_2, label, reduce_pq, out_writer, sorters_memory, level_width);
      } else {
        __reduce_level<dd_policy, pq_t, external_sorter>
          (sink_arcs, node_arcs_1, node_arcs_2, label, reduce_pq, out_writer, sorters_memory, level_width);
      }
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given decision diagram.
  ///
  /// \param dd_policy Which includes the types and the reduction rule.
  /// \param input     A (possibly) unreduced decision diagram.
  ///
  /// \return The equivalent reduced decision diagram (as a node_file).
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
      - 2 * node_arc_stream<>::memory_usage() - sink_arc_stream<>::memory_usage() - level_info_stream<arc_t>::memory_usage()
      // Output streams
      - node_writer::memory_usage();

    const size_t lpq_memory = aux_available_memory / 2;
    const size_t sorters_memory = aux_available_memory - lpq_memory - tpie::file_stream<mapping>::memory_usage();

    const size_t max_cut = in_file->max_1level_cut;
    const tpie::memory_size_type lpq_memory_fits =
      reduce_priority_queue<internal_sorter, internal_priority_queue>::memory_fits(lpq_memory);

    if(max_cut <= lpq_memory_fits) {
#ifdef ADIAR_STATS
        stats_reduce.lpq.internal++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<internal_sorter, internal_priority_queue>>
        (in_file, lpq_memory, sorters_memory);
    } else {
#ifdef ADIAR_STATS
        stats_reduce.lpq.external++;
#endif
      return __reduce<dd_policy, reduce_priority_queue<external_sorter, external_priority_queue>>
        (in_file, lpq_memory, sorters_memory);
    }
  }
}

#endif // ADIAR_INTERNAL_REDUCE_H
