#ifndef COOM_ASSIGNMENT_H
#define COOM_ASSIGNMENT_H

#include <optional>

#include <coom/data.h>
#include <coom/file.h>

#include <coom/reduce.h>

#include <coom/util.h>

namespace coom
{
  inline bool has_satisfying_sink(const node_t &n, const sink_pred &sink_pred)
  {
    return (is_sink_ptr(n.low) && sink_pred(n.low))
      || (is_sink_ptr(n.high) && sink_pred(n.high));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the (deepest) assignment that satisfies a given predicate
  ///
  /// \param nodes           input file of OBDD nodes
  ///
  /// \param sink_pred       type of sink-node to be reached with the assignment
  /// \param sorting_pred_t  the desired ordering of the returned assignment
  //////////////////////////////////////////////////////////////////////////////
  template<typename sorting_pred_t = std::less<assignment_t>>
  std::optional<assignment_file> bdd_get_assignment(const node_file &nodes,
                                                    const sink_pred &sink_pred = is_true,
                                                    const sorting_pred_t &sorting_pred = sorting_pred_t())
  {
    file_stream<node_t, false> in_nodes(nodes._files[0]);

    if (is_sink(in_nodes.peek())) {
      return std::make_optional<assignment_file>();
    }

    // Find the first node, that satisfies the predicate (if any)
    node_t prior_node;
    do {
      prior_node = in_nodes.pull();
    } while (in_nodes.can_pull() && !has_satisfying_sink(prior_node, sink_pred));

    if (!has_satisfying_sink(prior_node, sink_pred)) {
      return std::nullopt;
    }

    tpie::merge_sorter<assignment_t, false, sorting_pred_t> assignment_sorter(sorting_pred);
    assignment_sorter.set_available_memory(tpie::get_memory_manager().available());
    assignment_sorter.begin();

    label_t label = label_of(prior_node);
    bool value = is_sink_ptr(prior_node.high) && sink_pred(prior_node.high);

    assignment_sorter.push(create_assignment(label, value));

    // Output first-seen nodes that can lead to this node
    while (in_nodes.can_pull()) {
      node_t parent_node;
      do {
        parent_node = in_nodes.pull();
      } while (parent_node.low != prior_node.uid && parent_node.high != prior_node.uid);

      label_t label = label_of(parent_node);
      bool value = parent_node.high == prior_node.uid;

      assignment_sorter.push(create_assignment(label, value));
      prior_node = parent_node;
    }

    assignment_file out;
    sort_into_file(assignment_sorter, out._file);

    return std::make_optional<assignment_file>(out);
  }
}

#endif // COOM_ASSIGNMENT_H
