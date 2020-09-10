#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include "data.h"
#include "priority_queue.cpp"

#include "debug.h"
#include "debug_data.h"

#include "assert.h"

#include "reduce.h"

namespace coom
{
  namespace debug {
    inline void println_reduce_layer([[maybe_unused]] label_t label)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << std::endl << "| layer: " << label << std::endl;
#endif
    }

    inline void println_reduce_red_1_start()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  reduction 1:" << std::endl;
#endif
    }

    inline void println_reduce_red_1([[maybe_unused]] arc e_low,
                                     [[maybe_unused]] arc e_high,
                                     [[maybe_unused]] bool is_reduced)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|   | ";
      print_node({ e_low.source, e_low.target, e_high.target });
      if (is_reduced) {
        tpie::log_info() << " [X]" << std::endl;
      } else {
        tpie::log_info() << " [ ]" << std::endl;
      }
#endif
    }

    inline void println_reduce_red_2_start()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  reduction 2:" << std::endl;
#endif
    }

    inline void println_reduce_red_2([[maybe_unused]] node node,
                                     [[maybe_unused]] bool is_reduced)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|   | ";
      print_node(node);
      if (is_reduced) {
        tpie::log_info() << " [X]" << std::endl;
      } else {
        tpie::log_info() << " [ ]" << std::endl;
      }
#endif
    }

    inline void println_reduce_forward_start()
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|  forward:" << std::endl;
#endif
    }

    inline void println_reduce_forward([[maybe_unused]] ptr_t target,
                                       [[maybe_unused]] uid_t old_uid,
                                       [[maybe_unused]] uid_t new_uid)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|   | ";
      print_child(old_uid);
      tpie::log_info() << " -> ";
      print_child(new_uid);
      tpie::log_info() << " to ";
      print_child(target);
      tpie::log_info() << std::endl;
