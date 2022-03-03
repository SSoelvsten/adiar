#ifndef ADIAR_FILE_WRITER_H
#define ADIAR_FILE_WRITER_H

#include <tpie/file_stream.h>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/internal/assert.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  struct no_ordering : public std::binary_function<T, T, bool>
  {
    bool operator()(const T&, const T&) const
    { return true; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \param T    Type of the file's content
  ///
  /// \param Comp The logic to check for consistency (usually a comparator)
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = no_ordering<T>>
  class simple_file_writer
  {
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// The file stream includes a shared pointer to hook into the reference
    /// counting and garbage collection of the file.
    ////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<file<T>> _file_ptr;

    tpie::file_stream<T> _stream;

    Comp _comp = Comp();

    bool _has_latest = false;
    T _latest;

  public:
    simple_file_writer() { }
    simple_file_writer(const simple_file<T> &f) { attach(f); }

    ~simple_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const simple_file<T> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

      adiar_assert(!(_file_ptr -> is_read_only()), "Cannot attach a writer onto a read-only file");

      _stream.open(_file_ptr -> __base_file, ADIAR_WRITE_ACCESS);
      _stream.seek(0, tpie::file_stream_base::end);

      // Set up tracker of latest element added
      _has_latest = _stream.can_read_back();
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached to any file.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be).
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      adiar_assert(!(_file_ptr && _file_ptr -> is_read_only()),
                   "Stream was detached after someone started reading from it");
      _stream.close();
      // if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write an element to the file (without any checks).
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t)
    {
      _stream.write(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Sort the current content of file based on the given comparator.
    ///          Default sorting predicate is the validity checking logic.
    ////////////////////////////////////////////////////////////////////////////
    template<typename sorting_pred_t>
    void sort(sorting_pred_t pred = sorting_pred_t())
    {
      adiar_debug(attached(), "Cannot sort no content");

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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the current content of file based on the validity checker.
    ////////////////////////////////////////////////////////////////////////////
    void sort()
    {
      sort<Comp>(_comp);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the element to the file (with consistency checks).
    ////////////////////////////////////////////////////////////////////////////
    void push(const T &t)
    {
      adiar_assert(attached(), "file_writer is not yet attached to any file");

      // Check is sorted input
      adiar_assert(!_has_latest || _comp(_latest, t),
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
  /// \brief Writer to a set of file(s) with 'meta' information.
  ///
  /// \sa node_writer arc_writer
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class meta_file_writer
  {
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// The file stream includes a shared pointer to hook into the reference
    /// counting and garbage collection of the file.
    ////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<__meta_file<T>> _file_ptr;

    tpie::file_stream<level_info_t> _meta_stream;
    tpie::file_stream<T> _streams [FILE_CONSTANTS<T>::files];

  public:
    meta_file_writer() { }

    meta_file_writer(const meta_file<T> &f)
    {
      attach(f);
    }

    ~meta_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const meta_file<T> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

      adiar_assert(!(_file_ptr -> _level_info_file.is_read_only()),
                   "Cannot attach a writer onto a read-only meta file");

      _meta_stream.open(f._file_ptr -> _level_info_file.__base_file);
      _meta_stream.seek(0, tpie::file_stream_base::end);

      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        adiar_assert(!(_file_ptr -> _files[idx].is_read_only()),
                     "Cannot attach a writer onto a read-only content file");

        _streams[idx].open(f._file_ptr -> _files[idx].__base_file);
        _streams[idx].seek(0, tpie::file_stream_base::end);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _meta_stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _meta_stream.close();
      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        _streams[idx].close();
      }
      // if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Write directly (and solely) to the level_info file without any
    ///          checks for consistency.
    ///
    /// \param m Level information to push
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m)
    {
      _meta_stream.write(m);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief     Write directly (and solely) to some file without any checks
    ///            for consistency.
    ///
    /// \param t   Element to push
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t, size_t idx = 0)
    {
      adiar_debug(idx < FILE_CONSTANTS<T>::files, "Invalid index");
      _streams[idx].write(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed()
    {
      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        if (has_pushed(idx)) {
          return true;
        }
      }

      return _meta_stream.size() > 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    {
      return !has_pushed();
    }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief     Whether anything was pushed to a specific file of type \c T .
    ///
    /// \param idx Index for the file to check for any elements
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed(const size_t idx)
    {
      adiar_debug(idx < FILE_CONSTANTS<T>::files, "Invalid index");
      return _streams[idx].size() > 0;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for nodes, hiding derivation of all meta information and
  /// applying sanity checks on the validity of the input.
  ///
  /// \sa node_file
  //////////////////////////////////////////////////////////////////////////////
  class node_writer: public meta_file_writer<node_t>
  {
  private:
    node_t _latest_node = { NIL, NIL, NIL };
    bool _canonical = true;

    size_t _level_size = 0u;

  public:
    node_writer() : meta_file_writer() { }
    node_writer(const node_file &nf) : meta_file_writer(nf),
                                       _canonical(!meta_file_writer::has_pushed()
                                                  || nf._file_ptr -> canonical) { }

    ~node_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Write the next node to the file (and check consistency).
    ///
    /// \details Writes the given node to the end of the file and also writes to
    ///          the level_info file if necessary. The given node must have
    ///          valid children (not checked), no duplicate nodes created (not
    ///          properly checked), and must be topologically prior to any nodes
    ///          already written to the file (checked).
    ////////////////////////////////////////////////////////////////////////////
    void push(const node_t &n)
    {
      adiar_assert(attached(), "file_writer is not yet attached to any file");

      // Check validity of input based on latest written node
      if (!is_nil(_latest_node.uid)) {
        adiar_assert(!is_sink(_latest_node),
                     "Cannot push a node after having pushed a sink");
        adiar_assert(!is_sink(n),
                     "Cannot push a sink into non-empty file");

        // Check it is topologically sorted
        adiar_assert(n.uid < _latest_node.uid,
                     "Pushed node is required to be prior to the existing nodes");
        adiar_assert(!is_node(n.low) || label_of(n.uid) < label_of(n.low),
                     "Low child must point to a node with a higher label");
        adiar_assert(!is_node(n.high) || label_of(n.uid) < label_of(n.high),
                     "High child must point to a node with a higher label");

        // Check it is canonically sorted
        if (_canonical) {
          if (label_of(_latest_node) == label_of(n)) {
            bool id_diff = id_of(n.uid) == id_of(_latest_node) - 1u;
            bool children_ordered = n.high < _latest_node.high
              || (n.high == _latest_node.high && n.low < _latest_node.low);

            _canonical = id_diff && children_ordered;
          } else {
            bool id_reset = id_of(n) == MAX_ID;
            _canonical = id_reset;
          }
        }

        // Check if the level_info file has to be updated
        if (label_of(n) != label_of(_latest_node)) {
          meta_file_writer::unsafe_push(create_level_info(label_of(_latest_node),
                                                          _level_size));
          _level_size = 0u;
        }
      } else {
        if(!is_sink(n) && id_of(n) != MAX_ID) {
          _canonical = false;
        }
      }

      _file_ptr->true_sinks +=  (is_sink(n.low) && value_of(n.low)) +
                                (is_sink(n.high) && value_of(n.high)) +
                                (is_sink(n.uid) && value_of(n.uid));
      _file_ptr->false_sinks += (is_sink(n.low) && !value_of(n.low)) +
                                (is_sink(n.high) && !value_of(n.high)) +
                                (is_sink(n.uid) && !value_of(n.uid));

      // Write node to file
      _latest_node = n;
      _level_size++;

      meta_file_writer::unsafe_push(n, 0);
    }

    node_writer& operator<< (const node_t& n)
    {
      this -> push(n);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m) { meta_file_writer::unsafe_push(m); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the underlying node file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const node_t &n)
    {
      meta_file_writer::unsafe_push(n, 0);
      _file_ptr->true_sinks +=  (is_sink(n.low) && value_of(n.low)) +
                                (is_sink(n.high) && value_of(n.high));
      _file_ptr->false_sinks += (is_sink(n.low) && !value_of(n.low)) +
                                (is_sink(n.high) && !value_of(n.high));
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const node_file &f) {
      // TODO: set _latest_node etc. when opening file
      meta_file_writer::attach(f);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    { return meta_file_writer::attached(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach() {
      _file_ptr -> canonical = _canonical;

      if (!is_nil(_latest_node.uid) && !is_sink(_latest_node)) {
        meta_file_writer::unsafe_push(create_level_info(label_of(_latest_node),
                                                        _level_size));
        _level_size = 0u; // move to attach...
      }
      return meta_file_writer::detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed()
    { return meta_file_writer::has_pushed(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    { return meta_file_writer::empty(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for a set of arcs.
  ///
  /// \sa arc_file
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public meta_file_writer<arc_t>
  {
  private:
    bool __has_latest_sink = false;
    arc_t __latest_sink;

  public:
    arc_writer() { }
    arc_writer(const arc_file &af) {
      attach(af);
    }

    ~arc_writer()
    {
      detach();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m)
    {
      meta_file_writer::unsafe_push(m);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write an arc to the relevant underlying file without any checks
    ///
    /// \sa unsafe_push_node unsafe_push_sink
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const arc_t &a)
    {
      if (is_node(a.target)) {
        unsafe_push_node(a);
      } else { // is_sink(a.target)
        unsafe_push_sink(a);
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write an internal arc to its file, i.e. where the target is a node.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push_node(const arc_t &a)
    {
      adiar_debug(is_node(a.target), "pushing non-node arc into node file");
      meta_file_writer::unsafe_push(a, 0);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write a sink arc to its file, i.e. where the target is a sink.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push_sink(const arc_t &a)
    {
      adiar_debug(is_sink(a.target), "pushing non-sink into sink file");

      if (!__has_latest_sink || a.source > __latest_sink.source) { // in-order
        __has_latest_sink = true;
        __latest_sink = a;
        meta_file_writer::unsafe_push(a, 1);
      } else { // out-of-order
        meta_file_writer::unsafe_push(a, 2);
      }

      if(value_of(a.target)) {
        _file_ptr->true_sinks += 1;
      } else {
        _file_ptr->false_sinks += 1;
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    //////////////////////////////////////////////////////////////////////////////
    void attach(const arc_file &af) {
      meta_file_writer::attach(af);
      adiar_debug(meta_file_writer::empty(), "Attached to non-empty arc_file");
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    //////////////////////////////////////////////////////////////////////////////
    bool attached() const
    { return meta_file_writer::attached(); }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the out-of-order sink arcs and then detach from a file (if
    ///        need be).
    //////////////////////////////////////////////////////////////////////////////
    void detach() {
      if (attached() && _streams[2].size() > 0) {
        tpie::progress_indicator_null pi;
        tpie::sort(_streams[2], arc_source_lt(), pi);
      }

      return meta_file_writer::detach();
    }
  };
}

#endif // ADIAR_FILE_WRITER_H
