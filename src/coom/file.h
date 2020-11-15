#ifndef COOM_FILE_H
#define COOM_FILE_H

// STD imports for memory management, strings etc.
#include <string.h>
#include <memory>

// TPIE imports
#include <tpie/tpie.h>
#include <tpie/file.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>

// COOM imports of the foundational data structure and the union class.
#include <coom/data.h>
#include <coom/union.h>

#include <coom/assert.h>

namespace coom
{
  // TODO: we may want to add friends for the 'internal use only' functions and
  // variables!

  //////////////////////////////////////////////////////////////////////////////
  /// TPIE has many different ways to open a file
  ///
  /// - Opening through a `tpie::file<T>` allows multiple streams reading and
  ///   writing the same file simultaneously. This is slow for writing, since it
  ///   has to be synchronised across all streams.
  ///
  /// - Opening through `tpie::file_stream<T>` claims a single file on a single
  ///   stream, which means there is no cost for writing
  ///
  /// We need the first to ensure multi-access to the same BDD's, but we need
  /// the latter to not incur any performance loss in writing the output.
  ///
  /// For this we will make use of `tpie::temp_file<T>` on which we can hook the
  /// `tpie::file<T>` or the `tpie::file_stream<T>` respectively. The only thing
  /// is, that we have to be sure, that only one of the two are active at a
  /// time.
  ///
  /// TODO: Technically we may want to add a simple read/write lock on each
  /// file, but that is not currently necessary (and it does have a performance
  /// cost). Yet, the read-lock analogy we will already use in the choice of
  /// names for the class methods.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class file
  {
    static_assert(std::is_pod<T>::value, "File content must be a POD");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// Very simple write lock. This only is used to provide some better error
    /// messages for developers.
#if COOM_ASSERT
    bool write_locked = false;
#endif

    ////////////////////////////////////////////////////////////////////////////
    /// \brief For (simultaneous) read-only access to a file.
    ///
    /// This cannot be used, when 'is_read_only' is false.
    ////////////////////////////////////////////////////////////////////////////
    tpie::file<T> shared_access_file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief For write-only access to the file.
    ///
    /// Cannot be used, when 'is_read_only' is true.
    ////////////////////////////////////////////////////////////////////////////
    tpie::temp_file base_file;

    file() : base_file() {
#if COOM_ASSERT
      assert(!is_read_only());
#endif
    }

    file(const std::string &filename, bool persist = true) : base_file(filename, persist) {
#if COOM_ASSERT
      assert(!is_read_only());
#endif
    }

    ~file()
    {
      shared_access_file.close();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make file read-only, allowing multiple access via the
    /// `shared_access_file` variable.
    ///
    /// This assumes, that no `tpie::file_stream<T>` (that is, no
    /// `coom::file_writer`) currently is attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only()
    {
#if COOM_ASSERT
      coom_assert(!write_locked, "Cannot become read-only while a writer still is attached");
#endif
      if (!is_read_only()) {
        shared_access_file.open(base_file);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is currently in read-only mode.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      return shared_access_file.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the file
    ///
    /// This will make the file read-only, if it wasn't already
    ////////////////////////////////////////////////////////////////////////////
    size_t size()
    {
      make_read_only();
      return shared_access_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Same as size() == 0
    ///
    /// This will make the file read-only, if it wasn't already
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    {
      return size() == 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ///
    /// This will make the file read-only, if it wasn't already
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size()
    {
      return size() * sizeof(T);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// The core entities of COOM that represents DAGs also contain a 'meta' file,
  /// in which information is stored for the coom::priority_queue to work
  /// optimally. These entities then also store one or more files of elements of
  /// type T.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class __meta_file
  {
    static_assert(0 < Files, "The number of files must be positive");

  public:
    file<meta> _meta_file;
    file<T> _files [Files];

    __meta_file() {
#if COOM_ASSERT
      assert(!is_read_only());
#endif
    }

    // TODO: Opening a persistent file with meta information given a path.
    // __meta_file(const std::string& filename) : ? { ? }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make file read-only, allowing multiple access via the
    /// `shared_access_file` variable in the _meta_file or any of the _files.
    ///
    /// This assumes, that no `tpie::file_stream<T>` (that is, no
    /// `coom::file_writer`) currently is attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only()
    {
      _meta_file.make_read_only();
      for (size_t idx = 0; idx < Files; idx++) {
        _files[idx].make_read_only();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is currently in read-only mode.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      for (size_t idx = 0; idx < Files; idx++) {
        if (!_files[idx].is_read_only()) {
          return false;
        };
      }
      return _meta_file.is_read_only();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size()
    {
      size_t sum_size = 0;
      for(size_t idx = 0; idx < Files; idx++) {
        sum_size += _files[idx].size();
      }
      return sum_size;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the meta file
    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size()
    {
      return _meta_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size()
    {
      return size() * sizeof(T) + meta_size() * sizeof(meta_t);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// We want to be able to construct files like the ones above and return them
  /// out of a function. For that, we cannot place them on the stack, but have
  /// to place them on the heap. Yet, placing things on the heap brings with it
  /// a whole new set of problems with it. Furthermore, the user may reuse the
  /// same result in multiple places.
  ///
  /// So, we use a std::shared_ptr to be able to:
  ///
  /// - Place the files on the heap, so the __shared_file can be returned with a
  ///   copy-constructor without breaking any of the tpie::files
  ///
  /// - Provides reference counting, so everything is garbage collected as fast
  ///   as possible. With TPIE this specifically means, that disk space is freed
  ///   up as early as possible.
  ///
  /// - It is thread-safe in the reference counting, so we now have COOM to be
  ///   thread-safe for free!
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class __shared_file
  {
  public:
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

    ////////////////////////////////////////////////////////////////////////////
    /// Notice, that while the `make_read_only` and `is_read_only` are maybe not
    /// `const` for the pointed to underlying file, it is const with respect to
    /// this very object?
    void make_read_only() const
    {
      _file_ptr -> make_read_only();
    }

    bool is_read_only() const
    {
      return _file_ptr -> is_read_only();
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

  // All actual files we deal with are then actually a `__shared_file<x_file<T>>`
  template<typename T>
  using simple_file = __shared_file<file<T>>;

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

    // TODO: Files with persistent file names
    // node_file(const std::string &filename) : __shared_file(filename) { }

    node_file(const node_file &other) : __shared_file(other) { }

    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Should one have a 'maybe' reduced file with the union of a node_file and
    /// an arc_file, then we can automatically reduce it on assignment or
    /// argument passing.
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
  /// reduced BDD in an arc_file. So, the union_t will be a wrapper for the
  /// combined type.
  ///
  /// The union_t class (see union.h) uses the std::optional to ensure we don't
  /// call any expensive yet unnecessary constructors.
  //////////////////////////////////////////////////////////////////////////////
  typedef union_t<node_file, arc_file> node_or_arc_file;



  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a simple file including a consistency check
  /// on the given input.
  ///
  /// The consistency check answers, whether something is allowed to come after
  /// something else. In all our current use-cases, the check induces a total
  /// ordering.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  struct no_ordering : public std::binary_function<T, T, bool>
  {
    bool
    operator()(const T&, const T&) const
    { return true; }
  };

  template <typename T, typename Comp = no_ordering<T>>
  class simple_file_writer
  {
  protected:
    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<file<T>> _file_ptr;

    tpie::file_stream<T> _stream;

    Comp _comp = Comp();

    bool _has_latest;
    T _latest;

  public:
    simple_file_writer() { }
    simple_file_writer(const simple_file<T> &f) { attach(f); }

    ~simple_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const simple_file<T> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

#if COOM_ASSERT
      coom_assert(!(_file_ptr -> is_read_only()), "Cannot attach a writer onto a read-only file");
      _file_ptr -> write_locked = true;
#endif

      _stream.open(f._file_ptr -> base_file);
      _stream.seek(0, tpie::file_stream_base::end);

      // Set up tracker of latest element added
      _has_latest = _stream.can_read_back();
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Whether the file currently is attached. Needs to be true for any of the
    /// push functions to work.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _stream.close();
#if COOM_ASSERT
      _file_ptr -> write_locked = false;
#endif
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write the next T to the file (without any checks)
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t)
    {
      _stream.write(t);
    }

    // protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the current content of files based on the given comparator.
    /// Default is the validity check.
    ///
    /// Sorts the current pushed content based on the validity checker. This
    /// assumes, that the provided check induces a partial ordering.
    ////////////////////////////////////////////////////////////////////////////
    template<typename sorting_pred_t>
    void sort(sorting_pred_t pred = sorting_pred_t())
    {
#if COOM_ASSERT
      assert(attached());
#endif
      tpie::progress_indicator_null pi;
      tpie::sort(_stream, pred, pi);

      // Reset the _latest value.
      _stream.seek(0, tpie::file_stream_base::end);
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

  public:
    void sort()
    {
      sort<Comp>(_comp);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the next T to the file.
    ///
    /// Writes the given node to the end of the file, while it also checks it is
    /// provided in the desired order.
    ////////////////////////////////////////////////////////////////////////////
    void push(const T &t)
    {
      coom_assert(attached(), "file_writer is not yet attached to any file");

      // Check is sorted input
      coom_assert(!_has_latest || _comp(_latest, t),
                  "The given element must be provided in order");

      unsafe_push(t);
    }

    simple_file_writer<T,Comp>& operator<< (const T& t)
    {
      this -> push(t);
      return *this;
    }
  };

  typedef simple_file_writer<assignment_t, std::less<assignment_t>> assignment_writer;
  typedef simple_file_writer<label_t, no_ordering<label_t>> label_writer;



  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a file. Public usage is only relevant by use
  /// of the node_writer below and its safe `write` method.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class meta_file_writer
  {
  private:
    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<__meta_file<T, Files>> _file_ptr;

  protected:
    tpie::file_stream<meta_t> _meta_stream;
    tpie::file_stream<T> _streams [Files];

  public:
    meta_file_writer() { }

    meta_file_writer(const meta_file<T, Files> &f)
    {
      attach(f);
    }

    ~meta_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const meta_file<T, Files> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

#if COOM_ASSERT
      coom_assert(!(_file_ptr -> _meta_file.is_read_only()), "Cannot attach a writer onto a read-only meta file");
      _file_ptr -> _meta_file.write_locked = true;
#endif
      _meta_stream.open(f._file_ptr -> _meta_file.base_file);
      _meta_stream.seek(0, tpie::file_stream_base::end);

      for (size_t idx = 0; idx < Files; idx++) {
#if COOM_ASSERT
        coom_assert(!(_file_ptr -> _files[idx].is_read_only()), "Cannot attach a writer onto a read-only content file");
        _file_ptr -> _files[idx].write_locked = true;
#endif
        _streams[idx].open(f._file_ptr -> _files[idx].base_file);
        _streams[idx].seek(0, tpie::file_stream_base::end);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Whether the file currently is attached. Needs to be true for any of the
    /// push functions to work.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _meta_stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _meta_stream.close();
#if COOM_ASSERT
      _file_ptr -> _meta_file.write_locked = false;
#endif
      for (size_t idx = 0; idx < Files; idx++) {
        _streams[idx].close();
#if COOM_ASSERT
        _file_ptr -> _files[idx].write_locked = false;
#endif
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to the meta file without any checks
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const meta_t &m)
    {
      _meta_stream.write(m);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to the nodes file without any checks
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t, size_t idx = 0)
    {
#if COOM_ASSERT
      assert(idx < Files);
#endif
      _streams[idx].write(t);
    }

    bool has_pushed()
    {
      for (size_t idx = 0; idx < Files; idx++) {
        if (has_pushed(idx)) {
          return true;
        }
      }

      return _meta_stream.size() > 0;
    }

    bool empty()
    {
      return !has_pushed();
    }

  protected:
    bool has_pushed(const size_t idx)
    {
#if COOM_ASSERT
      assert(idx < Files);
#endif
      return _streams[idx].size() > 0;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Provides write-only access to a file of nodes; hiding all details about
  /// the meta file and providing sanity checks on the input on user-created
  /// BDDs.
  //////////////////////////////////////////////////////////////////////////////
  class node_writer: public meta_file_writer<node_t, 1>
  {
  private:
    bool _has_latest = false;
    uid_t _latest = NIL;

  public:
    node_writer() : meta_file_writer() { }
    node_writer(const node_file &nf) : meta_file_writer(nf) { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the next node to the file.
    ///
    /// Writes the given node to the end of the file and also writes to the meta
    /// file if necessary. The given node must have valid children (not checked)
    /// and must be topologically prior to any nodes already written to the
    /// file (checked).
    ////////////////////////////////////////////////////////////////////////////
    void push(const node_t &n)
    {
      coom_assert(attached(), "file_writer is not yet attached to any file");

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
      meta_file_writer::unsafe_push(n, 0);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to meta file without any checks
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const meta_t &m) { meta_file_writer::unsafe_push(m); }

    ////////////////////////////////////////////////////////////////////////////
    /// Write directly (and solely) to nodes file without any checks
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const node_t &n) { meta_file_writer::unsafe_push(n, 0); }

    ////////////////////////////////////////////////////////////////////////////
    void attach(const node_file &f) { meta_file_writer::attach(f); }
    bool attached() const { return meta_file_writer::attached(); }
    void detach() { return meta_file_writer::detach(); }

    bool has_pushed() { return meta_file_writer::has_pushed(); }
    bool empty() { return meta_file_writer::empty(); }
  };

  node_writer& operator<< (node_writer& nw, const node_t& n);


  //////////////////////////////////////////////////////////////////////////////
  /// FOR INTERNAL USE ONLY.
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public meta_file_writer<arc_t, 2>
  {
  public:
    arc_writer() { }
    arc_writer(const arc_file &af) {
      attach(af);
    }

    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const meta_t &m)
    {
      meta_file_writer::unsafe_push(m);
    }

    void unsafe_push_node(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_node_ptr(a.target));
#endif
      meta_file_writer::unsafe_push(a, 0);
    }

    void unsafe_push_sink(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_sink_ptr(a.target));
#endif
      meta_file_writer::unsafe_push(a, 1);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Most algorithms are able to output everything in-order. Yet, the
    /// Restrict and Quantify algorithms, since they skip levels, may output the
    /// sinks out-of-order.
    ///
    /// Since the correctness of all algorithms rely on the ordering of the
    /// input, then we need to be able to fix this. TPIE provides a sorting of a
    /// file_stream, which minimises the space usage as much as possible.
    ///
    /// One could think to instead always have the sinks in a tpie::merge_sorter
    /// and then pull them out, since the arc_file anyways only contains
    /// intermediate output. The time difference between writing an
    /// in-order-list to a stream is much lower than sorting an already sorted
    /// list. So, on average this seems to be better (and simpler).
    ////////////////////////////////////////////////////////////////////////////
    void sort_sinks()
    {
#if COOM_ASSERT
      assert(attached());
#endif
      tpie::progress_indicator_null pi;
      tpie::sort(_streams[1], by_source_lt(), pi);
    }

    ////////////////////////////////////////////////////////////////////////////
    void attach(const arc_file &af) {
      meta_file_writer::attach(af);
#if COOM_ASSERT
      assert(meta_file_writer::empty());
#endif
    }

    bool attached() const { return meta_file_writer::attached(); }

    void detach() { return meta_file_writer::detach(); }
  };



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

#endif // COOM_FILE_H
