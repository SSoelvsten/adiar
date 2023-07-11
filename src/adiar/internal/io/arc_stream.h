#ifndef ADIAR_INTERNAL_IO_ARC_STREAM_H
#define ADIAR_INTERNAL_IO_ARC_STREAM_H

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Levelized file stream with additional logic for arc files.
  ///
  /// \sa shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////
  template<bool reverse = false>
  class arc_stream : public levelized_file_stream<arc, !reverse>
  {
  private:
    using parent_t = levelized_file_stream<arc, !reverse>;

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

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Keep track of the number of unread terminals
    ////////////////////////////////////////////////////////////////////////////
    size_t _unread_terminals[2] = { 0u, 0u };

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    arc_stream() = default;

    ////////////////////////////////////////////////////////////////////////////
    arc_stream(const arc_stream<reverse> &) = delete;
    arc_stream(arc_stream<reverse> &&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream attached to an arc file.
    ////////////////////////////////////////////////////////////////////////////
    arc_stream(const levelized_file<arc> &file, bool negate = false)
    { attach(file, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream unattached to a shared arc file.
    ////////////////////////////////////////////////////////////////////////////
    arc_stream(const shared_ptr<levelized_file<arc>> &file, bool negate = false)
    { attach(file, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~arc_stream() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a levelized arc file.
    ///
    /// \pre No `levelized_file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const levelized_file<arc> &f,
                const bool negate = false)
    {
      parent_t::attach(f, negate);
      _unread_terminals[negate ^ false] = f.number_of_terminals[false];
      _unread_terminals[negate ^ true]  = f.number_of_terminals[true];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared levelized arc file.
    ///
    /// \pre No `levelized_file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const shared_ptr<levelized_file<arc>> &f,
                const bool negate = false)
    {
      parent_t::attach(f, negate);
      _unread_terminals[negate ^ false] = f->number_of_terminals[false];
      _unread_terminals[negate ^ true]  = f->number_of_terminals[true];
    }


  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more internal arcs.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull_internal() const
    { return parent_t::template can_pull<IDX__INTERNAL>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next internal arc (and move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc pull_internal()
    { return parent_t::template pull<IDX__INTERNAL>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next internal arc (but do not move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc peek_internal()
    { return parent_t::template peek<IDX__INTERNAL>(); }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the next element ought to be pulled from the \em in-order
    ///        index.
    ////////////////////////////////////////////////////////////////////////////
    bool take_in_order_terminal()
    {
      const bool in_order_pullable =
        parent_t::template can_pull<IDX__TERMINALS__IN_ORDER>();

      const bool out_of_order_pullable =
        parent_t::template can_pull<IDX__TERMINALS__OUT_OF_ORDER>();

      if (in_order_pullable != out_of_order_pullable) {
        return in_order_pullable;
      }

      const arc::ptr_t in_order_source =
        parent_t::template peek<IDX__TERMINALS__IN_ORDER>().source();

      const arc::ptr_t out_of_order_source =
        parent_t::template peek<IDX__TERMINALS__OUT_OF_ORDER>().source();

      if constexpr (reverse) {
        return in_order_source < out_of_order_source;
      } else {
        return in_order_source > out_of_order_source;
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the number of unread terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t unread_terminals() const
    { return _unread_terminals[false] + _unread_terminals[true]; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the number of unread terminals of a specific value.
    ////////////////////////////////////////////////////////////////////////////
    const size_t& unread_terminals(const bool terminal_value) const
    { return _unread_terminals[terminal_value]; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more terminal arcs.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull_terminal() const
    {
      return unread_terminals() > 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (and move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc pull_terminal()
    {
      const arc a = take_in_order_terminal()
        ? parent_t::template pull<IDX__TERMINALS__IN_ORDER>()
        : parent_t::template pull<IDX__TERMINALS__OUT_OF_ORDER>();

      adiar_debug(_unread_terminals[a.target().value()] > 0,
                  "Terminal counter should not be zero");
      _unread_terminals[a.target().value()]--;

      return a;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (but do not move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc peek_terminal()
    {
      return take_in_order_terminal()
        ? parent_t::template peek<IDX__TERMINALS__IN_ORDER>()
        : parent_t::template peek<IDX__TERMINALS__OUT_OF_ORDER>();
    }
  };
}

#endif // ADIAR_INTERNAL_IO_ARC_STREAM_H
