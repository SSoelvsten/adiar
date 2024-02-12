#ifndef ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H
#define ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H

#include <string>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Provides shared ownership of a single file. This also includes
  ///        (thread-safe) reference counting and automatic garbage collection.
  ///
  /// \details This is a wrapper on the `adiar::shared_ptr` (which in itself is
  ///          just a wrapper on `std::shared_ptr`) to slightly change its
  ///          semantics:
  ///          - Default constructor creates a new fresh file rather than being
  ///            null (TODO: change?)
  ///          - If `const` then not only can the pointer not be moved, but the
  ///            object underneath is `const` too, i.e. read-only.
  ///
  /// \tparam File Type of the underlying file
  //////////////////////////////////////////////////////////////////////////////
  template <typename File>
  class shared_file_ptr : public shared_ptr<File>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object.
    ////////////////////////////////////////////////////////////////////////////
    using file_type = File;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    using value_type = typename file_type::value_type;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor with a new fresh temporary file.
    ///
    /// \internal This constructor is part of a larger invariant: "A shared file
    ///           always is in a 'valid' state". That is, an algorithm right
    ///           chooses to crash/throw rather than return an error value that
    ///           may propagate throughout computation (similar to how CUDD
    ///           does). If this should be changed, then all internal functions
    ///           should be changed to use the <tt>make_shared_file</tt> and
    ///           <tt>make_shared_levelized_file</tt> instead.
    ///
    /// \see make_shared_file make_shared_levelized_file
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr()
      : shared_ptr<File>(adiar::make_shared<File>())
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor to reload a persisted file.
    ///
    /// \see make_shared_file make_shared_levelized_file
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const std::string p)
      : shared_ptr<File>(adiar::make_shared<File>(p))
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion-constructor from raw `shared_ptr<File>`.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const shared_ptr<File>& other)
      : shared_ptr<File>(other)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy-constructor.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const shared_file_ptr<File>& other) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move-constructor.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(shared_file_ptr<File>&& other) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr<File>&
    operator=(const shared_file_ptr<File>& o) = default;

    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr<File>&
    operator=(shared_file_ptr<File>&& o) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the raw pointer (read-only).
    ////////////////////////////////////////////////////////////////////////////
    const File*
    get() const
    {
      return shared_ptr<File>::get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the raw pointer.
    ////////////////////////////////////////////////////////////////////////////
    File*
    get()
    {
      return shared_ptr<File>::get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Dereference the pointer to obtain the file (read-only).
    ////////////////////////////////////////////////////////////////////////////
    const File&
    operator*() const
    {
      return *get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Dereference the pointer to obtain the file.
    ////////////////////////////////////////////////////////////////////////////
    File&
    operator*()
    {
      return *get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Member access (read-only) to the file.
    ////////////////////////////////////////////////////////////////////////////
    const File*
    operator->() const
    {
      return get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Member access for the underlying file.
    ////////////////////////////////////////////////////////////////////////////
    File*
    operator->()
    {
      return get();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy of a shared file.
    ///
    /// \remark This new file is a temporary file and must be marked persisted
    ///         to be kept existing beyond the last reference to it is gone.
    ////////////////////////////////////////////////////////////////////////////
    static shared_file_ptr<File>
    copy(const shared_file_ptr<File>& f)
    {
      return make_shared<File>(File::copy(*f));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared ownership of a file containing elements of the given type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type>
  using shared_file = shared_file_ptr<file<value_type>>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new (temporary) file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type>
  inline shared_file<value_type>
  make_shared_file()
  {
    return adiar::make_shared<file<value_type>>();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Open a persisted file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type>
  inline shared_file<value_type>
  make_shared_file(const std::string& p)
  {
    return adiar::make_shared<file<value_type>>(p);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared ownership of a levelized file.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type, bool split_on_levels = false>
  using shared_levelized_file = shared_file_ptr<levelized_file<value_type, split_on_levels>>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new (temporary) levelized file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type, bool split_on_levels = false>
  inline shared_levelized_file<value_type, split_on_levels>
  make_shared_levelized_file()
  {
    return adiar::make_shared<levelized_file<value_type, split_on_levels>>();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Open a persisted file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_type, bool split_on_levels = false>
  inline shared_levelized_file<value_type, split_on_levels>
  make_shared_levelized_file(const std::string& p)
  {
    return adiar::make_shared<levelized_file<value_type, split_on_levels>>(p);
  }
}

#endif // ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H
