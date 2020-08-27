#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include <tpie/priority_queue.h>

#include "data.h"
#include "debug.h"
#include "debug_data.h"

#include "assert.h"

#include "reduce.h"

namespace coom
{
  namespace debug {
    inline void println_reduce_layer([[maybe_unused]] uint64_t label)
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
      print_node(node_of_arcs(e_low, e_high));
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

    inline void println_reduce_forward([[maybe_unused]] uint64_t target,
                                       [[maybe_unused]] uint64_t old_node_ptr,
                                       [[maybe_unused]] uint64_t new_node_ptr)
    {
#if COOM_DEBUG >= 2
      tpie::log_info() << "|   | ";
      print_child(old_node_ptr);
      tpie::log_info() << " -> ";
      print_child(new_node_ptr);
      tpie::log_info() << " to ";
      print_child(target);
      tpie::log_info() << std::endl;
#endif
    }
  }

  inline auto bytes_from_size(tpie::file_stream<arc> &in_node_arcs,
                              tpie::file_stream<arc> &in_sink_arcs) {
    return sizeof(coom::node) * ((in_node_arcs.size() + in_sink_arcs.size()) / 2);
  }

  struct mapping
  {
    uint64_t old_node_ptr;
    uint64_t new_node_ptr;
  };

  //Predicate for redD
  struct reduce_queue_lt
  {
    bool operator()(const arc &a, const arc &b)
    {
      return (a.source > b.source || (a.source == b.source && a.is_high));
    }
  };

  //Predicate for L_j
  const auto reduce_node_children_lt = [](const node &a, const node &b) -> bool {
    return a.high > b.high ||
           (a.high == b.high && a.low > b.low) ||
           (a.high == b.high && a.low == b.low && a.node_ptr > b.node_ptr);
  };

  //Predicate for L_j_red2/out
  const auto reduce_node_ptr_lt = [](const mapping &a, const mapping &b) -> bool {
    return a.old_node_ptr > b.old_node_ptr;
  };

  void reduce(tpie::file_stream<arc> &in_node_arcs,
              tpie::file_stream<arc> &in_sink_arcs,
              tpie::file_stream<node> &out_nodes)
  {
    debug::println_algorithm_start("REDUCE");

    assert::is_valid_input_stream(in_node_arcs);
    debug::println_file_stream(in_node_arcs, "in_node_arcs");

    assert::is_valid_input_stream(in_sink_arcs);
    debug::println_file_stream(in_sink_arcs, "in_sink_arcs");

    assert::is_valid_output_stream(out_nodes);

    //Set up
    tpie::priority_queue<arc, reduce_queue_lt> redD;
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
      arc e_high = in_sink_arcs.read_back();
      arc e_low = in_sink_arcs.read_back();

      // Apply reduction rule 1 if applicable
      if (e_high.target == e_low.target) {
        out_nodes.write(create_sink_node(value_of(e_low.target)));
      } else {
        out_nodes.write(node_of_arcs(e_low, e_high));
      }

      debug::println_file_stream(out_nodes, "out_nodes");
      debug::println_algorithm_end("REDUCE");

      return;
    }

    // Find the first edge and its label
    arc next_node_arc = in_node_arcs.read_back();
    bool has_next_node_arc = true;

    uint64_t label = label_of(next_node_arc.target);

    arc next_sink_arc = in_sink_arcs.read_back();
    bool has_next_sink = true;

