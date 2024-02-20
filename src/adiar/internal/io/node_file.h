#ifndef ADIAR_INTERNAL_IO_NODE_FILE_H
#define ADIAR_INTERNAL_IO_NODE_FILE_H

#include <adiar/statistics.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Struct holding statistics on node files
  //////////////////////////////////////////////////////////////////////////////////////////////////
  extern statistics::node_file_t stats_node_file;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A reduced Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <>
  struct file_traits<node>
  {
    // TODO: reading direction?

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Number of files to store a node-based representation.
    ///
    /// \details The nodes are only in a single file at index 0 (sorted descendingly by
    ///          `node.uid()`)
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t files = 1u;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view false_print = "0";

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view true_print = "1";

    struct stats
    {
      // TODO: Add boolean for 'reduced'?

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   Boolean flag whether the nodes are sorted as per the Reduce algorithm.
      ///
      /// \details Nodes on each level have to be sorted based on their children, first on their
      ///          `high` child and then breaking ties on their `low` child.
      //////////////////////////////////////////////////////////////////////////////////////////////
      bool sorted = false;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   Boolean flag whether the nodes are indexable.
      ///
      /// \details Being *indexable* means that the *identifier* of nodes start at each level from
      ///          `node::max_id` and then decrement by one. That is, there are no gaps in the
      ///          identifiers at each level.
      //////////////////////////////////////////////////////////////////////////////////////////////
      bool indexable = false;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   Whether the decision diagram is on *canonical* form.
      ///
      /// \details Since subtrees are not shared between BDDs when stored on disk, canonical is in
      ///          this case a more strict notion than usual for BDD packages:
      ///          1. The diagram is fully reduced.
      ///          2. All nodes are sorted based on their children (high first, then low).
      ///          3. For each level, the nodes' identifier consecutively decrement by one from the
      ///          maximal value.
      ///          If this is true, then equality checking can be done in a single cheap linear scan
      ///          rather than with an *O(N log N)* time-forwarding algorithm.
      //////////////////////////////////////////////////////////////////////////////////////////////
      inline bool
      is_canonical() const
      {
        return this->sorted && this->indexable;
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief The diagram's width, i.e. the size of the largest level.
      //////////////////////////////////////////////////////////////////////////////////////////////
      size_t width = 0u;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 1-level cut of the DAG (with or without arcs to each
      ///        respective terminal). Use `cut` to index the desired variant of the type.
      ///
      /// \see   cut
      //////////////////////////////////////////////////////////////////////////////////////////////
      cuts_t max_1level_cut = { cut::max, cut::max, cut::max, cut::max };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief An upper bound for the maximum 2-level cut of the DAG (with or without arcs to each
      ///        respective terminal). Use `cut` to index the desired variant of the type.
      ///
      /// \see   cut
      //////////////////////////////////////////////////////////////////////////////////////////////
      cuts_t max_2level_cut = { cut::max, cut::max, cut::max, cut::max };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief The number of false and true terminals in the file. Index 0 gives the number of
      ///        false terminals and index 1 gives the number of true terminals.
      //////////////////////////////////////////////////////////////////////////////////////////////
      size_t number_of_terminals[2] = { 0, 0 };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief      Check whether this file represents a terminal-only DAG.
      ///
      /// \param file The shared_levelized_file<node> to check its content.
      //////////////////////////////////////////////////////////////////////////////////////////////
      inline bool
      is_terminal() const
      {
        // A shared_levelized_file<node> only contains a terminal iff the number
        // of arcs to a terminal value in its meta information is exactly one.
        return (this->number_of_terminals[false] + this->number_of_terminals[true]) == 1;
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Obtain the terminal value of this file.
      ///
      /// \param file The shared_levelized_file<node> to check its content
      ///
      /// \pre   `is_terminal() == true`
      //////////////////////////////////////////////////////////////////////////////////////////////
      inline bool
      value() const
      {
        adiar_assert(this->is_terminal());

        // Since the sum of the number of terminals is exactly one, then we can
        // use the value of the number of true terminals to indirectly derive
        // the value of the terminal.
        return this->number_of_terminals[true];
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Whether this DAG is the false terminal only.
      //////////////////////////////////////////////////////////////////////////////////////////////
      inline bool
      is_false() const
      {
        return this->is_terminal() && !this->value();
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Whether this DAG is the true terminal only.
      //////////////////////////////////////////////////////////////////////////////////////////////
      inline bool
      is_true() const
      {
        return this->is_terminal() && this->value();
      }
    };
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_FILE_H
