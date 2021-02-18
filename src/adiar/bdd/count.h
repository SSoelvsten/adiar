#ifndef ADIAR_COUNT_PATHS_H
#define ADIAR_COUNT_PATHS_H

#include <adiar/data.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of internal nodes used to represent the given BDD.
  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of levels, i.e. variables, occuring in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_varcount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to a T sink.
  ///
  /// \param bdd       The node-based BDD to count in.
  ///
  /// \return The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all assignments to variables that lead to a T sink. We don't
  /// need to know the exact variables in question, just the number of variables
  /// involved.
  ///
  /// \param bdd       The node-based BDD to count in.
  /// \param varcount  The total number of variables, that may occur in the bdd.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd, size_t varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Same as `bdd_satcount(bdd, varcount)`, with the varcount computed
  /// from the given min_label and max_label of the BDD.
  ///
  /// Counts all assignments for variables with the labels
  ///
  ///                 { min_label, min_label + 1, ..., max_label }
  ///
  /// Assumes that the given label interval is valid. That is, we assume that
  ///
  ///     min_label <= min_label(nodes) <= max_label(nodes) <= max_label
  ///
  /// \param bdd       The node-based BDD to count in.
  /// \param min_label The smallest label to count from.
  /// \param max_label The largest label to count to.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd, label_t min_label, label_t max_label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Same as `bdd_satcount(bdd, min_label, max_label)`, with
  /// min_label and max_label set based on the given BDD.
  ///
  /// \param bdd       The node-based BDD graph in reverse topological order.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &bdd);
}

#endif // ADIAR_COUNT_PATHS_H
