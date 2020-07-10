#ifndef COOM_REDUCE_CPP
#define COOM_REDUCE_CPP

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include <tpie/priority_queue.h>

#include "data.h"
#include "debug.h"
#include "debug_data.h"

#include "reduce.h"

namespace coom
{
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
           (a.high == b.high && a.low == b.low && a > b);
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
    debug::println_file_stream(in_node_arcs, "in_node_arcs");
    debug::println_file_stream(in_sink_arcs, "in_sink_arcs");

    //Set-up
    uint64_t id0 = MAX_ID;
    tpie::priority_queue<arc, reduce_queue_lt> redD;
    in_sink_arcs.seek(0, tpie::file_stream_base::end);
    in_node_arcs.seek(0, tpie::file_stream_base::end);
    tpie::progress_indicator_null pi;

    //Check to see if in_node_arcs and in_sink_arcs are empty
    //Edge case where we only return a sink
    if (!in_node_arcs.can_read_back())
    {
      arc e_high = in_sink_arcs.read_back();
      arc e_low = in_sink_arcs.read_back();
      if (e_high.target == e_low.target)
      {
        out_nodes.write(create_sink_node(value_of(e_low.target)));
      }
      else
      {
        out_nodes.write(node_of_arcs(e_low, e_high));
      }
      return;
    }

    //Find the first edge and its label
    arc current_arc = in_node_arcs.read_back();
    uint64_t label = label_of(current_arc.target);
    arc current_sink_arc = in_sink_arcs.read_back();
    bool use_current_sink = true;

