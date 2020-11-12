#ifndef COOM_FILE_H
#define COOM_FILE_H

#include <string.h>
#include <memory>
#include <optional>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <coom/data.h>
#include <coom/union.h>

#include <coom/assert.h>

namespace coom
{
  // TODO: we may want to add friends for the 'internal use only' functions and
  // variables!

  //////////////////////////////////////////////////////////////////////////////
  /// Object containing a pointer to a single TPIE file
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class __simple_file
  {
    static_assert(std::is_pod<T>::value, "File content must be a POD");

  public:
    tpie::file<T> _file;

    __simple_file()
    {
      _file.open();
    }

    __simple_file(const std::string &filename)
    {
      _file.open(filename);
    }

    ~__simple_file()
    {
      _file.close();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return _file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(T);
    }
  };

  template <typename T, size_t Files>
  class __meta_file
  {
    static_assert(0 < Files, "The number of files must be positive");
    static_assert(std::is_pod<T>::value, "File content must be a POD");

  public:
    tpie::file<meta> _meta_file;
    tpie::file<T> _files [Files];

    __meta_file()
    {
      _meta_file.open();
      for (size_t idx = 0; idx < Files; idx++) {
        _files[idx].open();
      }
    }

    __meta_file(const std::string& filename)
    {
      _meta_file.open(filename + ".meta", tpie::access_read_write);
      if constexpr (Files == 1) {
        _files[0].open(filename, tpie::access_read_write);
      } else {
        for (size_t idx = 0; idx < Files; idx++) {
          _files[idx].open(filename + "_" + std::to_string(idx), tpie::access_read_write);
        }
      }
    }

