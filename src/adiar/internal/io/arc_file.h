#ifndef ADIAR_INTERNAL_IO_ARC_FILE_H
#define ADIAR_INTERNAL_IO_ARC_FILE_H

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/io/levelized_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// An unreduced Decision Diagram is given by a three files of arcs:
  ///
  /// - [0] : node-to-node arcs (sorted by <tt>target</tt>)
  /// - [1] : node-to-terminal arcs (sorted by <tt>source</tt>).
  /// - [2] : node-to-terminal arcs (not sorted)
  //////////////////////////////////////////////////////////////////////////////
  template <>
  struct FILE_CONSTANTS<arc>
  {
    static constexpr size_t files = 3u;

    struct stats
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum one-level cut of the DAG (not
      ///        counting any arcs to terminals).
      ///
      /// TODO: use array of size 1?
      //////////////////////////////////////////////////////////////////////////
      cut_size_t max_1level_cut = MAX_CUT;
    };
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Files of arcs to represent an unreduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  typedef levelized_file<arc> arc_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  inline uint64_t nodecount(const arc_file &arcs)
  {
    return arcs->size() / node::OUTDEGREE;
  }
}

#endif // ADIAR_INTERNAL_IO_ARC_FILE_H
