#ifndef COOM_FILE_H
#define COOM_FILE_H

// STD imports for memory management, strings etc.
#include <string.h>
#include <memory>

// TPIE imports
#include <tpie/tpie.h>
#include <tpie/file.h>

// COOM imports of the foundational data structure and the union class.
#include <coom/data.h>

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
      coom_debug(!is_read_only(), "Created read-only");
    }

    file(const std::string &filename, bool persist = true) : base_file(filename, persist) {
      coom_debug(!is_read_only(), "Created read-only");
      // TODO: Make read only, if non-empty?
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
      return size() == 0u;
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
      coom_debug(!is_read_only(), "Created read-only");
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
      for (size_t idx = 0u; idx < Files; idx++) {
        _files[idx].make_read_only();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is currently in read-only mode.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      for (size_t idx = 0u; idx < Files; idx++) {
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
      size_t sum_size = 0u;
      for(size_t idx = 0; idx < Files; idx++) {
        sum_size += _files[idx].size();
      }
      return sum_size;
    }

    bool empty()
    {
      return size() == 0u;
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
    // TODO: make the pointer private?
    std::shared_ptr<T> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
    /// Construct a temporary shared file
    ////////////////////////////////////////////////////////////////////////////
    __shared_file() : _file_ptr(std::make_shared<T>()) { }

    ////////////////////////////////////////////////////////////////////////////
    /// Construct a persistent shared file given a specific filename
    ////////////////////////////////////////////////////////////////////////////
    __shared_file(const std::string &filename) : _file_ptr(std::make_shared<T>(filename)) { }

    // Copy constructor
    __shared_file(const __shared_file<T> &other) : _file_ptr(other._file_ptr) { }

    // Move constructor
    __shared_file(__shared_file<T> &&other) : _file_ptr(other._file_ptr) { }

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

    bool empty() const
    {
      return _file_ptr -> empty();
    }

    size_t file_size() const
    {
      return _file_ptr -> file_size();
    }

    ////////////////////////////////////////////////////////////////////////////
    __shared_file<T>& operator= (const __shared_file<T> &o)
    {
      _file_ptr = o._file_ptr;
      return *this;
    }

    __shared_file<T>& operator= (__shared_file<T> &&o)
    {
      _file_ptr = o._file_ptr;
      return *this;
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
  /// An unreduced Decision Diagram is given by a two files of arcs; one of
  /// node-to-node arcs (in reverse topological order) and one of node-to-sink
  /// arcs (in topological order).
  //////////////////////////////////////////////////////////////////////////////
  typedef meta_file<arc_t, 2> arc_file;


  //////////////////////////////////////////////////////////////////////////////
  /// A reduced Decision Diagram is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  // TODO: This is only made to add the `meta_size` function. Do we actually
  // care for that?

  class node_file : public meta_file<node_t, 1>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// Construct a temporary node_file
    ////////////////////////////////////////////////////////////////////////////
    node_file() : __shared_file() { }

    // TODO: Files with persistent file names
    // node_file(const std::string &filename) : __shared_file(filename) { }

    // Copy constructor
    node_file(const node_file &other) : __shared_file(other) { }

    // Move constructor
    node_file(node_file &&other) : __shared_file(other) { }

    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }

    ////////////////////////////////////////////////////////////////////////////
    node_file& operator= (const node_file &o)
    {
      _file_ptr = o._file_ptr;
      return *this;
    }

    node_file& operator= (node_file &&o)
    {
      _file_ptr = o._file_ptr;
      return *this;
    }
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

  label_t min_label(const node_file &file);
  label_t max_label(const node_file &file);


  uint64_t nodecount(const node_file &nodes);
  uint64_t nodecount(const arc_file &arcs);

  uint64_t varcount(const node_file &nodes);
}

#endif // COOM_FILE_H
