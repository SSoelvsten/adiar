#ifndef ADIAR_INTERNAL_IO_NODE_FILE_H
#define ADIAR_INTERNAL_IO_NODE_FILE_H

#include <adiar/statistics.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Struct holding statistics on node files
  //////////////////////////////////////////////////////////////////////////////
  extern stats_t::node_file_t stats_node_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A reduced Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////
  template <>
  struct file_traits<node>
  {
    // TODO: reading direction?

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of files to store a node-based representation.
    ///
    /// \details The nodes are only in a single file at index 0 (sorted
    ///          descendingly by <tt>node.uid()</tt>)
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t files = 1u;

    //////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    //////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view false_print = "0";

    //////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    //////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view true_print = "1";

    struct stats
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief Boolean flag whether a set of nodes are well-formed with
      ///        respect to the stricter ordering required by the fast equality
      ///        check.
      //////////////////////////////////////////////////////////////////////////
      bool canonical = false;

      //////////////////////////////////////////////////////////////////////////
      /// \brief The diagram's width, i.e. the size of the largest level.
      //////////////////////////////////////////////////////////////////////////
      size_t width = 0u;

      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 1-level cut of the DAG (with or
      ///        without arcs to each respective terminal). Use
      ///        <tt>cut_type</tt> to index the desired variant of the type.
      ///
      /// \sa    cut_type
      //////////////////////////////////////////////////////////////////////////
      cuts_t max_1level_cut =
        { MAX_CUT, MAX_CUT, MAX_CUT, MAX_CUT };

      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 2-level cut of the DAG (with or
      ///        without arcs to each respective terminal). Use
      ///        <tt>cut_type</tt> to index the desired variant of the type.
      ///
      /// \sa    cut_type
      //////////////////////////////////////////////////////////////////////////
      cuts_t max_2level_cut =
        { MAX_CUT, MAX_CUT, MAX_CUT, MAX_CUT };

      //////////////////////////////////////////////////////////////////////////
      /// \brief The number of false and true terminals in the file. Index 0
      ///        gives the number of false terminals and index 1 gives the
      ///        number of true terminals.
      //////////////////////////////////////////////////////////////////////////
      size_t number_of_terminals[2] = { 0, 0 };

      //////////////////////////////////////////////////////////////////////////
      /// \brief      Check whether this file represents a terminal-only DAG.
      ///
      /// \param file The shared_levelized_file<node> to check its content.
      //////////////////////////////////////////////////////////////////////////
      inline bool is_terminal() const
      {
        // A shared_levelized_file<node> only contains a terminal iff the number
        // of arcs to a terminal value in its meta information is exactly one.
        return (this->number_of_terminals[false] + this->number_of_terminals[true]) == 1;
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief      Obtain the terminal value of this file.
      ///
      /// \param file The shared_levelized_file<node> to check its content
      ///
      /// \pre `is_terminal() == true`
      //////////////////////////////////////////////////////////////////////////
      inline bool value() const
      {
        adiar_assert(this->is_terminal());

        // Since the sum of the number of terminals is exactly one, then we can
        // use the value of the number of true terminals to indirectly derive
        // the value of the terminal.
        return this->number_of_terminals[true];
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Whether this DAG is the false terminal only.
      //////////////////////////////////////////////////////////////////////////
      inline bool is_false() const
      {
        return this->is_terminal() && !this->value();
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Whether this DAG is the true terminal only.
      //////////////////////////////////////////////////////////////////////////
      inline bool is_true() const
      {
        return this->is_terminal() && this->value();
      }
    };
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_FILE_H
