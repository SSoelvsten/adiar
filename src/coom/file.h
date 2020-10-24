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
  template <typename T>
  using shared_file = typename std::shared_ptr<tpie::file<T>>;

  // TODO: we may want to add friends for the 'internal use only' functions and
  // variables!

  //////////////////////////////////////////////////////////////////////////////
  /// Object containing a pointer to a single TPIE file
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class simple_file
  {
  private:
    static_assert(std::is_pod<T>::value, "File content must be a POD");

    void init_file()
    {
      _file = std::make_shared<tpie::file<T>>();
    }

  public:
    shared_file<T> _file;

    simple_file()
    {
      init_file();
      _file -> open();
    }

    simple_file(const std::string &filename)
    {
      init_file();
      _file -> open(filename);
    }

    simple_file(const simple_file<T> &other)
    {
      _file = other._file;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return _file -> size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(T);
    }
  };

  typedef simple_file<assignment_t> assignment_file;
  typedef simple_file<label_t> label_file;

  //////////////////////////////////////////////////////////////////////////////
  /// Object containing a pointer to multiple TPIE files, that combined
  /// constitute a single entity of COOM.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class file
  {
  private:
    static_assert(0 < Files, "The number of files must be positive");
    static_assert(std::is_pod<T>::value, "File content must be a POD");

    void init_files()
    {
      _meta_file = std::make_shared<tpie::file<meta_t>>();
      for (size_t idx = 0; idx < Files; idx++) {
        _files[idx] = std::make_shared<tpie::file<T>>();
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// FOR INTERNAL USE ONLY. DO NOT USE!
    ///
    /// The TPIE sub files from which the entire COOM is created.
    ////////////////////////////////////////////////////////////////////////////
    shared_file<meta_t> _meta_file;
    shared_file<T> _files [Files];

    ////////////////////////////////////////////////////////////////////////////
    /// Instantiate a temporary COOM file
    ////////////////////////////////////////////////////////////////////////////
    file()
    {
      init_files();
      _meta_file -> open();

      for (size_t idx = 0; idx < Files; idx++) {
        _files[idx] -> open();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Instantiate a COOM file given a filename
    ////////////////////////////////////////////////////////////////////////////
    file(const std::string &filename)
    {
      init_files();
      _meta_file -> open(filename + ".meta", tpie::access_read_write);

      if constexpr (Files == 1) {
        _files[0] -> open(filename, tpie::access_read_write);
      } else {
        for (size_t idx = 0; idx < Files; idx++) {
          std::string ith_filename = filename + "_" + std::to_string(idx);
          _files[idx] -> open(ith_filename, tpie::access_read_write);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Copy-constructor to instantiate a COOM file, pointing to the same as
    /// another COOM file.
    ////////////////////////////////////////////////////////////////////////////
    file(const file<T, Files> &f)
    {
      _meta_file = f._meta_file;

      for (size_t idx = 0; idx < Files; idx++) {
        _files[idx] = f._files[idx];
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the main files (excluding the meta_file)
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t sum_size = 0;
      for(size_t idx = 0; idx < Files; idx++) {
        sum_size += _files[idx] -> size();
      }
      return sum_size;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The number of elements in the meta file
    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size() const
    {
      return _meta_file -> size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The total size of the files (including the meta_file) in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(T) + meta_size() * sizeof(meta_t);
    }
  };

  template<typename T, size_t Files>
  bool operator==(const file<T, Files> &f1, const file<T, Files> &f2)
  {
    // TPIE does not allow one to open the same file with multiple objects, so
    // we only need to check equivalence of the underlying file object.
    //
    // We don't need to check on all objects, since we aim to have the bundle of
    // files to be disjunctly associated with eachother
    return f1._meta_file == f2._meta_file;
  }

  template<typename T, size_t Files>
  bool operator!=(const file<T, Files> &f1, const file<T, Files> &f2)
  {
    return !(f1 == f2);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// An unreduced OBDD is given by its node-to-node arcs and its node-to-sink
  /// arcs.
  //////////////////////////////////////////////////////////////////////////////
  typedef file<arc_t, 2> arc_file;

  //////////////////////////////////////////////////////////////////////////////
  /// A reduced OBDD is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  class node_file : public file<node_t, 1>
  {
  public:
    node_file() : file() { }
    node_file(const std::string &filename) : file(filename) { }
    node_file(const node_file &other) : file(other) { }

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
  /// An algorithm may return a sink-only OBDD in a node_file or a yet to-be
  /// reduced OBDD in an arc_file. So, the union_t will keep be a wrapper for
  /// the type.
  //////////////////////////////////////////////////////////////////////////////
  typedef union_t<node_file, arc_file> node_or_arc_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given node_file is sink-only and satisfies the
  /// given sink_pred.
  ///
  /// \param file   The node_file to check its content
  /// \param pred   If the given OBDD is sink-only, then secondly the sink is
  ///               checked with the given sink predicate. Default is any type
  ///               sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const node_or_arc_file &file, const sink_pred &pred = is_any);

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
    shared_file<T> _file;
    typename tpie::file<T>::stream _stream;

    Check _check = Check();

    bool _has_latest;
    T _latest;

  public:
    simple_file_writer(const simple_file<T> &f) : _file(f._file)
    {
      _stream.attach(*f._file);
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
  class file_writer
  {
  private:
    file<T, Files> _file;

  protected:
    shared_file<meta_t> _meta_file;
    shared_file<T> _files [Files];

    tpie::file<meta_t>::stream _meta_stream;
    typename tpie::file<T>::stream _streams [Files];

  public:
    file_writer(const file<T, Files> &f) : _file(f)
    {
      _meta_stream.attach(*f._meta_file);
      _meta_stream.seek(0, tpie::file_base::stream::end);

      for (size_t idx = 0; idx < Files; idx++) {
        _streams[idx].attach(*f._files[idx]);
        _streams[idx].seek(0, tpie::file_base::stream::end);
      }
    }

    ~file_writer()
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
  class node_writer: public file_writer<node_t, 1>
  {
  private:
    bool _has_latest = false;
    uid_t _latest = NIL;

  public:
    node_writer(const node_file &nf) : file_writer(nf) { }
    // TODO: Copy constructor

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
        file_writer::unsafe_push(meta_t { label_of(n) });
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
      file_writer::unsafe_push(m);
    }

    inline void unsafe_push(const node_t &n)
    {
      file_writer::unsafe_push(n, 0);
    }
  };

  node_writer& operator<< (node_writer& nw, const node_t& n);

  //////////////////////////////////////////////////////////////////////////////
  /// FOR INTERNAL USE ONLY.
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public file_writer<arc_t, 2>
  {
  public:
    arc_writer(const arc_file &af) : file_writer(af)
    {
#if COOM_ASSERT
      assert(af.size() == 0);
      assert(af.meta_size() == 0);
#endif
    }
    // TODO: Copy constructor

    inline void unsafe_push(const meta_t &m)
    {
      file_writer::unsafe_push(m);
    }

    inline void unsafe_push_node(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_node_ptr(a.target));
#endif
    file_writer::unsafe_push(a, 0);
    }

    inline void unsafe_push_sink(const arc_t &a)
    {
#if COOM_ASSERT
      assert(is_sink_ptr(a.target));
#endif
      file_writer::unsafe_push(a, 1);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Provides a read-only access to a single sub_file of a file_stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, bool REVERSE = false>
  class file_stream
  {
  private:
    bool _has_peeked = false;
    T _peeked;
    bool _negate = false;

    shared_file<T> _file;
    typename tpie::file<T>::stream _stream;

  public:
    file_stream(const shared_file<T> &f, bool negate = false) : _negate(negate), _file(f)
    {
      _stream.attach(*f);
      reset();
    }

    file_stream(const simple_file<T> &f, bool negate = false) : _negate(negate), _file(f._file)
    {
      _stream.attach(*_file);
      reset();
    }

    ~file_stream()
    {
      if (_stream.attached()) {
        _stream.detach();
      }
    }

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

  typedef file_stream<assignment_t, false> assignment_stream;
  typedef file_stream<label_t, false> label_stream;

  template <typename T, size_t Files>
  class meta_stream : public file_stream<meta_t, true>
  {
  public:
    meta_stream(const file<T,Files> &f) : file_stream(f._meta_file, false) { }
  };

  class node_stream: public file_stream<node_t, true>
  {
  public:
    node_stream(const node_file &f, bool negate = false) : file_stream(f._files[0], negate)
    {
#if COOM_ASSERT
      assert(f.size() > 0);
#endif
    }
  };

  class node_arc_stream: public file_stream<arc_t, true>
  {
  public:
    node_arc_stream(const arc_file &f, bool negate = false) : file_stream(f._files[0], negate) { }
  };

  class sink_arc_stream: public file_stream<arc_t, true>
  {
  public:
    sink_arc_stream(const arc_file &f, bool negate = false) : file_stream(f._files[1], negate) { }
  };
}

#endif // COOM_FILE_H
