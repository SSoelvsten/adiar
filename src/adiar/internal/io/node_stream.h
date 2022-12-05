#ifndef ADIAR_INTERNAL_IO_NODE_STREAM_H
#define ADIAR_INTERNAL_IO_NODE_STREAM_H

#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  // TODO: node_stream for untransposed node files?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief         Stream of nodes from a node file.
  ///
  /// \param REVERSE Whether the reading direction should be reversed
  ///                (relatively to the ordering of nodes within the file).
  ///
  /// \sa shared_levelized_file<node>
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  class node_stream : public levelized_file_stream<node, !REVERSE>
  {
    using parent_t = levelized_file_stream<node, !REVERSE>;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    node_stream() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a node file.
    ////////////////////////////////////////////////////////////////////////////
    node_stream(const shared_levelized_file<node> &file, bool negate = false)
      : parent_t(file, negate)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a Decision Diagram.
    ////////////////////////////////////////////////////////////////////////////
    node_stream(const dd &diagram)
      : parent_t(diagram.file, diagram.negate)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    { return parent_t::template can_pull<0>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    node pull()
    { return parent_t::template pull<0>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    node peek()
    { return parent_t::template peek<0>(); }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_STREAM_H
