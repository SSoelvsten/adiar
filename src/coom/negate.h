#ifndef COOM_NEGATE_H
#define COOM_NEGATE_H

#include <tpie/file_stream.h>
#include "data.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate a given node-based OBDD.
  ///
  /// \param in_nodes  Input nodes sorted by target in reverse-topological order.
  /// \param out_nodes Output nodes sorted by target similar to the input, but
  ///                  where all sinks are negated.
  //////////////////////////////////////////////////////////////////////////////
  void negate(tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<node_t> &out_nodes);
}

#endif // COOM_NEGATE_H