    ~__meta_file()
    {
      _meta_file.close();
      for (size_t idx = 0; idx < Files; idx++) { _files[idx].close(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t sum_size = 0;
      for(size_t idx = 0; idx < Files; idx++) {
        sum_size += _files[idx].size();
      }
      return sum_size;
    }

    size_t meta_size() const
    {
      return _meta_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(T) + meta_size() * sizeof(meta_t);
    }
  };


  template <typename T>
  class __shared_file
  {
  public:
    // Use of a std::shared_ptr has the following benefits
    //
    // - The TPIE files are placed on the heap, which means we can return a
    //   __shared_file constructed from within a function without the TPIE files
    //   being garbage collected with the stack frame.
    //
    // - Provides reference counting, so the TPIE files are garbage collected as
    //   early as possible.
    //
    // - Is thread-safe in the reference counting, so we allow COOM to be
    //   thread-safe.
    std::shared_ptr<T> _file_ptr;

    __shared_file()
    {
      _file_ptr = std::make_shared<T>();
    }

    __shared_file(const std::string &filename)
    {
      _file_ptr = std::make_shared<T>(filename);
    }

    __shared_file(const __shared_file<T> &other)
    {
      _file_ptr = other._file_ptr;
    }

    size_t size() const
    {
      return _file_ptr -> size();
    }

    size_t file_size() const
    {
      return _file_ptr -> file_size();
    }
  };

  template<typename T>
  bool operator==(const __shared_file<T> &f1, const __shared_file<T> &f2)
  {
    return f1._file_ptr == f2._file_ptr;
  }

  template<typename T, size_t Files>
  bool operator!=(const __shared_file<T> &f1, const __shared_file<T> &f2)
  {
    return !(f1 == f2);
  }

  template<typename T>
  using simple_file = __shared_file<__simple_file<T>>;

  typedef simple_file<assignment_t> assignment_file;
  typedef simple_file<label_t> label_file;

  template<typename T, size_t Files>
  using meta_file = __shared_file<__meta_file<T,Files>>;

  //////////////////////////////////////////////////////////////////////////////
  /// An unreduced BDD is given by a two files of arcs; one of node-to-node arcs
  /// (in reverse topological order) and one of node-to-sink arcs (in
  /// topological order).
  //////////////////////////////////////////////////////////////////////////////
  typedef meta_file<arc_t, 2> arc_file;


  //////////////////////////////////////////////////////////////////////////////
  /// A reduced BDD is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  class node_file : public meta_file<node_t, 1>
  {
  public:
    node_file() : __shared_file() { }
    node_file(const std::string &filename) : __shared_file(filename) { }
    node_file(const node_file &other) : __shared_file(other) { }

    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Should one have a 'maybe' reduced file with the union of a node_file and
    /// an arc_file, then reduce it first.
    ////////////////////////////////////////////////////////////////////////////

    // TODO: Move this reduce logic into the BDD object?
    node_file(const union_t<node_file, arc_file> &node_or_arc_file);
    node_file& operator= (const union_t<node_file, arc_file> &other);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given node_file is sink-only and satisfies the
  /// given sink_pred.
  ///
  /// \param file   The node_file to check its content
  /// \param pred   If the given OBDD is sink-only, then secondly the sink is
  ///               checked with the given sink predicate. Default is any type
  ///               sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const node_file &file, const sink_pred &pred = is_any);

  node_t root_of(const node_file &file);

  label_t min_label(const node_file &file);
  label_t max_label(const node_file &file);


  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based BDD in a node_file or a yet to-be
  /// reduced BDD in an arc_file. So, the union_t will keep be a wrapper for the
  /// combined type.
  ///
  /// The union_t class (see union.h) uses the std::optional to ensure we don't
  /// call any expensive constructors.
  //////////////////////////////////////////////////////////////////////////////
  typedef union_t<node_file, arc_file> node_or_arc_file;


  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a simple file including a consistency check
  /// on the given input.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  struct no_check : public std::binary_function<T, T, bool>
  {
    bool
    operator()(const T&, const T&) const
    { return true; }
  };

  template <typename T, typename Check = no_check<T>>
  class simple_file_writer
  {
  protected:
    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<__simple_file<T>> _file_ptr;

    typename tpie::file<T>::stream _stream;

    Check _check = Check();

    bool _has_latest;
    T _latest;

  public:
    simple_file_writer(const simple_file<T> &f) : _file_ptr(f._file_ptr)
    {
      _stream.attach(f._file_ptr -> _file);
      _stream.seek(0, tpie::file_base::stream::end);

      // Set up tracker of latest element added
      _has_latest = _stream.can_read_back();
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

    ~simple_file_writer()
    {
      _stream.detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write the next T to the file (without any checks)
    ////////////////////////////////////////////////////////////////////////////
    inline void unsafe_push(const T &t)
    {
      _stream.write(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the next T to the file.
    ///
    /// Writes the given node to the end of the file, while it also checks it is
    /// provided in the desired order.
    ////////////////////////////////////////////////////////////////////////////
    inline void push(const T &t)
    {
      // Check is sorted input
      coom_assert(!_has_latest || _check(_latest, t),
                  "The given element must be provided in increasing order");

      unsafe_push(t);
    }

    simple_file_writer<T,Check>& operator<< (const T& t)
    {
      this -> push(t);
      return *this;
    }
  };

  typedef simple_file_writer<assignment_t, std::less<assignment_t>> assignment_writer;
  typedef simple_file_writer<label_t, no_check<label_t>> label_writer;

  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a file. Public usage is only relevant by use
  /// of the node_writer below and its safe `write` method.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class meta_file_writer
  {
  protected:
    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<__meta_file<T, Files>> _file_ptr;

    tpie::file<meta_t>::stream _meta_stream;
    typename tpie::file<T>::stream _streams [Files];

  public:
    meta_file_writer(const meta_file<T, Files> &f) : _file_ptr(f._file_ptr)
    {
      _meta_stream.attach(f._file_ptr -> _meta_file);
      _meta_stream.seek(0, tpie::file_base::stream::end);

      for (size_t idx = 0; idx < Files; idx++) {
        _streams[idx].attach(f._file_ptr -> _files[idx]);
        _streams[idx].seek(0, tpie::file_base::stream::end);
      }
    }

    ~meta_file_writer()
    {
      _meta_stream.detach();
      for (size_t idx = 0; idx < Files; idx++) {
        _streams[idx].detach();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to the meta file without any checks
    ////////////////////////////////////////////////////////////////////////////
    inline void unsafe_push(const meta_t &m)
    {
      _meta_stream.write(m);
    }

    inline void unsafe_push(const T &t, size_t idx = 0)
    {
#if COOM_ASSERT
      assert(idx < Files);
#endif
      _streams[idx].write(t);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a file, hiding all details about the meta
  /// file.
  //////////////////////////////////////////////////////////////////////////////
  class node_writer: public meta_file_writer<node_t, 1>
  {
  private:
    bool _has_latest = false;
    uid_t _latest = NIL;

  public:
    node_writer(const node_file &nf) : meta_file_writer(nf) { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the next node to the file.
    ///
    /// Writes the given node to the end of the file and also writes to the meta
    /// file if necessary. The given node must have valid children (not checked)
    /// and must be topologically prior to any nodes already written to the
    /// file (checked).
    ////////////////////////////////////////////////////////////////////////////
    inline void push(const node_t &n)
    {
      // Check latest was not a sink
      coom_assert(!_has_latest || !is_sink_ptr(_latest),
                  "Cannot push a node after having pushed a sink");
      coom_assert(!_has_latest || !is_sink(n),
                  "Cannot push a sink into non-empty file");

      // Check it is topologically sorted
      if (_has_latest) {
        coom_assert(n.uid < _latest,
                    "Pushed node is required to be prior to the existing nodes");
        coom_assert(!is_node_ptr(n.low) || label_of(n.uid) < label_of(n.low),
                    "Low child must point to a node with a higher label");
        coom_assert(!is_node_ptr(n.high) || label_of(n.uid) < label_of(n.high),
                    "High child must point to a node with a higher label");
      }

      // Check if meta file has to be updated
      if ((!_has_latest && !is_sink(n)) ||
          ( _has_latest && label_of(n) != label_of(_latest))) {
        meta_file_writer::unsafe_push(meta_t { label_of(n) });
      }

      // Write node to file
      _has_latest = true;
      _latest = n.uid;
      unsafe_push(n);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to node file without any checks
    ////////////////////////////////////////////////////////////////////////////
    inline void unsafe_push(const meta_t &m)
    {
      meta_file_writer::unsafe_push(m);
    }

    inline void unsafe_push(const node_t &n)
    {
      meta_file_writer::unsafe_push(n, 0);
    }
  };

  node_writer& operator<< (node_writer& nw, const node_t& n);

  //////////////////////////////////////////////////////////////////////////////
  /// FOR INTERNAL USE ONLY.
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public meta_file_writer<arc_t, 2>
  {
  public:
    arc_writer(const arc_file &af) : meta_file_writer(af)
    {
#if COOM_ASSERT
      assert(af.file_size() == 0);
#endif
    }
    // TODO: Copy constructor

    inline void unsafe_push(const meta_t &m)
    {
      meta_file_writer::unsafe_push(m);
    }

    inline void unsafe_push_node(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_node_ptr(a.target));
#endif
      meta_file_writer::unsafe_push(a, 0);
    }

    inline void unsafe_push_sink(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_sink_ptr(a.target));
#endif
      meta_file_writer::unsafe_push(a, 1);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Provides a read-only access to a single tpie::file<T>. This base class
  /// does not 'claim' ownership of the original file in any way. That is, TPIE
  /// errors may occur, if the original file is garbage collected, before this
  /// file_stream is.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename SharedPtr_T, bool REVERSE = false>
  class file_stream
  {
  private:
    bool _has_peeked = false;
    T _peeked;
    bool _negate = false;

    typename tpie::file<T>::stream _stream;

    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<SharedPtr_T> _file_ptr;

  protected:
    file_stream(tpie::file<T> &f,
                const std::shared_ptr<SharedPtr_T> &shared_ptr,
                bool negate = false)
      : _negate(negate), _file_ptr(shared_ptr)
    {
      _stream.attach(f);
      reset();
    }

  public:
    ~file_stream()
    {
#if COOM_ASSERT
      assert(_stream.attached());
#endif
      _stream.detach();
    }

  public:
    inline void reset()
    {
      if constexpr (REVERSE) {
        _stream.seek(0, tpie::file_base::stream::end);
      } else {
        _stream.seek(0);
      }
    }

    inline bool can_pull()
    {
      return _has_peeked
        || (REVERSE ? _stream.can_read_back() : _stream.can_read());
    }

    inline const T pull()
    {
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      T t = REVERSE ? _stream.read_back() : _stream.read();
      return _negate ? !t : t;
    }

    inline const T peek()
    {
      if (!_has_peeked) {
        _peeked = pull();
        _has_peeked = true;
      }
      return _peeked;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// File streams for simple files.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T, bool REVERSE = false>
  class simple_file_stream : public file_stream<T, __simple_file<T>, REVERSE>
  {
  public:
    simple_file_stream(const simple_file<T> &f, bool negate = false)
      : file_stream<T, __simple_file<T>, REVERSE>(f._file_ptr -> _file, f._file_ptr, negate)
    { }
  };

  typedef simple_file_stream<assignment_t, false> assignment_stream;
  typedef simple_file_stream<label_t, false> label_stream;

  //////////////////////////////////////////////////////////////////////////////
  /// File streams for files with meta information
  //////////////////////////////////////////////////////////////////////////////
  template <typename File_T, size_t Files, size_t File, typename Elem_T, bool REVERSE = false>
  class meta_file_stream : public file_stream<Elem_T, __meta_file<File_T, Files>, REVERSE>
  {
    static_assert(File < Files, "The file to pick must be a valid index");

  protected:
    meta_file_stream(const std::shared_ptr<__meta_file<File_T, Files>> &shared_ptr,
                     tpie::file<Elem_T> &file,
                     bool negate = false)
      : file_stream<Elem_T, __meta_file<File_T, Files>, REVERSE>(file, shared_ptr, negate)
    { }

  public:
    meta_file_stream(const meta_file<File_T, Files> &file, bool negate = false)
      : meta_file_stream(file._file_ptr, file._file_ptr -> _files[File], negate)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Since all files with meta information are written to in reverse, then
  /// 'reversing' the stream (from the point of sorting) is equivalent to not
  /// reversing the underlying stream.
  ///
  /// Default will be to read in-order of the elements. We may change the hidden
  /// flip of the REVERSE value, but you can always rely on the default value
  /// will result in the same behaviour.
  template <typename T, size_t Files, bool REVERSE = false>
  class meta_stream : public meta_file_stream<T, Files, 0, meta_t, !REVERSE>
  {
  public:
    meta_stream(const meta_file<T,Files> &f)
      : meta_file_stream<T, Files, 0, meta_t, !REVERSE>(f._file_ptr, f._file_ptr -> _meta_file, false) { }
  };

  template<bool REVERSE = false>
  using node_stream = meta_file_stream<node_t, 1, 0, node_t, !REVERSE>;

  template<bool REVERSE = false>
  using node_arc_stream = meta_file_stream<arc_t, 2, 0, arc_t, !REVERSE>;

  template<bool REVERSE = false>
  using sink_arc_stream = meta_file_stream<arc_t, 2, 1, arc_t, !REVERSE>;
}

#endif // COOM_FILE_H
