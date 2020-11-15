#ifndef COOM_FILE_WRITER_H
#define COOM_FILE_WRITER_H

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <coom/data.h>
#include <coom/file.h>

#include <coom/assert.h>

namespace coom {
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
}

#endif // COOM_FILE_WRITER_H
