#ifndef ADIAR_INTERNAL_REDUCE_H
#define ADIAR_INTERNAL_REDUCE_H

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/levelized_priority_queue.h>
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

  template<template<typename, typename> typename sorter_template,
           template<typename, typename> typename priority_queue_template>
  using reduce_priority_queue_t =
    levelized_arc_priority_queue<arc_t, reduce_queue_label, reduce_queue_lt,
                                 sorter_template, priority_queue_template>;

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

  // Merging priority queue with sink_arc stream
  template <typename pq_t>
  inline arc_t __reduce_get_next(pq_t &reduce_pq, sink_arc_stream<false> &sink_arcs)
  {
    if (!reduce_pq.can_pull()
        || (sink_arcs.can_pull() && sink_arcs.peek().source > reduce_pq.top().source)) {
      return sink_arcs.pull();
    } else {
      return reduce_pq.pull();
    }
  }

  template <typename dd_policy, typename pq_t, template<typename, typename> typename sorter_t>
  void __reduce_level(sink_arc_stream<> &sink_arcs,
                      node_arc_stream<> &node_arcs,
                      pq_t &reduce_pq,
                      label_t &label,
                      node_writer &out_writer,
                      const tpie::memory_size_type available_memory,
                      const size_t level_width)
  {
    // Temporary file for Reduction Rule 1 mappings (opened later if need be)
    tpie::file_stream<mapping> red1_mapping;

    // Sorter to find Reduction Rule 2 mappings
    sorter_t<node_t, reduce_node_children_lt>
      child_grouping(available_memory, level_width, 2);

    sorter_t<mapping, reduce_uid_lt>
      red2_mapping(available_memory, level_width, 2);

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
        red1_mapping.write({ n.uid, reduction_rule_ret });
      } else {
        child_grouping.push(n);
      }
    }
    // Sort and apply Reduction rule 2
    child_grouping.sort();

    if (child_grouping.can_pull()) {
      // Set up for remapping, keeping the very first node seen
      id_t out_id = MAX_ID;
      node_t current_node = child_grouping.pull();

      adiar_debug(out_id > 0, "Has run out of ids");

      node_t out_node = create_node(label, out_id--, current_node.low, current_node.high);
      out_writer.unsafe_push(out_node);

      red2_mapping.push({ current_node.uid, out_node.uid });

      // Keep the first node with different children than prior, and remap all
      // the later that match its children.
      while (child_grouping.can_pull()) {
        node_t next_node = child_grouping.pull();
        if (current_node.low == next_node.low && current_node.high == next_node.high) {
#ifdef ADIAR_STATS_EXTRA
          stats_reduce.removed_by_rule_2++;
#endif
          red2_mapping.push({ next_node.uid, out_node.uid });
        } else {
          current_node = next_node;

          out_node = create_node(label, out_id, current_node.low, current_node.high);
          out_writer.unsafe_push(out_node);
          out_id--;

          red2_mapping.push({current_node.uid, out_node.uid});
        }
      }

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
        // The is_high flag is included in arc_t..source
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

      label = !reduce_pq.empty_level()
        ? reduce_pq.current_level()
        : label_of(sink_arcs.peek().source);

    } else if (!out_writer.has_pushed()) {
      adiar_debug(!node_arcs.can_pull() && !sink_arcs.can_pull(),
                  "Nodes are still left to be processed");

      adiar_debug(reduce_pq.empty(),
                  "Nothing has been pushed to a 'parent'");

      adiar_debug(!out_writer.has_pushed(),
                  "No nodes are pushed when it collapses to a sink");

      out_writer.unsafe_push({ next_red1.new_uid, NIL, NIL });
    }
  }

  template<typename dd_policy, typename pq_t>
  void __reduce(const arc_file &in_file,
                node_arc_stream<> &node_arcs,
                sink_arc_stream<> &sink_arcs,
                level_info_stream<arc_t> &level_info,
                node_writer &out_writer,
                const tpie::memory_size_type available_memory)
  {
    pq_t reduce_pq({in_file}, available_memory / 2, in_file._file_ptr->max_1level_cut);

    // Find the first label
    // TODO take from level info instead
    label_t label = label_of(sink_arcs.peek().source);

    // Process bottom-up each level
    while (sink_arcs.can_pull() || !reduce_pq.empty()) {
      adiar_invariant(!reduce_pq.has_current_level() || label == reduce_pq.current_level(),
                      "label and priority queue should be in sync");

      const level_info_t current_level_info = level_info.pull();
      const size_t level_width = width_of(current_level_info);
      const size_t level_memory = available_memory / 2;

      const size_t internal_sorter_can_fit = internal_sorter<node_t>::memory_fits(level_memory / 2);

      if(level_width <= internal_sorter_can_fit) {
        __reduce_level<dd_policy, pq_t, internal_sorter>
          (sink_arcs, node_arcs, reduce_pq, label, out_writer, level_memory, level_width);
      } else {
        __reduce_level<dd_policy, pq_t, external_sorter>
          (sink_arcs, node_arcs, reduce_pq, label, out_writer, level_memory, level_width);
      }

    }
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

#ifdef ADIAR_STATS
    stats_reduce.sum_node_arcs += in_file._file_ptr -> _files[0].size();
    stats_reduce.sum_sink_arcs += in_file._file_ptr -> _files[1].size();
#endif

    node_arc_stream<> node_arcs(in_file);
    sink_arc_stream<> sink_arcs(in_file);
    level_info_stream<arc_t> level_info(in_file);

    // Set up output
    node_file out_file;
    out_file._file_ptr -> canonical = true;

    node_writer out_writer(out_file);

    // Trivial single-node case
    if (!node_arcs.can_pull()) {
      const arc_t e_high = sink_arcs.pull();
      const arc_t e_low = sink_arcs.pull();

      // Apply reduction rule 1, if applicable
      ptr_t reduction_rule_ret = dd_policy::reduction_rule(node_of(e_low,e_high));
      if (reduction_rule_ret != e_low.source) {
#ifdef ADIAR_STATS_EXTRA
        stats_reduce.removed_by_rule_1++;
#endif
        out_writer.unsafe_push(create_sink(value_of(reduction_rule_ret)));
      } else {
        label_t label = label_of(e_low.source);
        out_writer.unsafe_push(create_node(label, MAX_ID,
                                           e_low.target,
                                           e_high.target));

        out_writer.unsafe_push(create_level_info(label,1u));
      }
      return out_file;
    }

    // Derive an upper bound on the size of auxiliary data structures and check
    // whether we can run them with a faster internal memory variant.
    const tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    const size_t max_cut = in_file._file_ptr->max_1level_cut;
    const tpie::memory_size_type lpq_memory_fits =
      reduce_priority_queue_t<internal_sorter, internal_priority_queue>::memory_fits(available_memory / 2);

    if(max_cut <= lpq_memory_fits) {
      __reduce<dd_policy, reduce_priority_queue_t<internal_sorter, internal_priority_queue>>
        (in_file, node_arcs, sink_arcs, level_info, out_writer, available_memory);
    } else {
      __reduce<dd_policy, reduce_priority_queue_t<external_sorter, external_priority_queue>>
        (in_file, node_arcs, sink_arcs, level_info, out_writer, available_memory);
    }

    return out_file;
  }
}

#endif // ADIAR_INTERNAL_REDUCE_H
