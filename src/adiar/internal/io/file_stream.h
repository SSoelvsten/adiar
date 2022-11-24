#ifndef ADIAR_FILE_STREAM_H
#define ADIAR_FILE_STREAM_H

#include <tpie/tpie.h>
#include <tpie/sort.h>

#include <adiar/assignment.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream to a file with a one-way reading direction.
  ///
  /// \param elem_type   The type of the file's elements
  ///
  /// \param REVERSE     Whether the reading direction should be reversed
  ///
  /// \param file_t      The type of the shared pointer to a file
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type, bool REVERSE = false, typename file_type = file<elem_type>>
  class file_stream
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

  public:
    ////////////////////////////////////////////////////////////////////////////
    static size_t memory_usage()
    {
      return tpie::file_stream<elem_t>::memory_usage();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer of a single element, since TPIE does not support peeking
    ///        in revese.
    ////////////////////////////////////////////////////////////////////////////
    elem_t _peeked;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether an \em unpulled element is stored in `_peeked`.
    ////////////////////////////////////////////////////////////////////////////
    bool _has_peeked = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements should be \em negated on-the-fly.
    ////////////////////////////////////////////////////////////////////////////
    bool _negate = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief TPIE's file stream object to read the file with.
    ////////////////////////////////////////////////////////////////////////////
    typename tpie::file_stream<elem_t> _stream;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief If attached to a shared file then hook into the reference
    ///        counting such that the file is not garbage collected while we
    ///        read from it.
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<file_type> _file_ptr;

  protected:
    void attach(file<elem_t> &f,
                const shared_ptr<file_type> &shared_ptr,
                bool negate = false)
    {
      if (attached()) { detach(); }

      _file_ptr = shared_ptr;

      _stream.open(f._tpie_file, file<elem_t>::read_access);
      _negate = negate;

      reset();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct an unattached file stream.
    ////////////////////////////////////////////////////////////////////////////
    file_stream() { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a file stream attached to the given shared file.
    ////////////////////////////////////////////////////////////////////////////
    file_stream(const shared_ptr<file<elem_t>> &f, bool negate = false)
    {
      attach(*f, f, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a file stream attached to the given shared file.
    ////////////////////////////////////////////////////////////////////////////
    file_stream(const simple_file<elem_t> &f, bool negate = false)
      : file_stream(f._file_ptr, negate)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~file_stream()
    {
      detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const simple_file<elem_t> &f, bool negate = false)
    {
      attach(f, f._file_ptr, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the reader is currently attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached()
    {
      return _stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from the file, i.e. close the stream.
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _stream.close();
      // if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the read head back to the beginning (relatively to the
    ///        reading direction).
    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      if constexpr (REVERSE) {
        _stream.seek(0, tpie::file_stream_base::end);
      } else {
        _stream.seek(0);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      return _has_peeked
        || (REVERSE ? _stream.can_read_back() : _stream.can_read());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (and move the read head)
    ////////////////////////////////////////////////////////////////////////////
    const elem_t pull()
    {
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      const elem_t t = REVERSE ? _stream.read_back() : _stream.read();
      return _negate ? !t : t;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head)
    ////////////////////////////////////////////////////////////////////////////
    const elem_t peek()
    {
      if (!_has_peeked) {
        _peeked = pull();
        _has_peeked = true;
      }
      return _peeked;
    }
  };





  //////////////////////////////////////////////////////////////////////////////
  /// \brief         File stream of levelized files.
  ///
  /// \param T       The type of the file(s)'s elements
  ///
  /// \param File    Index for the file to read from
  ///
  /// \param REVERSE Whether the reading direction should be reversed
  ///
  /// \remark Since all files are written to in reverse of the desired reading
  ///         order, then 'reversing' the reversed input is equivalent to not
  ///         reversing the underlying stream. Hence, we do hide a a negation of
  ///         the \c REVERSE parameter.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, size_t file_idx, bool REVERSE = false>
  class levelized_file_stream : public file_stream<elem_t, REVERSE, __levelized_file<elem_t>>
  {
    static_assert(file_idx < FILE_CONSTANTS<elem_t>::files, "The file to pick must be a valid index");

  public:
    levelized_file_stream(const levelized_file<elem_t> &file, bool negate = false)
    {
      file_stream<elem_t, REVERSE, __levelized_file<elem_t>>::attach(file->_files[file_idx], file._file_ptr, negate);
    }

    // TODO: 'attach', 'attached', and 'detach'
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief         Stream of nodes from a node file.
  ///
  /// \param REVERSE Whether the reading direction should be reversed
  ///                (relatively to the ordering of nodes within the file).
  ///
  /// \sa node_file
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  class node_stream : public levelized_file_stream<node, 0, !REVERSE>
  {
  public:
    node_stream(const node_file &file, bool negate = false)
      : levelized_file_stream<node, 0, !REVERSE>(file, negate)
    { }

    node_stream(const dd &diagram)
      : levelized_file_stream<node, 0, !REVERSE>(diagram.file, diagram.negate)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream for internal (reversed) arcs of an arc file.
  ///
  /// \sa arc_file
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  using node_arc_stream = levelized_file_stream<arc, 0, !REVERSE>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream for terminal arcs of an arc file.
  ///
  /// \sa arc_file
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  class terminal_arc_stream
  {
  private:
    typedef levelized_file_stream<arc, 1, !REVERSE> in_order_t;
    in_order_t in_order;

    typedef levelized_file_stream<arc, 2, !REVERSE> out_of_order_t;
    out_of_order_t out_of_order;

    size_t _unread[2] = { 0u, 0u };

  public:
    static size_t memory_usage()
    {
      return in_order_t::memory_usage() + out_of_order_t::memory_usage();
    }

  public:
    terminal_arc_stream(const arc_file &file, bool negate = false)
      : in_order(file, negate)
      , out_of_order(file, negate)
      , _unread{ file->number_of_terminals[false], file->number_of_terminals[true] }
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the number of unread terminals of a specific value.
    ////////////////////////////////////////////////////////////////////////////
    const size_t& unread(const bool terminal_value) const
    {
      return _unread[terminal_value];
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the next element ought to be pulled from `in_order`.
    ////////////////////////////////////////////////////////////////////////////
    bool next_from_in_order()
    {
      const bool in_order_pull = in_order.can_pull();
      const bool out_of_order_pull = out_of_order.can_pull();

      if (in_order_pull != out_of_order_pull) {
        return in_order_pull;
      }

      const arc::ptr_t in_order_source = in_order.peek().source();
      const arc::ptr_t out_of_order_source = out_of_order.peek().source();

      return (REVERSE && in_order_source < out_of_order_source)
         || (!REVERSE && in_order_source > out_of_order_source);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the read head back to the beginning (relatively to the
    ///        reading direction).
    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      in_order.reset();
      out_of_order.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more arcs.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      return in_order.can_pull() || out_of_order.can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (and move the read head)
    ////////////////////////////////////////////////////////////////////////////
    const arc pull()
    {
      const arc a = next_from_in_order() ? in_order.pull() : out_of_order.pull();
      _unread[a.target().value()]--;
      return a;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (but do not move the read head)
    ////////////////////////////////////////////////////////////////////////////
    const arc peek()
    {
      return next_from_in_order() ? in_order.peek() : out_of_order.peek();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream for the levelized meta information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool REVERSE = false>
  class level_info_stream : public file_stream<level_info_t, !REVERSE, __levelized_file<elem_t>>
  {
  public:
    //////////////////////////////////////////////////////////////////////////////
    /// Access the level information of a file with meta information.
    //////////////////////////////////////////////////////////////////////////////
    level_info_stream(const levelized_file<elem_t> &f)
    {
      file_stream<level_info_t, !REVERSE, __levelized_file<elem_t>>::attach(f->_level_info_file, f._file_ptr);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// Access the level information stream of a decision diagram.
    //////////////////////////////////////////////////////////////////////////////
    level_info_stream(const dd &diagram)
      : level_info_stream(diagram.file)
    { }

  private:
    //////////////////////////////////////////////////////////////////////////////
    /// For unit testing only!
    //////////////////////////////////////////////////////////////////////////////
    levelized_file<elem_t> __obtain_file(const __dd &diagram)
    {
      if constexpr (std::is_same<node, elem_t>::value) {
        return diagram.get<node_file>();
      } else {
        return diagram.get<arc_file>();
      }
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    /// For unit testing only!
    ///
    /// Access to level information of an unreduced decision diagram.
    //////////////////////////////////////////////////////////////////////////////
    level_info_stream(const __dd &diagram)
      : level_info_stream(__obtain_file(diagram))
    { }

    // TODO: 'attach', 'attached', and 'detach'
  };
}

#endif // ADIAR_FILE_STREAM_H
