#ifndef COOM_ASSIGNMENT_H
#define COOM_ASSIGNMENT_H

#include <tpie/file_stream.h>

#include "data.h"
#include "pred.h"

namespace coom
{
  struct assignment {
    uint64_t label;
    bool value;
  };

  assignment create_assignment(uint64_t label, bool value);

  bool operator< (const assignment& a, const assignment& b);
  bool operator> (const assignment& a, const assignment& b);
  bool operator== (const assignment& a, const assignment& b);
  bool operator!= (const assignment& a, const assignment& b);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the (deepest) assignment that satisfies a given predicate
  ///
  /// \param nodes_in        input stream of OBDD; sorted in reverse topological
  ///                        order
  ///
  /// \param sink_pred       type of sink-node to be reached with the assignment
  ///
  /// \param out_assignment  (empty) output stream for a partial assignment
  ///                        reaching a sink satisfying sink_pred.
  ///
  /// \param comparator      The ordering for the outputted assignment. Default
  ///                        is in ascending order by the label.
  //////////////////////////////////////////////////////////////////////////////
  template<typename pred_t = std::less<assignment>>
  bool get_assignment(tpie::file_stream<node> &in_nodes,
                      const sink_pred& sink_pred,
                      tpie::file_stream<assignment> &out_assignment,
                      const pred_t pred = std::less<assignment>());
}

#endif // COOM_ASSIGNMENT_H
