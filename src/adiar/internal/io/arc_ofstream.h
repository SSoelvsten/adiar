#ifndef ADIAR_INTERNAL_IO_ARC_OFSTREAM_H
#define ADIAR_INTERNAL_IO_ARC_OFSTREAM_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/levelized_ofstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for a set of arcs.
  ///
  /// \see shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class arc_ofstream : public levelized_ofstream<arc>
  {
  private:
    bool __has_latest_terminal = false;
    arc __latest_terminal;

  private:
    static constexpr size_t idx__internal = file_traits<arc>::idx__internal;

    static constexpr size_t idx__terminals__in_order = file_traits<arc>::idx__terminals__in_order;

    static constexpr size_t idx__terminals__out_of_order =
      file_traits<arc>::idx__terminals__out_of_order;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any levelized arc file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ofstream()
      : levelized_ofstream<arc>()
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized arc file.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ofstream(levelized_file<arc>& af)
      : levelized_ofstream<arc>()
    {
      open(af);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized arc file.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ofstream(shared_ptr<levelized_file<arc>> af)
      : levelized_ofstream<arc>()
    {
      open(af);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up from the levelized file (if need be).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~arc_ofstream()
    {
      // Sort the out-of-order terminals with 'close()'.
      close();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open a levelized file of arcs.
    ///
    /// \warning Since ownership is \em not shared with this writer, you have to ensure, that the
    ///          file in question is not destructed before `.close()` is called.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(levelized_file<arc>& af)
    {
      if (is_open()) close();
      adiar_assert(af.empty());

      // TODO: remove precondition and set up __latest_terminal.

      return levelized_ofstream::open(af);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open a shared levelized file of arcs.
    ///
    /// \pre The file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(shared_ptr<levelized_file<arc>>& af)
    {
      if (is_open()) close();
      adiar_assert(af->empty());

      // TODO: remove precondition and set up __latest_terminal.

      return levelized_ofstream::open(af);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches after having sort the out-of-order terminals.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void
    close()
    {
      if (!is_open()) return;

#ifdef ADIAR_STATS
      if (_elem_ofstreams[idx__terminals__out_of_order].size() != 0u) {
        stats_arc_file.sort_out_of_order += 1;
      }
#endif
      _elem_ofstreams[idx__terminals__out_of_order].sort<arc_source_lt>();
      levelized_ofstream::close();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `is_open() == true`.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const level_info& li)
    {
#ifdef ADIAR_STATS
      stats_arc_file.push_level += 1;
#endif
      _file_ptr->width = std::max<size_t>(_file_ptr->width, li.width());
      levelized_ofstream::push(li);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `is_open() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ofstream&
    operator<<(const level_info& li)
    {
      this->push(li);
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an arc to the relevant underlying file.
    ///
    /// \param a An arc with `a.target() != a::pointer_type::nil`.
    ///
    /// \pre `is_open() == true`.
    ///
    /// \see unsafe_push_internal unsafe_push_terminal
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const arc& a)
    {
      adiar_assert(!a.target().is_nil(), "Should not push an arc to nil.");
      if (a.target().is_node()) {
        push_internal(a);
      } else { // a.target().is_terminal()
        push_terminal(a);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an ac to the relevant underlying file.
    ///
    /// \pre `is_open() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ofstream&
    operator<<(const arc& a)
    {
      this->push(a);
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an internal arc to its file, i.e. where the target is a node.
    ///
    /// \pre `is_open() == true`.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push_internal(const arc& a)
    {
      adiar_assert(is_open());
      adiar_assert(a.target().is_node());
      adiar_assert(!a.source().is_nil());
#ifdef ADIAR_STATS
      stats_arc_file.push_internal += 1;
#endif
      levelized_ofstream::template push<idx__internal>(a);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push a terminal arc to its file, i.e. where the target is a terminal.
    ///
    /// \pre `is_open() == true`.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push_terminal(const arc& a)
    {
      adiar_assert(is_open());
      adiar_assert(a.target().is_terminal());
      adiar_assert(!a.source().is_nil());

      if (!__has_latest_terminal || a.source() > __latest_terminal.source()) {
        // Given arc is 'in-order' compared to latest 'in-order' pushed
        __has_latest_terminal = true;
        __latest_terminal     = a;
#ifdef ADIAR_STATS
        stats_arc_file.push_in_order += 1;
#endif
        levelized_ofstream::template push<idx__terminals__in_order>(a);
      } else {
        // Given arc is 'out-of-order' compared to latest 'in-order' pushed
#ifdef ADIAR_STATS
        stats_arc_file.push_out_of_order += 1;
#endif
        levelized_ofstream::template push<idx__terminals__out_of_order>(a);
      }

      _file_ptr->number_of_terminals[a.target().value()]++;
    }
  };
}

#endif // ADIAR_INTERNAL_IO_ARC_OFSTREAM_H
