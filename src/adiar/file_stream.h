#ifndef ADIAR_FILE_STREAM_H
#define ADIAR_FILE_STREAM_H

#include <tpie/tpie.h>
#include <tpie/sort.h>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/bdd/bdd.h>

#include <adiar/assert.h>

namespace adiar {
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
    typename tpie::file_stream<T> _stream;

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
      if (attached()) { detach(); }

      _file_ptr = shared_ptr;
      _file_ptr -> make_read_only();

      _stream.open(f.__base_file, ADIAR_READ_ACCESS);
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
      detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      if constexpr (REVERSE) {
        _stream.seek(0, tpie::file_stream_base::end);
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
      return _stream.is_open();
    }

    void detach()
    {
      _stream.close();
      // if (_file_ptr) { _file_ptr.reset(); }
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
  class node_stream : public meta_file_stream<node_t, NODE_FILE_COUNT, 0, !REVERSE>
  {
  public:
    node_stream(const node_file &file, bool negate = false)
      : meta_file_stream<node_t, 1, 0, !REVERSE>(file, negate)
    { }

    node_stream(const decision_diagram &dd)
      : meta_file_stream<node_t, 1, 0, !REVERSE>(dd.file, dd.negate)
    { }
  };

  template<bool REVERSE = false>
  using node_arc_stream = meta_file_stream<arc_t, ARC_FILE_COUNT, 0, !REVERSE>;

  template<bool REVERSE = false>
  using in_order_arc_stream = meta_file_stream<arc_t, ARC_FILE_COUNT, 1, !REVERSE>;

  template<bool REVERSE = false>
  using out_of_order_arc_stream = meta_file_stream<arc_t, ARC_FILE_COUNT, 2, !REVERSE>;

  template<bool REVERSE = false>
  class sink_arc_stream
    : private in_order_arc_stream<REVERSE>, private out_of_order_arc_stream<REVERSE>
  {
  public:
    sink_arc_stream(const arc_file &file, bool negate = false)
      : in_order_arc_stream<REVERSE>(file, negate),
        out_of_order_arc_stream<REVERSE>(file, negate)
    { }

  private:
    bool pull_from_in_order()
    {
      bool in_order_pull = in_order_arc_stream<REVERSE>::can_pull();
      bool out_of_order_pull = out_of_order_arc_stream<REVERSE>::can_pull();

      if (in_order_pull != out_of_order_pull) {
        return in_order_pull;
      }

      ptr_t in_order_source = in_order_arc_stream<REVERSE>::peek().source;
      ptr_t out_of_order_source = out_of_order_arc_stream<REVERSE>::peek().source;

      return (REVERSE && in_order_source < out_of_order_source)
        || (!REVERSE && in_order_source > out_of_order_source);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      in_order_arc_stream<REVERSE>::reset();
      out_of_order_arc_stream<REVERSE>::reset();
    }

    bool can_pull()
    {
      return in_order_arc_stream<REVERSE>::can_pull()
        || out_of_order_arc_stream<REVERSE>::can_pull();
    }

    const arc_t pull()
    {
      return pull_from_in_order()
        ? in_order_arc_stream<REVERSE>::pull()
        : out_of_order_arc_stream<REVERSE>::pull();
    }

    const arc_t peek()
    {
      return pull_from_in_order()
        ? in_order_arc_stream<REVERSE>::peek()
        : out_of_order_arc_stream<REVERSE>::peek();
    }
  };

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

    meta_stream(const decision_diagram &dd) : meta_stream(dd.file) { }

    // Only used for testing...
    meta_stream(const __decision_diagram &dd);

    // TODO: 'attach', 'attached', and 'detach'
  };
}

#endif // ADIAR_FILE_STREAM_H