    //Process bottom-up each layer
    while (in_sink_arcs.can_read_back() || !redD.empty())
    {
      //Reset ID for this layer
      id0 = MAX_ID;

      //Set-up for L_j_red1
      tpie::file_stream<mapping> reduction_rule_1_mapping;
      reduction_rule_1_mapping.open();

      //Set-up for L_j
      tpie::merge_sorter<node, true, decltype(reduce_node_children_lt)> sorter1(reduce_node_children_lt);
      sorter1.set_available_memory(tpie::get_memory_manager().available() / 2);
      sorter1.begin();

      //Pull out all nodes from redD and in_sink_arcs for this layer and check reduction 1
      arc e_high;
      arc e_low;
      while ((in_sink_arcs.can_read_back() && label_of(current_sink_arc.source) == label) || (!redD.empty() && label_of(redD.top().source) == label))
      {
        //Pull both from in_sink_arcs
        if (redD.empty() || (in_sink_arcs.can_read_back() && redD.top().source < current_sink_arc.source))
        {
          e_high = current_sink_arc;
          e_low = in_sink_arcs.read_back();
          if (in_sink_arcs.can_read_back())
          {
            current_sink_arc = in_sink_arcs.read_back();
          }
          else
          {
            use_current_sink = false;
          }
        }

        //Pull at least one from redD
        else
        {
          arc top = redD.top();
          //Pull one from redD and one from in_sink_arcs
          if (top.source == current_sink_arc.source)
          {
            //high from redD, low from in_sink_arcs
            if (top.is_high)
            {
              e_high = top;
              redD.pop();
              e_low = current_sink_arc;
            }
            //high from in_sink_arcs, low from redD
            else
            {
              e_high = current_sink_arc;
              e_low = top;
              redD.pop();
            }
            //Update current_sink_arc if possible
            if (in_sink_arcs.can_read_back())
            {
              current_sink_arc = in_sink_arcs.read_back();
            }
            else
            {
              use_current_sink = false;
            }
          }
          //Pull two from redD
          else
          {
            e_high = top;
            redD.pop();
            e_low = redD.top();
            redD.pop();
          }
        }

        //Check if the pulled arcs can be reduced with reduction 1
        if (e_high.source == e_low.source && e_high.target == e_low.target)
        {
          //Write a mapping to L_j_red1
          reduction_rule_1_mapping.write({e_low.source, e_low.target});
        }
        else
        {
          //Write a node to L_j
          sorter1.push(node_of_arcs(e_low, e_high));
        }
      }

      // Sort L_j
      sorter1.end();
      sorter1.calc(pi);

      //Pull the first element from L_j if possible
      node current_node;
      bool could_pull = false;
      if (sorter1.can_pull())
      {
        current_node = sorter1.pull();
        could_pull = true;
      }

      //Set up L_j_red2/out
      tpie::merge_sorter<mapping, true, decltype(reduce_node_ptr_lt)> sorter2(reduce_node_ptr_lt);
      sorter2.set_available_memory(tpie::get_memory_manager().available() / 2);
      sorter2.begin();

      //Pull the rest from L_j and check for reduction 2
      while (sorter1.can_pull())
      {
        //Find the node to compare with the current
        node next_node = sorter1.pull();
        if (current_node.low == next_node.low && current_node.high == next_node.high)
        {
          //The nodes are equal, "throw away" the second (pass it to L_j_red2/out)
          sorter2.push({next_node.node_ptr, create_node_ptr(label, id0)});
        }
        else
        {
          //The nodes are not equal, pass the first to L_j_red2/out and out_nodes and continue with the second
          out_nodes.write(create_node(label, id0, current_node.low, current_node.high));
          sorter2.push({current_node.node_ptr, create_node_ptr(label, id0)});
          id0 = id0 - 1;
          current_node = next_node;
        }
      }

      //Write the last node to L_j_red2/out and out_nodes
      if (could_pull)
      {
        out_nodes.write(create_node(label, id0, current_node.low, current_node.high));
        sorter2.push({current_node.node_ptr, create_node_ptr(label, id0)});
      }

      // Sort L_j_red2/out
      sorter2.end();
      sorter2.calc(pi);

      //Find the first mappings from L_j_red1 and L_j_red2/out
      reduction_rule_1_mapping.seek(0);
      mapping red1 = {0, 0};
      bool new_red1 = false;
      if (reduction_rule_1_mapping.can_read())
      {
        red1 = reduction_rule_1_mapping.read();
        new_red1 = true;
      }

      mapping red2 = {0, 0};
      bool new_red2 = false;
      if (sorter2.can_pull())
      {
        red2 = sorter2.pull();
        new_red2 = true;
      }

      mapping current_map;
      bool red1_current;

      //Pass all the mappings to Q
      while (new_red1 || new_red2)
      {
        //Find the mapping with largest old_node_ptr
        if ((new_red1 && red1.old_node_ptr > red2.old_node_ptr) || !new_red2)
        {
          current_map = red1;
          red1_current = true;
        }
        else
        {
          current_map = red2;
          red1_current = false;
        }

        //Find all arcs that have sources that match the current mapping's old_node_ptr
        while (current_map.old_node_ptr == current_arc.target)
        {
          arc new_arc = create_arc(current_arc.source, current_arc.is_high, current_map.new_node_ptr);
          redD.push(new_arc);
          if (in_node_arcs.can_read_back())
          {
            current_arc = in_node_arcs.read_back();
          }
          else
          {
            break;
          }
        }

        //Update the mapping that was used
        if (red1_current)
        {
          if (reduction_rule_1_mapping.can_read())
          {
            red1 = reduction_rule_1_mapping.read();
          }
          else
          {
            new_red1 = false;
          }
        }
        else
        {
          if (sorter2.can_pull())
          {
            red2 = sorter2.pull();
          }
          else
          {
            new_red2 = false;
          }
        }
      }

      //Move on to the next layer (check if everything has been reduced down to one sink)
      reduction_rule_1_mapping.close();

      if (!redD.empty() || use_current_sink)
      {
        if (redD.empty() || (use_current_sink && label_of(current_sink_arc.source) > label_of(redD.top().source)))
        {
          label = label_of(current_sink_arc.source);
        }
        else
        {
          label = label_of(redD.top().source);
        }
      }

      if (!in_node_arcs.can_read_back() && !in_sink_arcs.can_read_back() && out_nodes.size() == 0)
      {
        out_nodes.write(create_sink_node(value_of(current_map.new_node_ptr)));

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
