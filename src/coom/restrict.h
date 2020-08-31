#ifndef COOM_RESTRICT_H
#define COOM_RESTRICT_H

#include <tpie/file_stream.h>

#include "data.h"
#include "assignment.h"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a given node-based OBDD based on a partial assignment.
  ///
  /// \param nodes_in   input stream of OBDD; sorted in reverse topological
  ///                   order.
  ///
  /// \param assignment Assignment sorted in order of label (and expected
  ///                   unique)
  ///
  /// \param nodes_out  (empty) output stream for result OBDD
  //////////////////////////////////////////////////////////////////////////////
  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<assignment_t> &in_assignment,
                tpie::file_stream<node_t> &out_nodes);
}

#endif // COOM_RESTRICT_H
