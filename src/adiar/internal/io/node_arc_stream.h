#ifndef ADIAR_INTERNAL_IO_NODE_ARC_STREAM_H
#define ADIAR_INTERNAL_IO_NODE_ARC_STREAM_H

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/arc_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream nodes from an arc file (skipping any of Reduce algorithm).
  ///
  /// \param reverse Whether the reading direction should be reversed
  ///                (relatively to the ordering of nodes within the file).
  ///
  /// \sa shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////
  template<bool reverse = false>
  class node_arc_stream : protected arc_stream<!reverse>
  {
  private:
    using parent_t = arc_stream<!reverse>;

  public:
    static size_t memory_usage()
    {
      return parent_t::memory_usage();
    }

  private:
    static constexpr size_t IDX__INTERNAL =
      file_traits<arc>::IDX__INTERNAL;

    static constexpr size_t IDX__TERMINALS__IN_ORDER =
      file_traits<arc>::IDX__TERMINALS__IN_ORDER;

    static constexpr size_t IDX__TERMINALS__OUT_OF_ORDER =
      file_traits<arc>::IDX__TERMINALS__OUT_OF_ORDER;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    node_arc_stream() = default;

    ////////////////////////////////////////////////////////////////////////////
    node_arc_stream(const node_stream<reverse> &) = delete;
    node_arc_stream(node_stream<reverse> &&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an arc file.
    ////////////////////////////////////////////////////////////////////////////
    node_arc_stream(levelized_file<arc> &file, const bool negate = false)
      : parent_t(/*need to sort before attach*/)
    {
      attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a shared arc file.
    ////////////////////////////////////////////////////////////////////////////
    node_arc_stream(shared_ptr<levelized_file<arc>> &file,
                    const bool negate = false)
      : parent_t(/*need to sort before attach*/)
    {
      attach(file, negate);
    }


    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an Decision Diagram.
    ///
    /// \pre The given diagram should contain an unreduced diagram.
    ////////////////////////////////////////////////////////////////////////////
    node_arc_stream(const __dd &diagram)
      : parent_t(/*need to sort before attach*/)
    {
      adiar_precondition(diagram.template has<__dd::shared_arcs_t>());
      attach(diagram.template get<__dd::shared_arcs_t>(), diagram.negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to an arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(levelized_file<arc> &file, const bool negate = false)
    {
      if (file.semi_transposed) {
        file.sort<arc_source_lt>(IDX__INTERNAL);
        file.semi_transposed = false;
      }
      parent_t::attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const shared_ptr<levelized_file<arc>> &file,
                const bool negate = false)
    {
      if (file->semi_transposed) {
        file->sort<arc_source_lt>(IDX__INTERNAL);
        file->semi_transposed = false;
      }
      parent_t::attach(file, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull() const
    {
      return parent_t::can_pull_internal() || parent_t::can_pull_terminal();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    const node pull()
    {
      const arc a_first =
        take_internal() ? parent_t::pull_internal() : parent_t::pull_terminal();
      const arc a_second =
        take_internal() ? parent_t::pull_internal() : parent_t::pull_terminal();

      // Merge into a node (providing low arc first)
      if constexpr (reverse) {
        return node_of(a_second, a_first);
      } else {
        return node_of(a_first, a_second);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    const node peek()
    {
      const arc a_first =
        take_internal() ? parent_t::peek_internal() : parent_t::peek_terminal();
      const arc a_second =
        take_internal() ? parent_t::peek_internal() : parent_t::peek_terminal();

      // Merge into a node (providing low arc first)
      if constexpr (reverse) {
        return node_of(a_second, a_first);
      } else {
        return node_of(a_first, a_second);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Obtain the first node past the seeked value.
    ///
    /// \param u Unique Identifier to seek for.
    ///
    /// \pre     `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    const node seek(const node::uid_t &u);
    // TODO

  private:
    bool take_internal()
    {
      if constexpr (reverse) {
        if (!parent_t::can_pull_terminal()) { return true;  }
        if (!parent_t::can_pull_internal()) { return false; }
      } else {
        adiar_debug(parent_t::can_pull_terminal(),
                    "When reading top-down there must be terminal arcs left");

        if (!parent_t::can_pull_internal()) { return false; }
      }

      const arc::ptr_t internal_source = parent_t::peek_internal().source();
      const arc::ptr_t terminal_source = parent_t::peek_terminal().source();

      if constexpr (reverse) {
        return internal_source > terminal_source;
      } else {
        return internal_source < terminal_source;
      }
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_ARC_STREAM
