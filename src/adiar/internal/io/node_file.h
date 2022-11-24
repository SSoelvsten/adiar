#ifndef ADIAR_INTERNAL_IO_NODE_FILE_H
#define ADIAR_INTERNAL_IO_NODE_FILE_H

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// A reduced Decision Diagram is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  template <>
  struct FILE_CONSTANTS<node>
  {
    static constexpr size_t files = 1u;

    struct stats
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief Boolean flag whether a set of nodes are well-formed with
      ///        respect to the stricter ordering required by the fast equality
      ///        check.
      //////////////////////////////////////////////////////////////////////////
      bool canonical = false;

      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 1-level cut of the DAG (with or
      ///        without arcs to each respective terminal). Use <tt>cut_type</tt> to
      ///        index the desired variant of the type.
      ///
      /// \sa    cut_type
      //////////////////////////////////////////////////////////////////////////
      cuts_t max_1level_cut =
        { MAX_CUT, MAX_CUT, MAX_CUT, MAX_CUT };

      //////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 2-level cut of the DAG (with or
      ///        without arcs to each respective terminal). Use <tt>cut_type</tt> to
      ///        index the desired variant of the type.
      ///
      /// \sa    cut_type
      //////////////////////////////////////////////////////////////////////////
      cuts_t max_2level_cut =
        { MAX_CUT, MAX_CUT, MAX_CUT, MAX_CUT };
    };
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief File of nodes to represent a reduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  class node_file : public levelized_file<node>
  {
  public:
    static constexpr std::string_view false_print = "0";
    static constexpr std::string_view true_print = "1";

  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors

    // Temporary node_file constructor
    node_file() = default;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the meta file
    //////////////////////////////////////////////////////////////////////////////
    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Check whether a given node_file represents a terminal-only DAG.
  ///
  /// \param file The node_file to check its content
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(const node_file &file)
  {
    // A node_file only contains a terminal iff the number of arcs to a terminal value
    // in its meta information is exactly one.
    return (file->number_of_terminals[false] + file->number_of_terminals[true]) == 1;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Obtain the terminal value of a node_file where 'is_terminal' is true.
  ///
  /// \param file The node_file to check its content
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const node_file &file)
  {
    adiar_debug(is_terminal(file), "Must be a terminal to extract its value");

    // Since the sum of the number of terminals is exactly one, then we can use the
    // value of the number of true terminals to indirectly derive the value of the
    // terminal.
    return file._file_ptr -> number_of_terminals[true];
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The minimal label, i.e. the label of the root.
  //////////////////////////////////////////////////////////////////////////////
  node::label_t min_label(const node_file &file);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal label, i.e. the label of the deepest node.
  //////////////////////////////////////////////////////////////////////////////
  node::label_t max_label(const node_file &file);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  inline uint64_t nodecount(const node_file &nodes)
  {
    return is_terminal(nodes) ? 0u : nodes.size();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of levels (i.e. number of unique labels) in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  inline uint64_t varcount(const node_file &nodes)
  {
    return nodes.meta_size();
  }
}

#endif // ADIAR_INTERNAL_IO_NODE_FILE_H
