#ifndef ADIAR_INTERNAL_IO_NODE_ARC_STREAM_H
#define ADIAR_INTERNAL_IO_NODE_ARC_STREAM_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/arc_stream.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream nodes from an arc file (skipping any of Reduce algorithm).
  ///
  /// \param Reverse Whether the reading direction should be reversed
  ///                (relatively to the ordering of nodes within the file).
  ///
  /// \see shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////
  template <bool Reverse = false>
  class node_arc_stream : protected arc_stream<!Reverse>
  {
  private:
    /// \brief Arc-based input to-be converted into nodes on-the-fly.
    arc_stream<!Reverse> _stream;

    /// \brief Whether a converted node has been buffered.
    bool _has_peeked = false;

    /// \brief Latest converted node.
    node _peeked;

  public:
    static size_t
    memory_usage()
    {
      return arc_stream<!Reverse>::memory_usage();
    }

  private:
    static constexpr size_t idx__internal = file_traits<arc>::idx__internal;

    static constexpr size_t idx__terminals__in_order = file_traits<arc>::idx__terminals__in_order;

    static constexpr size_t idx__terminals__out_of_order =
      file_traits<arc>::idx__terminals__out_of_order;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_stream() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_stream(const node_arc_stream<Reverse>&) = delete;
    node_arc_stream(node_arc_stream<Reverse>&&)      = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an arc file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_stream(levelized_file<arc>& file, const bool negate = false)
      : _stream(/*need to sort before attach*/)
    {
      attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a shared arc file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_stream(shared_ptr<levelized_file<arc>>& file, const bool negate = false)
      : _stream(/*need to sort before attach*/)
    {
      attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an Decision Diagram.
    ///
    /// \pre The given diagram should contain an unreduced diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_stream(const __dd& diagram)
      : _stream(/*need to sort before attach*/)
    {
      adiar_assert(diagram.template has<__dd::shared_arc_file_type>());
      attach(diagram.template get<__dd::shared_arc_file_type>(), diagram.negate);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Attach to an arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    attach(levelized_file<arc>& file, const bool negate = false)
    {
      if (file.semi_transposed) {
        file.sort<arc_source_lt>(idx__internal);
        file.semi_transposed = false;
      }
      _stream.attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Attach to a shared arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    attach(const shared_ptr<levelized_file<arc>>& file, const bool negate = false)
    {
      if (file->semi_transposed) {
        file->sort<arc_source_lt>(idx__internal);
        file->semi_transposed = false;
      }
      _stream.attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull() const
    {
      return _has_peeked || _stream.can_pull_internal() || _stream.can_pull_terminal();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    pull()
    {
      adiar_assert(can_pull());
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      return merge_next();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    peek()
    {
      adiar_assert(can_pull());
      if (!_has_peeked) {
        _has_peeked = true;
        _peeked = merge_next();
      }
      return _peeked;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Obtain the first node past the seeked value.
    ///
    /// \param u Unique Identifier to seek for.
    ///
    /// \pre     `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    seek(const node::uid_type& u);
    // TODO

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    merge_next()
    {
      const arc a_first  = take_internal() ? _stream.pull_internal() : _stream.pull_terminal();
      const arc a_second = take_internal() ? _stream.pull_internal() : _stream.pull_terminal();

      // Merge into a node (providing low arc first)
      if constexpr (Reverse) {
        return node_of(a_second, a_first);
      } else {
        return node_of(a_first, a_second);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    take_internal()
    {
      if constexpr (Reverse) {
        if (!_stream.can_pull_terminal()) { return true; }
        if (!_stream.can_pull_internal()) { return false; }
      } else {
        adiar_assert(_stream.can_pull_terminal(),
                     "When reading top-down there must be terminal arcs left");

        if (!_stream.can_pull_internal()) { return false; }
      }

      const arc::pointer_type internal_source = _stream.peek_internal().source();
      const arc::pointer_type terminal_source = _stream.peek_terminal().source();

      if constexpr (Reverse) {
        return internal_source > terminal_source;
      } else {
        return internal_source < terminal_source;
      }
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_ARC_STREAM
