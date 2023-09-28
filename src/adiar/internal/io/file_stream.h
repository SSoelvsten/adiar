#ifndef ADIAR_INTERNAL_IO_FILE_STREAM_H
#define ADIAR_INTERNAL_IO_FILE_STREAM_H

#include <tpie/tpie.h>
#include <tpie/sort.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/io/file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream to a file with a one-way reading direction.
  ///
  /// \tparam T       The type of the file's elements
  ///
  /// \tparam Reverse Whether the reading direction should be reversed
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, bool Reverse = false>
  class file_stream
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    using value_type = T;

  public:
    ////////////////////////////////////////////////////////////////////////////
    static size_t memory_usage()
    {
      return tpie::file_stream<value_type>::memory_usage();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer of a single element, since TPIE does not support a
    ///        `peek_back` function yet (TPIE Issue #187).
    ////////////////////////////////////////////////////////////////////////////
    mutable value_type _peeked;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether an \em unpulled element is stored in `_peeked`.
    ////////////////////////////////////////////////////////////////////////////
    mutable bool _has_peeked = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements should be \em negated on-the-fly.
    ////////////////////////////////////////////////////////////////////////////
    bool _negate = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief TPIE's file stream object to read the file with.
    ////////////////////////////////////////////////////////////////////////////
    mutable typename tpie::file_stream<value_type> _stream;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief If attached to a shared file then hook into the reference
    ///        counting such that the file is not garbage collected while we
    ///        read from it.
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<void> _file_ptr;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    file_stream() { }

    ////////////////////////////////////////////////////////////////////////////
    file_stream(const file_stream<value_type, Reverse> &) = delete;
    file_stream(file_stream<value_type, Reverse> &&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given shared `file<value_type>`.
    ////////////////////////////////////////////////////////////////////////////
    file_stream(const file<value_type> &f,
                bool negate = false)
    { attach(f, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given shared `file<value_type>`.
    ////////////////////////////////////////////////////////////////////////////
    file_stream(const adiar::shared_ptr<file<value_type>> &f,
                bool negate = false)
    { attach(f, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~file_stream()
    { detach(); }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    void attach(const file<value_type> &f,
                const adiar::shared_ptr<void> &shared_ptr,
                bool negate)
    {
      // Detach from prior file, if any.
      if (attached()) { detach(); }

      // Hook into reference counting.
      _file_ptr = shared_ptr;

      // Touch the file to make sure it exists on disk. Since 'f' is const, use
      // the private '__touch()' member function instead.
      f.__touch();

      // Open the stream to the file
      _stream.open(f._tpie_file, file<value_type>::read_access);
      reset();

      // Store negation flag.
      _negate = negate;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to the above 'attach'.
    template <typename tparam__elem_t, bool tparam__Reverse>
    friend class levelized_file_stream;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file.
    ///
    /// \pre No `file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const file<value_type> &f,
                bool negate = false)
    {
      attach(f, nullptr, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared file.
    ///
    /// \pre No `file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const adiar::shared_ptr<file<value_type>> &f,
                bool negate = false)
    {
      attach(*f, f, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the reader is currently attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from the file, i.e. close the stream.
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _stream.close();
      if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the read head back to the beginning (relatively to the
    ///        reading direction).
    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      if constexpr (Reverse) {
        _stream.seek(0, tpie::file_stream_base::end);
      } else {
        _stream.seek(0);
      }
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    bool __can_read() const
    {
      if constexpr (Reverse) {
        return _stream.can_read_back();
      } else {
        return _stream.can_read();
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull() const
    { return _has_peeked || __can_read(); }

  private:
    ////////////////////////////////////////////////////////////////////////////
    const value_type __read()
    {
      value_type v;
      if constexpr (Reverse) {
        v = _stream.read_back();
      } else {
        v = _stream.read();
      }
      return _negate ? ~v : v;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    const value_type pull()
    {
       adiar_assert(can_pull());
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      return __read();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head)
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    const value_type peek()
    {
      adiar_assert(can_pull());
      if (!_has_peeked) {
        _peeked = __read();
        _has_peeked = true;
      }
      return _peeked;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Obtain the next element that is greater than or equal to the
    ///          given target value.
    ///
    /// \param s The value to seek for
    ///
    /// \pre     The content is in ascending order and `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////
    template<typename seek_t = value_type>
    const value_type seek(seek_t tgt)
    {
      // Notice, the peek() also changes the state of '_peeked' and
      // '_has_peeked'. This initializes the invariant for 'seek' that '_peeked'
      // is the "current position".
      if (tgt < peek()) { return _peeked; }

      // Only move to "next value", if there is more to pull.
      while (_peeked < tgt && __can_read()) { _peeked = __read(); }
      return _peeked;
    }
  };
}

#endif // ADIAR_INTERNAL_IO_FILE_STREAM_H
