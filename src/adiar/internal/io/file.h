#ifndef ADIAR_INTERNAL_IO_FILE_H
#define ADIAR_INTERNAL_IO_FILE_H

#include <string.h>
#include <memory>
#include <limits>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/file.h>
#include <tpie/sort.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/data_types/level_info.h>

namespace adiar::internal
{
  constexpr tpie::access_type ADIAR_READ_ACCESS  = tpie::access_type::access_read;
  constexpr tpie::access_type ADIAR_WRITE_ACCESS = tpie::access_type::access_write;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Wrapper for TPIE's <tt>temp_file</tt>.
  ///
  /// \param T Type of the file's content
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class file
  {
    static_assert(std::is_pod<T>::value, "File content must be a POD");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef T elem_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    // The variables above are 'mutable' to allow them to be used with
    // 'non-const' operations (opening a stream) in a 'const' context.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file has been read (and hence should not be further)
    ////////////////////////////////////////////////////////////////////////////
    mutable bool _is_read_only = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The underlying TPIE file
    ////////////////////////////////////////////////////////////////////////////
    mutable tpie::temp_file _tpie_file;

    ////////////////////////////////////////////////////////////////////////////
    /// Befriend the few places that need direct access to these variables.
    template <typename elem_t, typename pred_t>
    friend class simple_file_sorter;

    template <typename elem_t, typename Comp>
    friend class simple_file_writer;

    template <typename elem_t>
    friend class levelized_file_writer;

    template <typename elem_t, bool REVERSE, typename SharedPtr_T>
    friend class file_stream;

  private:
    void touch_file()
    {
      // Opening the file with 'access_read_write' automatically creates the
      // file with header on disk.
      tpie::file_stream<elem_t> fs;
      fs.open(_tpie_file, tpie::access_type::access_read_write);
    }

  public:
    file() : _tpie_file() { touch_file(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Mark file to be read-only.
    ////////////////////////////////////////////////////////////////////////////
    void make_read_only() const
    {
      _is_read_only = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is currently in read-only mode, i.e. some
    /// adiar::file_stream has been attached to it and has marked it.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      return _is_read_only;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      tpie::file_stream<elem_t> fs;
      fs.open(_tpie_file, ADIAR_READ_ACCESS);
      return fs.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return size() == 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the file in bytes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(elem_t);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// We want to be able to construct files like the ones above and return them
  /// out of a function. For that, we cannot place them on the stack, but have
  /// to place them on the heap. Yet, placing things on the heap brings with it
  /// a whole new set of problems. Furthermore, the user may reuse the same
  /// result in multiple places.
  ///
  /// So, we use a `shared_ptr` to be able to:
  ///
  /// - Place the files on the heap, so the `__shared_file` can be returned with
  ///   a copy-constructor without breaking any of the `tpie::files`
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename T::elem_t elem_t;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The raw file underneath
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<T> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Member access for the underlying file, i.e. this is similar to
    ///        writing <tt>._file_ptr-></tt>.
    ////////////////////////////////////////////////////////////////////////////
    T* operator->() const
    {
      return _file_ptr.get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a new temporary shared file
    ////////////////////////////////////////////////////////////////////////////
    __shared_file() : _file_ptr(adiar::make_shared<T>())
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy-constructor.
    ////////////////////////////////////////////////////////////////////////////
    __shared_file(const __shared_file<T> &other) : _file_ptr(other._file_ptr)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move-constructor.
    /////////////////////////////////////////////////////////////////////////
    __shared_file(__shared_file<T> &&other) : _file_ptr(other._file_ptr)
    { }

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
}

#endif // ADIAR_INTERNAL_IO_FILE_H
