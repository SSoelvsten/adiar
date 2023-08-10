#ifndef ADIAR_INTERNAL_IO_ARC_WRITER_H
#define ADIAR_INTERNAL_IO_ARC_WRITER_H

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/levelized_file_writer.h>
#include <adiar/internal/io/arc_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for a set of arcs.
  ///
  /// \sa shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public levelized_file_writer<arc>
  {
  private:
    bool __has_latest_terminal = false;
    arc __latest_terminal;

  private:
    static constexpr size_t IDX__INTERNAL =
      file_traits<arc>::IDX__INTERNAL;

    static constexpr size_t IDX__TERMINALS__IN_ORDER =
      file_traits<arc>::IDX__TERMINALS__IN_ORDER;

    static constexpr size_t IDX__TERMINALS__OUT_OF_ORDER =
      file_traits<arc>::IDX__TERMINALS__OUT_OF_ORDER;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any levelized arc file.
    ////////////////////////////////////////////////////////////////////////////
    arc_writer() : levelized_file_writer<arc>()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized arc file.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////
    arc_writer(levelized_file<arc> &af)
      : levelized_file_writer<arc>()
    {
      attach(af);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized arc file.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////
    arc_writer(shared_ptr<levelized_file<arc>> af)
      : levelized_file_writer<arc>()
    {
      attach(af);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up from the levelized file (if need be).
    ////////////////////////////////////////////////////////////////////////////
    ~arc_writer()
    {
      // Sort the out-of-order terminals with 'detach()'.
      detach();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a levelized file of arcs.
    ///
    /// \warning Since ownership is \em not shared with this writer, you have to
    ///          ensure, that the file in question is not destructed before
    ///          `.detach()` is called.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////
    void attach(levelized_file<arc> &af) {
      if (attached()) detach();
      adiar_debug(af.empty());

      // TODO: remove precondition and set up __latest_terminal.

      return levelized_file_writer::attach(af);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared levelized file of arcs.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////
    void attach(shared_ptr<levelized_file<arc>> &af) {
      if (attached()) detach();
      adiar_debug(af->empty());

      // TODO: remove precondition and set up __latest_terminal.

      return levelized_file_writer::attach(af);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches after having sort the out-of-order terminals.
    //////////////////////////////////////////////////////////////////////////////
    void detach() {
      if (!attached()) return;

#ifdef ADIAR_STATS
      if (_elem_writers[IDX__TERMINALS__OUT_OF_ORDER].size() != 0u) {
        stats_arc_file.sort_out_of_order += 1;
      }
#endif
      _elem_writers[IDX__TERMINALS__OUT_OF_ORDER].sort<arc_source_lt>();
      levelized_file_writer::detach();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `attached() == true`.
    //////////////////////////////////////////////////////////////////////////////
    void push(const level_info &li)
    {
#ifdef ADIAR_STATS
      stats_arc_file.push_level += 1;
#endif
      levelized_file_writer::push(li);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    arc_writer& operator<< (const level_info& li)
    {
      this->push(li);
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Push an arc to the relevant underlying file.
    ///
    /// \param a An arc with `a.target() != a::ptr_t::NIL`.
    ///
    /// \pre `attached() == true`.
    ///
    /// \sa unsafe_push_internal unsafe_push_terminal
    //////////////////////////////////////////////////////////////////////////////
    void push(const arc &a)
    {
      adiar_debug(!a.target().is_nil(), "Should not push an arc to NIL.");
      if (a.target().is_node()) {
        push_internal(a);
      } else { // a.target().is_terminal()
        push_terminal(a);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an ac to the relevant underlying file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    arc_writer& operator<< (const arc& a)
    {
      this->push(a);
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Push an internal arc to its file, i.e. where the target is a node.
    ///
    /// \pre `attached() == true`.
    //////////////////////////////////////////////////////////////////////////////
    void push_internal(const arc &a)
    {
      adiar_debug(attached());
      adiar_debug(a.target().is_node());
      adiar_debug(!a.source().is_nil());
#ifdef ADIAR_STATS
      stats_arc_file.push_internal += 1;
#endif
      levelized_file_writer::template push<IDX__INTERNAL>(a);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Push a terminal arc to its file, i.e. where the target is a terminal.
    ///
    /// \pre `attached() == true`.
    //////////////////////////////////////////////////////////////////////////////
    void push_terminal(const arc &a)
    {
      adiar_debug(attached());
      adiar_debug(a.target().is_terminal());
      adiar_debug(!a.source().is_nil());

      if (!__has_latest_terminal || a.source() > __latest_terminal.source()) {
        // Given arc is 'in-order' compared to latest 'in-order' pushed
        __has_latest_terminal = true;
        __latest_terminal = a;
#ifdef ADIAR_STATS
        stats_arc_file.push_in_order += 1;
#endif
        levelized_file_writer::template push<IDX__TERMINALS__IN_ORDER>(a);
      } else {
        // Given arc is 'out-of-order' compared to latest 'in-order' pushed
#ifdef ADIAR_STATS
        stats_arc_file.push_out_of_order += 1;
#endif
        levelized_file_writer::template push<IDX__TERMINALS__OUT_OF_ORDER>(a);
      }

      _file_ptr->number_of_terminals[a.target().value()]++;
    }
  };
}

#endif // ADIAR_INTERNAL_IO_ARC_WRITER_H
