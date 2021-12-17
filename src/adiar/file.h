////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
// - foundational data structure and the union class.
#include <adiar/data.h>
// - assertions
#include <adiar/assert.h>

namespace adiar
{
#define ADIAR_READ_ACCESS tpie::access_type::access_read
#define ADIAR_WRITE_ACCESS tpie::access_type::access_write

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
    /// The number of elements in the file
    ////////////////////////////////////////////////////////////////////////////
    size_t size()
    {
      tpie::file_stream<T> fs;
      fs.open(__base_file, ADIAR_READ_ACCESS);
      return fs.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Same as size() == 0
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    {
      return size() == 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size()
    {
      return size() * sizeof(T);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// The core entities of ADIAR that represents DAGs also contain a 'meta' file,
  /// in which information is stored for the adiar::priority_queue to work
  /// optimally. These entities then also store one or more files of elements of
  /// type T.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, size_t Files>
  class __meta_file
  {
    static_assert(0 < Files, "The number of files must be positive");

  public:
    // Boolean flag primarily used for __meta_file<node_t,1> to recognise the
    // easy cases to check for isomorphism.
    bool canonical = false;

    file<level_info> _level_info_file;
    file<T> _files [Files];

    __meta_file() {
      adiar_debug(!is_read_only(), "Created read-only");
    }

    // TODO: Opening a persistent file with meta information given a path.
    // __meta_file(const std::string& filename) : ? { ? }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make file read-only, allowing multiple access via the
    /// `shared_access_file` variable in the _level_info_file or any of the _files.
    ///
    /// This assumes, that no `tpie::file_stream<T>` (that is, no
    /// `adiar::file_writer`) currently is attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only() const
    {
      _level_info_file.make_read_only();
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
      return _level_info_file.is_read_only();
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
    /// The number of elements in the level_info file
    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size()
    {
      return _level_info_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// The size of the file in bytes
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
  /// - It is thread-safe in the reference counting, so we now have ADIAR to be
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
    /// Notice, that while some of the functions below are not `const` for the
    /// pointed to underlying file, it is const with respect to this very
    /// object.
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

  template<typename T, typename sorting_pred_t = std::less<>>
  void sort(simple_file<T> f, sorting_pred_t pred = sorting_pred_t())
  {
    adiar_assert(!f.is_read_only(), "Cannot sort file after read-access");

    tpie::file_stream<T> fs;
    fs.open(f._file_ptr -> __base_file);

    tpie::progress_indicator_null pi;
    tpie::sort(fs, pred, pi);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// An unreduced Decision Diagram is given by a three files of arcs:
  ///
  /// - [0] : node-to-node arcs (in reverse topological order)
  /// - [1] : node-to-sink arcs (in topological order).
  /// - [2] : node-to-sink arcs (out-of order of the ones in [1])
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t ARC_FILE_COUNT = 3u;

  typedef meta_file<arc_t, ARC_FILE_COUNT> arc_file;


  //////////////////////////////////////////////////////////////////////////////
  /// A reduced Decision Diagram is given by a single sorted file by nodes.
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t NODE_FILE_COUNT = 1u;

  class node_file : public meta_file<node_t, NODE_FILE_COUNT>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors

    // Temporary node_file constructor
    node_file() : __shared_file() { }

    // TODO: Persistent files with a filename
    // node_file(const std::string &filename) : __shared_file(filename) { }

    // Copy constructor
    node_file(const node_file &o) : __shared_file(o) { }

    // Move constructor
    node_file(node_file &&o) : __shared_file(o) { }

    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size() const
    {
      return _file_ptr -> meta_size();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator
    // TODO: Are these not autogenerated?
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
  ///
  /// \param pred   If the given node_file only contains a sink node, then
  ///               secondly the sink is checked with the given sink predicate.
  ///               Default is any type sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const node_file &file, const sink_pred &pred = is_any);

  label_t min_label(const node_file &file);
  label_t max_label(const node_file &file);

  uint64_t nodecount(const node_file &nodes);
  uint64_t nodecount(const arc_file &arcs);

  uint64_t varcount(const node_file &nodes);
}

#endif // ADIAR_FILE_H
