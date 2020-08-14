#ifndef COOM_COUNT_PATHS_H
#define COOM_COUNT_PATHS_H

#include <tpie/file_stream.h>
#include "data.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// Some preset predicates
  //////////////////////////////////////////////////////////////////////////////

  /// \param sink
  /// \return Whether to count the path ending in sink
  typedef std::function<bool(uint64_t)> sink_pred;

  const sink_pred is_any = [] (uint64_t /* sink */) -> bool
  {
    return true;
  };

  const sink_pred is_true = [] (uint64_t sink) -> bool
  {
    return value_of(sink);
  };

  const sink_pred is_false = [] (uint64_t sink) -> bool
  {
    return !value_of(sink);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths that satisfy
  ///        given predicates.
  ///
  /// \param nodes The node-based OBDD graph in reverse topological order.
  /// \param sink_pred Predicate whether to count paths going to the given sink.
  ///
  /// \return The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t count_paths(tpie::file_stream<node> &nodes,
                       const sink_pred &sink_pred);

  uint64_t count_paths(tpie::file_stream<node> &nodes);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all assignments between the minimal and maximal label of a
  ///        given OBDD that satisfies a given predicate.
  ///
  /// \param nodes The node-based OBDD graph in reverse topological order.
  /// \param sink_pred Predicate whether to count paths going to the given sink.
  ///
  /// \return The number of unique assignments.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t count_assignment(tpie::file_stream<node> &nodes,
                           const sink_pred &sink_pred);
}

#endif // COOM_COUNT_PATHS_H
