#ifndef COOM_COUNT_PATHS_H
#define COOM_COUNT_PATHS_H

#include <coom/data.h>

#include <coom/bdd/bdd.h>

namespace coom
{
  uint64_t bdd_nodecount(const bdd &bdd);
  uint64_t bdd_varcount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths that satisfy
  /// given predicates.
  ///
  /// \param nodes The node-based OBDD graph in reverse topological order.
  /// \param sink_pred Predicate whether to count paths going to the given sink.
  ///
  /// \return The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const bdd &bdd, const sink_pred &sink_pred = is_true);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all assignments to variables in the given interval that
  /// satisfy a given predicate.
  ///
  /// Counts all assignments for variables with the labels
  ///
  ///                 { min_label, min_label + 1, ..., max_label }
  ///
  /// Assumes that the given label interval is valid. That is, we assume that
  ///
  ///     min_label <= min_label(nodes) <= max_label(nodes) <= max_label
  ///
  /// \param nodes     The node-based OBDD graph in reverse topological order.
  /// \param min_label The smallest label to count from
  /// \param max_label The largest label to count to
  /// \param sink_pred Predicate whether to count paths going to the given sink.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd,
                        label_t min_label,
                        label_t max_label,
                        const sink_pred &sink_pred = is_true);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Same as count_assignments, with min_label and max_label set based
  /// on the minimum and maximum label in the given node_file.
  ///
  /// \param nodes The node-based OBDD graph in reverse topological order.
  /// \param sink_pred Predicate whether to count paths going to the given sink.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd,
                        const sink_pred &sink_pred = is_true);

  // TODO: bdd_satcount with a size_t varcount

  // TODO: bdd_satcount with a tpie::file<label_t> or second node_file for
  // a variable cube
}

#endif // COOM_COUNT_PATHS_H