#endif
    }
  }

  inline auto bytes_from_size(tpie::file_stream<arc_t> &in_node_arcs,
                              tpie::file_stream<arc_t> &in_sink_arcs) {
    return sizeof(coom::node) * ((in_node_arcs.size() + in_sink_arcs.size()) / 2);
  }

  struct mapping
  {
    uid_t old_uid;
    uid_t new_uid;
  };


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
      return coom::label_of(a.source);
    }
  };


  // For sorting for common children
  const auto reduce_node_children_lt = [](const node_t &a, const node_t &b) -> bool {
    return a.high > b.high ||
           (a.high == b.high && a.low > b.low) ||
           (a.high == b.high && a.low == b.low && a.uid > b.uid);
  };


  // For sorting back to original order
  const auto reduce_uid_lt = [](const mapping &a, const mapping &b) -> bool {
    return a.old_uid > b.old_uid;
  };


  // Merging priority queue with sink_arc stream
  inline arc_t reduce_get_next(priority_queue<arc_t, reduce_queue_label, reduce_queue_lt, 1, std::greater<label_t>> &redD,
                               tpie::file_stream<arc_t> &in_sink_arcs,
                               arc_t &next_sink_arc,
                               bool &has_next_sink)
  {
    if (!redD.can_pull() || (has_next_sink && next_sink_arc.source > redD.top().source)) {
      arc_t ret_value = next_sink_arc;

      if (in_sink_arcs.can_read_back()) {
        next_sink_arc = in_sink_arcs.read_back();
      } else {
        has_next_sink = false;
      }

      return ret_value;
    } else {
      return redD.pull();
    }
  }


  void reduce(tpie::file_stream<arc_t> &in_node_arcs,
              tpie::file_stream<arc_t> &in_sink_arcs,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    debug::println_algorithm_start("REDUCE");

    assert::is_valid_input_stream(in_node_arcs);
    debug::println_file_stream(in_node_arcs, "in_node_arcs");

    assert::is_valid_input_stream(in_sink_arcs);
    debug::println_file_stream(in_sink_arcs, "in_sink_arcs");

    assert::is_valid_output_stream(out_nodes);
    assert::is_valid_output_stream(out_meta);

    // Set up
    priority_queue<arc_t, reduce_queue_label, reduce_queue_lt, 1, std::greater<label_t>> redD(tpie::get_memory_manager().available() / 2);
    redD.hook_meta_stream(in_meta);

    in_sink_arcs.seek(0, tpie::file_stream_base::end);
    in_node_arcs.seek(0, tpie::file_stream_base::end);
    tpie::dummy_progress_indicator pi;

    auto max_sorter_memory = std::max(// Take at least enough space to make the merge_sorter not cry
                                      sizeof(node) * 124 * 1024 + 15 * 1024 * 1024,
                                      // Take at most a quarter of memory or half of the input size
                                      std::min(tpie::get_memory_manager().available() / 4,
                                               bytes_from_size(in_node_arcs, in_sink_arcs) / 2));

    // Check to see if in_node_arcs is empty
    if (!in_node_arcs.can_read_back()) {
      arc_t e_high = in_sink_arcs.read_back();
      arc_t e_low = in_sink_arcs.read_back();

      // Apply reduction rule 1 if applicable
      if (e_high.target == e_low.target) {
        out_nodes.write(create_sink(value_of(e_low.target)));
      } else {
        out_nodes.write({ e_low.source, e_low.target, e_high.target });
      }

      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("REDUCE");

      return;
    }

    // Find the first edge and its label
    arc_t next_node_arc = in_node_arcs.read_back();
    bool has_next_node_arc = true;

    arc_t next_sink_arc = in_sink_arcs.read_back();
    bool has_next_sink = true;

    label_t label = label_of(next_sink_arc.source);

    // Process bottom-up each layer
    while (has_next_sink || redD.can_pull()) {
#if COOM_ASSERT
      assert (label == redD.current_layer());
#endif

      debug::println_reduce_layer(label);

      // Set up for L_j_red1
      tpie::file_stream<mapping> red1_mapping;
      red1_mapping.open();

      // Set-up for L_j
      tpie::merge_sorter<node_t, false, decltype(reduce_node_children_lt)> child_grouping(reduce_node_children_lt);
      child_grouping.set_available_memory(max_sorter_memory);
      child_grouping.begin();

      debug::println_reduce_red_1_start();

      // Pull out all nodes from redD and in_sink_arcs for this layer
      while ((has_next_sink && label_of(next_sink_arc.source) == label) || redD.can_pull()) {
        arc_t e_high = reduce_get_next(redD, in_sink_arcs, next_sink_arc, has_next_sink);
        arc_t e_low = reduce_get_next(redD, in_sink_arcs, next_sink_arc, has_next_sink);

        node_t n = node_of(e_low, e_high);

        // Apply Reduction rule 1
        if (n.low == n.high) {
          debug::println_reduce_red_1(e_low, e_high, true);
          red1_mapping.write({ n.uid, n.low });
        } else {
          debug::println_reduce_red_1(e_low, e_high, false);
          child_grouping.push(n);
        }
      }

      // Output nodes and apply Reduction rule 2
      debug::println_reduce_red_2_start();

      child_grouping.end();
      child_grouping.calc(pi);

      tpie::merge_sorter<mapping, false, decltype(reduce_uid_lt)> red2_mapping(reduce_uid_lt);
      red2_mapping.set_available_memory(max_sorter_memory);
      red2_mapping.begin();

      if (child_grouping.can_pull()) {
        // Output the first
        id_t out_id = MAX_ID;
        node_t current_node = child_grouping.pull();
        out_meta.write(meta_t {label});

        node_t out_node = create_node(label, out_id, current_node.low, current_node.high);
        out_nodes.write(out_node);
        out_id--;

        debug::println_reduce_red_2(current_node, false);

        red2_mapping.push({ current_node.uid, out_node.uid });

        // Output nodes and remap the ones match the one just output
        while (child_grouping.can_pull()) {
          node_t next_node = child_grouping.pull();
          if (current_node.low == next_node.low && current_node.high == next_node.high) {
            debug::println_reduce_red_2(next_node, true);
            red2_mapping.push({ next_node.uid, out_node.uid });
          } else {
            debug::println_reduce_red_2(next_node, false);
            current_node = next_node;

            out_node = create_node(label, out_id, current_node.low, current_node.high);
            out_nodes.write(out_node);
            out_id--;

            red2_mapping.push({current_node.uid, out_node.uid});
          }
        }
      }

      // Sort mappings for Reduction rule 2 back in order of input
      red2_mapping.end();
      red2_mapping.calc(pi);

      // Merging of red1_mapping and red2_mapping
      red1_mapping.seek(0);
      mapping next_red1 = {0, 0};
      bool has_next_red1 = red1_mapping.can_read();
      if (has_next_red1) {
        next_red1 = red1_mapping.read();
      }

      mapping next_red2 = {0, 0};
      bool has_next_red2 = red2_mapping.can_pull();
      if (has_next_red2) {
        next_red2 = red2_mapping.pull();
      }

      debug::println_reduce_forward_start();

      // Pass all the mappings to Q
      while (has_next_red1 || has_next_red2) {
        // Find the mapping with largest old_uid
        bool is_red1_current = !has_next_red2 ||
                               (has_next_red1 && next_red1.old_uid > next_red2.old_uid);
        mapping current_map = is_red1_current ? next_red1 : next_red2;

#if COOM_ASSERT
        assert(!has_next_node_arc || current_map.old_uid == next_node_arc.target);
#endif

        // Find all arcs that have sources that match the current mapping's old_uid
        while (has_next_node_arc && current_map.old_uid == next_node_arc.target) {
          // The is_high flag is already included in next_node_arc.source
          arc_t new_arc = { next_node_arc.source, current_map.new_uid };

          debug::println_reduce_forward(next_node_arc.source,
                                        current_map.old_uid,
                                        current_map.new_uid);

          redD.push(new_arc);
          if (in_node_arcs.can_read_back()) {
            next_node_arc = in_node_arcs.read_back();
          } else {
            has_next_node_arc = false;
          }
        }

        // Update the mapping that was used
        if (is_red1_current) {
          if (red1_mapping.can_read()) {
            next_red1 = red1_mapping.read();
          } else {
            has_next_red1 = false;
          }
        } else {
          if (red2_mapping.can_pull()) {
            next_red2 = red2_mapping.pull();
          } else {
            has_next_red2 = false;
          }
        }
      }

      // Move on to the next layer
      red1_mapping.close();

      if (redD.has_next_layer()) {
        if (has_next_sink) {
          redD.setup_next_layer(label_of(next_sink_arc.source));
        } else {
          redD.setup_next_layer();
        }
        label = redD.current_layer();
      } else if (out_nodes.size() == 0) {
#if COOM_ASSERT
        assert (!has_next_node_arc && !has_next_sink);
#endif
        out_nodes.write({ next_red1.new_uid, NIL, NIL });

        debug::println_file_stream(out_nodes, "out_nodes");
        debug::println_algorithm_end("REDUCE");
        return;
      }
    }

    debug::println_file_stream(out_nodes, "out_nodes");
    debug::println_algorithm_end("REDUCE");
  }
} // namespace coom

#endif // COOM_REDUCE_CPP
