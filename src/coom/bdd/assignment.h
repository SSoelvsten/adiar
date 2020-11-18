#ifndef COOM_ASSIGNMENT_H
#define COOM_ASSIGNMENT_H

#include <optional>

#include <coom/data.h>
#include <coom/file.h>
#include <coom/file_stream.h>
#include <coom/file_writer.h>
#include <coom/bdd/bdd.h>

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
  std::optional<assignment_file> bdd_get_assignment(const bdd &bdd,
                                                    const sink_pred &sink_pred = is_true,
                                                    const sorting_pred_t &pred = sorting_pred_t())
  {
    // Read the nodes bottom-up (i.e. in reverse)
    node_stream<true> in_nodes(bdd);

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

    assignment_file out;
    simple_file_writer<assignment_t, no_ordering<assignment_t>> ow(out);

    label_t label = label_of(prior_node);
    bool value = is_sink_ptr(prior_node.high) && sink_pred(prior_node.high);

    ow.unsafe_push(create_assignment(label, value));

    // Output first-seen nodes that can lead to this node
    while (in_nodes.can_pull()) {
      node_t parent_node;
      do {
        parent_node = in_nodes.pull();
      } while (parent_node.low != prior_node.uid && parent_node.high != prior_node.uid);

      label_t label = label_of(parent_node);
      bool value = parent_node.high == prior_node.uid;

      ow.unsafe_push(create_assignment(label, value));
      prior_node = parent_node;
    }

    ow.sort(pred);

    return std::make_optional<assignment_file>(out);
  }
}

#endif // COOM_ASSIGNMENT_H