    // Process bottom-up each layer
    while (has_next_sink || !redD.empty()) {
      debug::println_reduce_layer(label);

      // Set up for L_j_red1
      tpie::file_stream<mapping> red1_mapping;
      red1_mapping.open();

      // Set-up for L_j
      tpie::merge_sorter<node, false, decltype(reduce_node_children_lt)> child_grouping(reduce_node_children_lt);
      child_grouping.set_available_memory(max_sorter_memory);
      child_grouping.begin();

      debug::println_reduce_red_1_start();

      // Pull out all nodes from redD and in_sink_arcs for this layer
      while ((has_next_sink && label_of(next_sink_arc.source) == label)
             || (!redD.empty() && label_of(redD.top().source) == label)) {
        arc e_low;
        arc e_high;

        if (redD.empty() || (has_next_sink && redD.top().source < next_sink_arc.source)) {
          // Pull both from in_sink_arcs
          e_high = next_sink_arc;
          e_low = in_sink_arcs.read_back();

          if (in_sink_arcs.can_read_back()) {
            next_sink_arc = in_sink_arcs.read_back();
          } else {
            has_next_sink = false;
          }
        } else if (redD.top().source == next_sink_arc.source) {
          // Pull one from redD and one from in_sink_arcs
          e_low = redD.top().is_high ? next_sink_arc : redD.top();
          e_high = redD.top().is_high ? redD.top() : next_sink_arc;
          redD.pop();

          if (in_sink_arcs.can_read_back()) {
            next_sink_arc = in_sink_arcs.read_back();
          } else {
            has_next_sink = false;
          }
        } else {
          // Pull both from redD
          e_high = redD.top();
          redD.pop();
          e_low = redD.top();
          redD.pop();
        }

        // Apply Reduction rule 1
        if (e_high.target == e_low.target) {
          debug::println_reduce_red_1(e_low, e_high, true);
          red1_mapping.write({e_low.source, e_low.target});
        } else {
          debug::println_reduce_red_1(e_low, e_high, false);
          child_grouping.push(node_of_arcs(e_low, e_high));
        }
      }

      // Output nodes and apply Reduction rule 2
      debug::println_reduce_red_2_start();

      child_grouping.end();
      child_grouping.calc(pi);

      tpie::merge_sorter<mapping, false, decltype(reduce_node_ptr_lt)> red2_mapping(reduce_node_ptr_lt);
      red2_mapping.set_available_memory(max_sorter_memory);
      red2_mapping.begin();

      if (child_grouping.can_pull()) {
        // Output the first
        uint64_t out_id = MAX_ID;
        node current_node = child_grouping.pull();

        node out_node = create_node(label, out_id, current_node.low, current_node.high);
        out_nodes.write(out_node);
        out_id--;

        debug::println_reduce_red_2(current_node, false);

        red2_mapping.push({ current_node.node_ptr, out_node.node_ptr });

        // Output nodes and remap the ones match the one just output
        while (child_grouping.can_pull()) {
          node next_node = child_grouping.pull();
          if (current_node.low == next_node.low && current_node.high == next_node.high) {
            debug::println_reduce_red_2(next_node, true);
            red2_mapping.push({ next_node.node_ptr, out_node.node_ptr });
          } else {
            debug::println_reduce_red_2(next_node, false);
            current_node = next_node;

            out_node = create_node(label, out_id, current_node.low, current_node.high);
            out_nodes.write(out_node);
            out_id--;

            red2_mapping.push({current_node.node_ptr, out_node.node_ptr});
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
        // Find the mapping with largest old_node_ptr
        bool is_red1_current = !has_next_red2 ||
                               (has_next_red1 && next_red1.old_node_ptr > next_red2.old_node_ptr);
        mapping current_map = is_red1_current ? next_red1 : next_red2;

        // Find all arcs that have sources that match the current mapping's old_node_ptr
        while (has_next_node_arc && current_map.old_node_ptr == next_node_arc.target) {
          arc new_arc = create_arc(next_node_arc.source,
                                   next_node_arc.is_high,
                                   current_map.new_node_ptr);
          debug::println_reduce_forward(next_node_arc.source,
                                        current_map.old_node_ptr,
                                        current_map.new_node_ptr);

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

      if (!redD.empty() || has_next_sink) {
        if (redD.empty() || (has_next_sink && next_sink_arc.source > redD.top().source)) {
          label = label_of(next_sink_arc.source);
        } else {
          label = label_of(redD.top().source);
        }
      }

      // Check if everything has been reduced down to one sink
      if (!in_node_arcs.can_read_back() &&
          !has_next_sink &&
          out_nodes.size() == 0) {
        out_nodes.write({ next_red1.new_node_ptr, NIL, NIL });

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
