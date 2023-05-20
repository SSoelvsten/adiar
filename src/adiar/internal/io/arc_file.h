#ifndef ADIAR_INTERNAL_IO_ARC_FILE_H
#define ADIAR_INTERNAL_IO_ARC_FILE_H

#include <adiar/statistics.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Struct holding statistics on arc files
  //////////////////////////////////////////////////////////////////////////////
  extern stats_t::arc_file_t stats_arc_file;

  // TODO (ADD):
  // TODO (QMDD):
  //  Generalize the arc type and the arc_file based on a given node.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief An unreduced Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////
  template <>
  struct file_traits<arc>
  {
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of files to store an arc-based representation.
    ///
    /// \details The arcs are split into the following three files.
    ///          0. node-to-node arcs (sorted by <tt>target</tt>)
    ///          1. node-to-terminal arcs (sorted by <tt>source</tt>).
    ///          2. node-to-terminal arcs (not sorted)
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t files = 3u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File index for internal arcs.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t IDX__INTERNAL = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File index for terminal arcs generated out-of-order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t IDX__TERMINALS__IN_ORDER = 1u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File index for terminal arcs generated out-of-order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t IDX__TERMINALS__OUT_OF_ORDER = 2u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view false_print = "0";

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view true_print = "1";

    struct stats
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief Whether the arcs represent a semi-transposed DAG, i.e. the
      ///        internal arcs are sorted by their 'target' rather than their
      ///        'source'.
      //////////////////////////////////////////////////////////////////////////
      // bool semi_transposed = true;

      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum one-level cut of the DAG (not
      ///        counting any arcs to terminals).
      //////////////////////////////////////////////////////////////////////////
      // TODO: use array of size 1?
      cut_size_t max_1level_cut = MAX_CUT;

      //////////////////////////////////////////////////////////////////////////
      /// \brief The number of false and true terminals in the file. Index 0
      ///        gives the number of false terminals and index 1 gives the
      ///        number of true terminals.
      //////////////////////////////////////////////////////////////////////////
      size_t number_of_terminals[2] = { 0, 0 };

      // TODO: if allowing to be a terminal (with NIL -> {0,1} arcs), then
      //       create base_stats class to inherit here and in node_file from.
    };
  };

  // TODO: Add 'untranspose' function to levelized_file<arc> (which is an alias
  //       for .sort<0>()).
  //
  //       To this end:
  //       - Rename 'levelized_file<elem_t>' to '__levelized_file<elem_t>'.
  //
  //       - Make 'levelized_file<elem_t>' be a class that publicly inherit the
  //         base class '__levelized_file<elem_t>'.
  //
  //       - Add a specialization for 'levelized_file<arc>' that also adds the
  //         'untranspose' function.
}

#endif // ADIAR_INTERNAL_IO_ARC_FILE_H
