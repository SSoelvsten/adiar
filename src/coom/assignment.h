#ifndef COOM_ASSIGNMENT_H
#define COOM_ASSIGNMENT_H

#include <tpie/file_stream.h>

#include "data.h"

namespace coom
{
  struct assignment {
    label_t label;
    bool value;
  };

  typedef assignment assignment_t;

  assignment_t create_assignment(label_t label, bool value);

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
  template<typename pred_t = std::less<assignment_t>>
  bool get_assignment(tpie::file_stream<node_t> &in_nodes,
                      const sink_pred& sink_pred,
                      tpie::file_stream<assignment_t> &out_assignment,
                      const pred_t pred = std::less<assignment_t>());
}

#endif // COOM_ASSIGNMENT_H
