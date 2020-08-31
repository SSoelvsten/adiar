#ifndef COOM_APPLY_H
#define COOM_APPLY_H

#include <stdint.h>
#include "data.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// Some preset operators
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<ptr_t(ptr_t,ptr_t)> bool_op;

  const bool_op and_op = [] (ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return sink1 & sink2;
  };

  const bool_op or_op = [] (ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return sink1 | sink2;
  };

  const bool_op xor_op = [] (ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(value_of(sink1) != value_of(sink2));
  };

  const bool_op implies_op = [] (ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(value_of(sink1) ? value_of(sink2) : true);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a given node-based OBDD based on a partial assignment.
  ///
  /// \param out_nodes_1 Nodes in reverse topological order of the first OBDD.
  /// \param out_nodes_2 Nodes in reverse topological order of the second OBDD.
  /// \param op Binary boolean operator to be applied.
  ///
  /// \param out_nodes The output stream to send the nodes in reverse topological
  ///                  order.
  //////////////////////////////////////////////////////////////////////////////
  void apply(tpie::file_stream<node_t> &in_nodes_1,
             tpie::file_stream<node_t> &in_nodes_2,
             const bool_op &op,
             tpie::file_stream<node_t> &out_nodes);
}

#endif // COOM_APPLY_H
