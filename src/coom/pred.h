#ifndef COOM_PRED_H
#define COOM_PRED_H

#include <stdint.h>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include "data.h"

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  /// Sink predicates
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<bool(uint64_t)> sink_pred;

  const sink_pred is_any = [] (ptr_t /* sink */) -> bool
    {
      return true;
    };

  const sink_pred is_true = [] (ptr_t sink) -> bool
    {
      return value_of(sink);
    };

  const sink_pred is_false = [] (ptr_t sink) -> bool
    {
      return !value_of(sink);
    };

  //////////////////////////////////////////////////////////////////////////////
  /// Check whether a given OBDD is sink-only and satisfies the given sink_pred.
  ///
  /// \param nodes     The given OBDD of nodes in reverse topological order
  /// \param sink_pred If the given OBDD is sink-only, then secondly the sink is
  ///                  checked with the given sink predicate. Default is any
  ///                  sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(tpie::file_stream<node_t>& nodes, const sink_pred& sink_pred);
}

#endif // COOM_PRED_H
