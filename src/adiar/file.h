#ifndef ADIAR_FILE_H
#define ADIAR_FILE_H

// STD imports for memory management, strings etc.
#include <string.h>
#include <memory>

// TPIE imports
#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/file.h>
#include <tpie/sort.h>

// ADIAR imports
#include <adiar/data.h>
#include <adiar/internal/assert.h>

namespace adiar
{
#define ADIAR_READ_ACCESS tpie::access_type::access_read
#define ADIAR_WRITE_ACCESS tpie::access_type::access_write

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Wrapper for TPIE's <tt>temp_file</tt>.
  ///
  /// \param T Type of the file's content
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class file
  {
    static_assert(std::is_pod<T>::value, "File content must be a POD");

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file has been read (and hence should not be further)
    ////////////////////////////////////////////////////////////////////////////
    mutable bool __is_read_only = false;

  public: // TODO: Privatize and make friends
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The underlying TPIE file
    ////////////////////////////////////////////////////////////////////////////
    tpie::temp_file __base_file;

  private:
    void touch_file()
    {
      // Opening the file with 'access_read_write' automatically creates the
      // file with header on disk.
      tpie::file_stream<T> fs;
      fs.open(__base_file, tpie::access_type::access_read_write);
    }

  public:
    file() : __base_file() { touch_file(); }

    file(const std::string &filename) : __base_file(filename, true) {
      touch_file();
      if (!empty()) { make_read_only(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Mark file to be read-only.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only() const
    {
      __is_read_only = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is currently in read-only mode, i.e. some
    /// adiar::file_stream has been attached to it and has marked it.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      return __is_read_only;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size()
    {
      tpie::file_stream<T> fs;
      fs.open(__base_file, ADIAR_READ_ACCESS);
      return fs.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    {
      return size() == 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the file in bytes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size()
    {
      return size() * sizeof(T);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       File(s) with meta information.
  ///
  /// \param T     Type of the file's content
  ///
  /// \param Files Number of files of type T
  ///
  /// \details     The entities of Adiar that represents DAGs, which are
  ///              represented by one or more files of type <tt>T</tt>. To
  ///              optimise several algorithms, these files also carry around
  ///              'meta' information. This information is stored in the
  ///              variables and in a levelized fashion depending on the
  ///              granularity of the information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class __meta_file
  {
    static_assert(0 < Files, "The number of files must be positive");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Boolean flag whether a set of nodes are well-formed with respect
    ///        to the stricter ordering required by the fast equality check.
    ////////////////////////////////////////////////////////////////////////////
    bool canonical = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Meta information on a level by level granularity.
    ////////////////////////////////////////////////////////////////////////////
    file<level_info> _level_info_file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Files describing the directed acyclic graph.
    ////////////////////////////////////////////////////////////////////////////
    file<T> _files [Files];

    __meta_file() {
      adiar_debug(!is_read_only(), "Should be writable on creation");
    }

    // TODO: Opening a persistent file with meta information given a path.
    // __meta_file(const std::string& filename) : ? { ? }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Make the file read-only. This disallows use of any writers but
    ///         (multiple) access by several readers.
    ///
    /// \remark Any writer should be detached from this object before making it
    ///         read only.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only() const
    {
      _level_info_file.make_read_only();
      for (size_t idx = 0u; idx < Files; idx++) {
        _files[idx].make_read_only();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is read-only.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      for (size_t idx = 0u; idx < Files; idx++) {
        if (!_files[idx].is_read_only()) {
          return false;
        };
      }
      return _level_info_file.is_read_only();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the file(s)
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
    /// \brief The number of elements in the levelized meta information file
    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size()
    {
      return _level_info_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The size of the file(s) in bytes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size()
    {
      return size() * sizeof(T) + meta_size() * sizeof(level_info_t);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// We want to be able to construct files like the ones above and return them
  /// out of a function. For that, we cannot place them on the stack, but have
  /// to place them on the heap. Yet, placing things on the heap brings with it
  /// a whole new set of problems. Furthermore, the user may reuse the same
  /// result in multiple places.
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
  /// - It is thread-safe in the reference counting, so we now have ADIAR to be
  ///   thread-safe for free!
  ///
  /// \param T The type of the underlying file
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class __shared_file
  {
  public:
    // TODO: make the pointer private?
    std::shared_ptr<T> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a temporary shared file
    ////////////////////////////////////////////////////////////////////////////
    __shared_file() : _file_ptr(std::make_shared<T>()) { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a persistent shared file given a specific filename
    ////////////////////////////////////////////////////////////////////////////
    __shared_file(const std::string &filename) : _file_ptr(std::make_shared<T>(filename)) { }

    // Copy constructor
    __shared_file(const __shared_file<T> &other) : _file_ptr(other._file_ptr) { }

    // Move constructor
    __shared_file(__shared_file<T> &&other) : _file_ptr(other._file_ptr) { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Make the file read-only. This disallows use of any writers but
    ///         (multiple) access by several readers.
    ///
    /// \remark Any writer should be detached from this object before making it
    ///         read only.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only() const
    {
      _file_ptr -> make_read_only();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is read-only.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      return _file_ptr -> is_read_only();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return _file_ptr -> size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are no elements in the file
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return _file_ptr -> empty();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The size of the file in bytes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return _file_ptr -> file_size();
    }

    __shared_file<T>& operator= (const __shared_file<T> &o) = default;
    __shared_file<T>& operator= (__shared_file<T> &&o) = default;
  };


  ////////////////////////////////////////////////////////////////////////////
  /// The <tt>file</tt> and <tt>__meta_file</tt> classes are hidden behind a
  /// shared pointer, such that we can parse it around. That is, all actual
  /// files we are going to deal with are a <tt>__shared_file<x_file<T>></tt>.
  ///
  /// \param T Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  using simple_file = __shared_file<file<T>>;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Sorts the content of a <tt>simple_file</tt> given some sorting
  /// predicate.
  ////////////////////////////////////////////////////////////////////////////
  template<typename T, typename sorting_pred_t = std::less<>>
  void sort(simple_file<T> f, sorting_pred_t pred = sorting_pred_t())
  {
    adiar_assert(!f.is_read_only(), "Cannot sort file after read-access");

    tpie::file_stream<T> fs;
    fs.open(f._file_ptr -> __base_file);

    tpie::progress_indicator_null pi;
    tpie::sort(fs, pred, pi);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A file of assignments (label, value)
  ////////////////////////////////////////////////////////////////////////////
  typedef simple_file<assignment_t> assignment_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A file of variable labels
  ////////////////////////////////////////////////////////////////////////////
  typedef simple_file<label_t> label_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief File(s) with 'meta' information
  ///
  /// \param T     Type of the file's content
  ///
  /// \param Files The number of files with type T
  ////////////////////////////////////////////////////////////////////////////
  template<typename T, size_t Files>
  using meta_file = __shared_file<__meta_file<T,Files>>;

  //////////////////////////////////////////////////////////////////////////////
  /// An unreduced Decision Diagram is given by a three files of arcs:
  ///
  /// - [0] : node-to-node arcs (sorted by <tt>target</tt>)
  /// - [1] : node-to-sink arcs (sorted by <tt>source</tt>).
  /// - [2] : node-to-sink arcs (not sorted)
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t ARC_FILE_COUNT = 3u;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Files of arcs to represent an unreduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  typedef meta_file<arc_t, ARC_FILE_COUNT> arc_file;


  //////////////////////////////////////////////////////////////////////////////
  /// A reduced Decision Diagram is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t NODE_FILE_COUNT = 1u;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief File of nodes to represent a reduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  class node_file : public meta_file<node_t, NODE_FILE_COUNT>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors

    // Temporary node_file constructor
    node_file() = default;

    // TODO: Persistent files with a filename
    // node_file(const std::string &filename) : __shared_file(filename) { }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the meta file
    //////////////////////////////////////////////////////////////////////////////
    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Check whether a given node_file is sink-only and satisfies the
  ///             given sink_pred.
  ///
  /// \param file The node_file to check its content
  ///
  /// \param pred If the given node_file only contains a sink node, then
  ///             secondly the sink is checked with the given sink predicate.
  ///             Default is any type sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const node_file &file, const sink_pred &pred = is_any);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The minimal label, i.e. the label of the root.
  //////////////////////////////////////////////////////////////////////////////
  label_t min_label(const node_file &file);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal label, i.e. the label of the deepest node.
  //////////////////////////////////////////////////////////////////////////////
  label_t max_label(const node_file &file);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t nodecount(const node_file &nodes);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t nodecount(const arc_file &arcs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of levels (i.e. number of unique labels) in the DAG.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t varcount(const node_file &nodes);
}

#endif // ADIAR_FILE_H
