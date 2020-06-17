#ifndef COOM_APPLY_H
#define COOM_APPLY_H

#include <stdint.h>
#include "data.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// Some preset operators
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<uint64_t(uint64_t,uint64_t)> bool_op;

  const bool_op and_op = [] (uint64_t sink1, uint64_t sink2) -> uint64_t
    {
      return sink1 ^ sink2;
    };

  const bool_op or_op = [] (uint64_t sink1, uint64_t sink2) -> uint64_t
    {
      return sink1 | sink2;
    };

  const bool_op xor_op = [] (uint64_t sink1, uint64_t sink2) -> uint64_t
    {
      return create_sink(value_of(sink1) != value_of(sink2));
    };

  const bool_op implies_op = [] (uint64_t sink1, uint64_t sink2) -> uint64_t
    {
      return create_sink(value_of(sink1) ? value_of(sink2) : true);
    };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a given node-based OBDD based on a partial assignment.
  ///
  /// \param nodes_1 Nodes in reverse topological order of the first OBDD.
  /// \param nodes_2 Nodes in reverse topological order of the first OBDD.
  /// \param op Binary boolean operator to be applied.
  ///
  /// \param reduce_node_arcs The (ptr,ptr) arcs sorted by target in order
  /// \param reduce_sink_arcs The (ptr,sink) arcs sorted by source in order
  //////////////////////////////////////////////////////////////////////////////
  void apply(tpie::file_stream<node>* nodes_1,
             tpie::file_stream<node>* nodes_2,
             bool_op op,
             tpie::file_stream<arc>* reduce_node_arcs,
             tpie::file_stream<arc>* reduce_sink_arcs);
}

#endif // COOM_APPLY_H
