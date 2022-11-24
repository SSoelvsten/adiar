#ifndef ADIAR_INTERNAL_IO_SHARED_FILE_H
#define ADIAR_INTERNAL_IO_SHARED_FILE_H

#include <adiar/internal/memory.h>

namespace adiar::internal
{
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
  /// The <tt>file</tt> and <tt>__levelized_file</tt> classes are hidden behind a
  /// shared pointer, such that we can parse it around. That is, all actual
  /// files we are going to deal with are a <tt>__shared_file<x_file<T>></tt>.
  ///
  /// \param T Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  using simple_file = __shared_file<file<T>>;
}

#endif // ADIAR_INTERNAL_IO_SHARED_FILE_H
