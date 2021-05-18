#ifndef ADIAR_FILE_WRITER_H
#define ADIAR_FILE_WRITER_H

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/assert.h>

namespace adiar {
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

    bool _has_latest = false;
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
      adiar_assert(!(_file_ptr && _file_ptr -> is_read_only()),
                   "Stream was detached after someone started reading from it");
      _stream.close();
      // if (_file_ptr) { _file_ptr.reset(); }
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
  /// Provides write-only access to a file. Public usage is only relevant by use
  /// of the node_writer below and its safe `write` method.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class meta_file_writer
  {
  protected:
    // Keep a local shared_ptr to be in on the reference counting
    std::shared_ptr<__meta_file<T, Files>> _file_ptr;

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

      adiar_assert(!(_file_ptr -> _meta_file.is_read_only()),
                   "Cannot attach a writer onto a read-only meta file");

      _meta_stream.open(f._file_ptr -> _meta_file.__base_file);
      _meta_stream.seek(0, tpie::file_stream_base::end);

      for (size_t idx = 0; idx < Files; idx++) {
        adiar_assert(!(_file_ptr -> _files[idx].is_read_only()),
                     "Cannot attach a writer onto a read-only content file");

        _streams[idx].open(f._file_ptr -> _files[idx].__base_file);
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
      for (size_t idx = 0; idx < Files; idx++) {
        _streams[idx].close();
      }
      // if (_file_ptr) { _file_ptr.reset(); }
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
      adiar_debug(idx < Files, "Invalid index");
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
      adiar_debug(idx < Files, "Invalid index");
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
    /// \brief Write the next node to the file.
    ///
    /// Writes the given node to the end of the file and also writes to the meta
    /// file if necessary. The given node must have valid children (not
    /// checked), no duplicate nodes created (not properly checked), and must be
    /// topologically prior to any nodes already written to the file (checked).
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

        // Check if the meta file has to be updated
        if (label_of(n) != label_of(_latest_node)) {
          meta_file_writer::unsafe_push(create_meta(label_of(_latest_node),
                                                    _level_size));
          _level_size = 0u;
        }
      }

      // Write node to file
      _latest_node = n;
      _level_size++;

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
    void attach(const node_file &f) {
      // TODO: set _latest_node etc. when opening file
      meta_file_writer::attach(f);
    }
    bool attached() const { return meta_file_writer::attached(); }
    void detach() {
      _file_ptr -> canonical = _canonical;

      if (!is_nil(_latest_node.uid) && !is_sink(_latest_node)) {
        meta_file_writer::unsafe_push(create_meta(label_of(_latest_node),
                                                  _level_size));
        _level_size = 0u; // move to attach...
      }
      return meta_file_writer::detach();
    }

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
      adiar_debug(is_node(a.target), "pushing non-node arc into node file");
      meta_file_writer::unsafe_push(a, 0);
    }

    void unsafe_push_sink(const arc_t &a)
    {
      adiar_debug(is_sink(a.target), "pushing non-sink into sink file");
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
      adiar_debug(attached(), "Missing arc_file attached");
      tpie::progress_indicator_null pi;
      tpie::sort(_streams[1], by_source_lt(), pi);
    }

    ////////////////////////////////////////////////////////////////////////////
    void attach(const arc_file &af) {
      meta_file_writer::attach(af);
      adiar_debug(meta_file_writer::empty(), "Attached to non-empty arc_file");
    }

    bool attached() const { return meta_file_writer::attached(); }

    void detach() { return meta_file_writer::detach(); }
  };
}

#endif // ADIAR_FILE_WRITER_H
