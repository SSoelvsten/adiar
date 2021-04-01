#ifndef ADIAR_REDUCE_CPP
#define ADIAR_REDUCE_CPP

#include "reduce.h"

#include <tpie/sort.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/levelized_priority_queue.h>

#include <adiar/assert.h>

namespace adiar
{
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
    label_t label_of(const arc_t &a)
    {
      return adiar::label_of(a.source);
    }
  };


  //////////////////////////////////////////////////////////////////////////////
  // For sorting for Reduction Rule 2 (and back again)
  const auto reduce_node_children_lt = [](const node_t &a, const node_t &b) -> bool {
    return a.high > b.high ||
           (a.high == b.high && a.low > b.low) ||
           (a.high == b.high && a.low == b.low && a.uid > b.uid);
  };

  const auto reduce_uid_lt = [](const mapping &a, const mapping &b) -> bool {
    return a.old_uid > b.old_uid;
  };

  typedef levelized_arc_priority_queue<arc_t, reduce_queue_label, reduce_queue_lt, std::greater<label_t>> reduce_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions

  // Merging priority queue with sink_arc stream
  inline arc_t reduce_get_next(reduce_priority_queue_t &reduce_pq,
                               sink_arc_stream<false> &sink_arcs)
  {
    if (!reduce_pq.can_pull() || (sink_arcs.can_pull() && sink_arcs.peek().source > reduce_pq.top().source)) {
      return sink_arcs.pull();
    } else {
      return reduce_pq.pull();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const arc_file &in_file)
  {
    node_file out_file;
    node_writer out_writer(out_file);

    // Set up
    node_arc_stream<> node_arcs(in_file);
    sink_arc_stream<> sink_arcs(in_file);

    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    tpie::dummy_progress_indicator pi;

    // Phase 1 and 3 of the sorters; pushing and popping data.
    size_t sorter_phase_1 = std::max((sizeof(node) * 128 * 1024 + 5 * 1024 * 1024),
                                     available_memory / 20); // ~8MB

    adiar_debug(2 * sorter_phase_1 < available_memory / 2,
               "Taking too much memory for the sorter blocks");

    // Phase 2 of the sorter; merge_sort on the data. Take...
    size_t sorter_phase_2 = std::max(sorter_phase_1 * 2, // at least 2x the first and third phase (~16 MB), or...
                                     std::min(// ...the rest of the memory
                                              available_memory / 2,
                                              // ...half of the input, plus then some
                                              sizeof(node_t) * (in_file.size() / 2))
                                     // Yet, minus the amount of space we have reserved for the other sorter
                                     - sorter_phase_1);

    adiar_debug(sorter_phase_1 < sorter_phase_2,
               "Not enough space for sorting phase");

    // The memory given to the priority queue may overlap on the sorter_phase_2,
    // but should not overlap with the two sorter_phase_1
    reduce_priority_queue_t reduce_pq({in_file}, available_memory - (2 * sorter_phase_1));

    // Check to see if node_arcs is empty
    if (!node_arcs.can_pull()) {
      arc_t e_high = sink_arcs.pull();
      arc_t e_low = sink_arcs.pull();

      // Apply reduction rule 1 if applicable
      if (e_high.target == e_low.target) {
        out_writer.unsafe_push(create_sink(value_of(e_low.target)));
      } else {
        label_t label = label_of(e_low.source);
        out_writer.unsafe_push(create_node(label, MAX_ID,
                                            e_low.target,
                                            e_high.target));

        out_writer.unsafe_push(meta_t { label });
      }

      return out_file;
    }

    // Find the first label
    label_t label = label_of(sink_arcs.peek().source);

    // Process bottom-up each level
    while (sink_arcs.can_pull() || reduce_pq.can_pull()) {
      adiar_invariant(label == reduce_pq.current_level(), "label and priority queue are in sync");

      // Temporary file for Reduction Rule 1 mappings (opened later if need be)
      tpie::file_stream<mapping> red1_mapping;

      // Sorter to find Reduction Rule 2 mappings
      tpie::merge_sorter<node_t, false, decltype(reduce_node_children_lt)> child_grouping(reduce_node_children_lt);
      child_grouping.set_available_memory(sorter_phase_1, sorter_phase_2, sorter_phase_1);
      child_grouping.begin();

      // Pull out all nodes from reduce_pq and sink_arcs for this level
      while ((sink_arcs.can_pull() && label_of(sink_arcs.peek().source) == label) || reduce_pq.can_pull()) {
        arc_t e_high = reduce_get_next(reduce_pq, sink_arcs);
        arc_t e_low = reduce_get_next(reduce_pq, sink_arcs);

        node_t n = node_of(e_low, e_high);

        // Apply Reduction rule 1
        if (n.low == n.high) {
          if (!red1_mapping.is_open()) {
            red1_mapping.open();
          }
          red1_mapping.write({ n.uid, n.low });
        } else {
          child_grouping.push(n);
        }
      }

      // Sort and apply Reduction rule 2
      child_grouping.end();
      child_grouping.calc(pi);

      tpie::merge_sorter<mapping, false, decltype(reduce_uid_lt)> red2_mapping(reduce_uid_lt);
      red2_mapping.set_available_memory(sorter_phase_1, sorter_phase_2, sorter_phase_1);
      red2_mapping.begin();

      if (child_grouping.can_pull()) {
        // Set up for remapping, keeping the very first node seen
        id_t out_id = MAX_ID;
        node_t current_node = child_grouping.pull();

        out_writer.unsafe_push(meta_t { label });

        node_t out_node = create_node(label, out_id, current_node.low, current_node.high);
        out_writer.unsafe_push(out_node);

        adiar_debug(out_id > 0, "Has run out of ids");
        out_id--;

        red2_mapping.push({ current_node.uid, out_node.uid });

        // Keep the first node with different children than prior, and remap all
        // the later that match its children.
        while (child_grouping.can_pull()) {
          node_t next_node = child_grouping.pull();
          if (current_node.low == next_node.low && current_node.high == next_node.high) {
            red2_mapping.push({ next_node.uid, out_node.uid });
          } else {
            current_node = next_node;

            out_node = create_node(label, out_id, current_node.low, current_node.high);
            out_writer.unsafe_push(out_node);
            out_id--;

            red2_mapping.push({current_node.uid, out_node.uid});
          }
        }
      }

      // Sort mappings for Reduction rule 2 back in order of node_arcs
      red2_mapping.end();
      red2_mapping.calc(pi);

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

      if (reduce_pq.has_next_level()) {
        if (sink_arcs.can_pull()) {
          reduce_pq.setup_next_level(label_of(sink_arcs.peek().source));
        } else {
          reduce_pq.setup_next_level();
        }
        label = reduce_pq.current_level();
      } else if (!out_writer.has_pushed()) {
        adiar_debug(!node_arcs.can_pull() && !sink_arcs.can_pull(),
                   "Nodes are still left to be processed");
        out_writer.unsafe_push({ next_red1.new_uid, NIL, NIL });

        return out_file;
      }
    }
    return out_file;
  }
} // namespace adiar

#endif // ADIAR_REDUCE_CPP
