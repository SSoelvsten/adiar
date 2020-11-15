#ifndef COOM_FILE_STREAM_H
#define COOM_FILE_STREAM_H

#include <tpie/tpie.h>

#include <coom/data.h>
#include <coom/file.h>

#include <coom/assert.h>

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  /// Provides a read-only access to a file. Here we internalise the logic to
  /// hook into the file in question.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, bool REVERSE = false, typename SharedPtr_T = file<T>>
  class file_stream
  {
  private:
    bool _has_peeked = false;
    T _peeked;
    bool _negate = false;

    // Use a stream on the shared_access_file to allow simultaneous reads
    typename tpie::file<T>::stream _stream;

    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<SharedPtr_T> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
  protected:
    file_stream() { }

    file_stream(file<T> &f,
                const std::shared_ptr<SharedPtr_T> &shared_ptr,
                bool negate = false)
    {
      attach(f, shared_ptr, negate);
    }

    void attach(file<T> &f,
                const std::shared_ptr<SharedPtr_T> &shared_ptr,
                bool negate = false)
    {
#if COOM_ASSERT
      coom_assert(!f.write_locked, "Cannot attach a stream to a file on which still a writer is attached");
#endif
      if (attached()) { detach(); }

      _file_ptr = shared_ptr;
      if (!_file_ptr -> is_read_only()) { _file_ptr -> make_read_only(); }

      _stream.attach(f.shared_access_file);
      _negate = negate;

      reset();
    }

    ////////////////////////////////////////////////////////////////////////////
  public:
    file_stream(const std::shared_ptr<file<T>> &f, bool negate = false)
    {
      attach(*f, f, negate);
    }

    file_stream(const simple_file<T> &f, bool negate = false) : file_stream(f._file_ptr, negate) { }

    ~file_stream()
    {
#if COOM_ASSERT
      assert(_stream.attached());
#endif
      _stream.detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      if constexpr (REVERSE) {
        _stream.seek(0, tpie::file_base::stream::end);
      } else {
        _stream.seek(0);
      }
    }

    bool can_pull()
    {
      return _has_peeked
        || (REVERSE ? _stream.can_read_back() : _stream.can_read());
    }

    const T pull()
    {
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      T t = REVERSE ? _stream.read_back() : _stream.read();
      return _negate ? !t : t;
    }

    const T peek()
    {
      if (!_has_peeked) {
        _peeked = pull();
        _has_peeked = true;
      }
      return _peeked;
    }

    ////////////////////////////////////////////////////////////////////////////
    void attach(const simple_file<T> &f, bool negate = false)
    {
      attach(f, f._file_ptr, negate);
    }

    bool attached()
    {
      return _stream.attached();
    }

    void detach()
    {
      _stream.detach();
      _file_ptr.reset();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// File streams for 'simple' files.
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  using assignment_stream = file_stream<assignment_t, REVERSE>;

  template<bool REVERSE = false>
  using label_stream = file_stream<label_t, REVERSE>;

  //////////////////////////////////////////////////////////////////////////////
  /// For file streams of the elements of a file with meta information, we
  /// create one stream type, per underlying file.
  ///
  ///  - TODO: Could we need an abstracted file_stream merger, like the
  ///          pq_label_mgr we have made for the priority_queue?
  ///
  /// Since all files with meta information are written to in reverse, then
  /// 'reversing' the stream (from the point of sorting) is equivalent to not
  /// reversing the underlying stream.
  ///
  /// Default will be to read in-order of the elements. We may change the hidden
  /// flip of the REVERSE value, but you can always rely on the default value
  /// will result in the same behaviour.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files, size_t File, bool REVERSE = false>
  class meta_file_stream : public file_stream<T, REVERSE, __meta_file<T, Files>>
  {
    static_assert(File < Files, "The file to pick must be a valid index");

  public:
    meta_file_stream(const meta_file<T, Files> &file, bool negate = false)
      : file_stream<T, REVERSE, __meta_file<T, Files>>(file._file_ptr -> _files[File], file._file_ptr, negate)
    { }

    // TODO: 'attach', 'attached', and 'detach'
  };

  template<bool REVERSE = false>
  using node_stream = meta_file_stream<node_t, 1, 0, !REVERSE>;

  template<bool REVERSE = false>
  using node_arc_stream = meta_file_stream<arc_t, 2, 0, !REVERSE>;

  template<bool REVERSE = false>
  using sink_arc_stream = meta_file_stream<arc_t, 2, 1, !REVERSE>;

  //////////////////////////////////////////////////////////////////////////////
  /// For file streams of elements of a file with meta information, we also
  /// provide a stream to read content of the underlying meta file.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files, bool REVERSE = false>
  class meta_stream : public file_stream<meta_t, !REVERSE, __meta_file<T, Files>>
  {
  public:
    meta_stream(const meta_file<T,Files> &f)
      : file_stream<meta_t, !REVERSE, __meta_file<T, Files>>(f._file_ptr -> _meta_file, f._file_ptr)
    { }

    // TODO: 'attach', 'attached', and 'detach'
  };
}

#endif // COOM_FILE_STREAM_H
