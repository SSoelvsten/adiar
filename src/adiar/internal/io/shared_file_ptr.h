#ifndef ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H
#define ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H

#include <string>

#include <adiar/internal/memory.h>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/levelized_file.h>

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
  /// \param file_type The type of the underlying file
  //////////////////////////////////////////////////////////////////////////////
  template <typename file_type>
  class shared_file_ptr : public shared_ptr<file_type>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object.
    ////////////////////////////////////////////////////////////////////////////
    typedef file_type file_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename file_t::elem_t elem_t;

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
    /// \sa make_shared_file make_shared_levelized_file
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr()
      : shared_ptr<file_t>(adiar::make_shared<file_t>())
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor to reload a persisted file.
    ///
    /// \sa make_shared_file make_shared_levelized_file
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const std::string p)
      : shared_ptr<file_t>(adiar::make_shared<file_t>(p))
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion-constructor from raw `shared_ptr<file_t>`.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const shared_ptr<file_t> &other) : shared_ptr<file_t>(other)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy-constructor.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(const shared_file_ptr<file_t> &other) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move-constructor.
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr(shared_file_ptr<file_t> &&other) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr<file_t>& operator= (const shared_file_ptr<file_t> &o) = default;

    ////////////////////////////////////////////////////////////////////////////
    shared_file_ptr<file_t>& operator= (shared_file_ptr<file_t> &&o) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the raw pointer (read-only).
    ////////////////////////////////////////////////////////////////////////////
    const file_t* get() const
    { return shared_ptr<file_t>::get(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the raw pointer.
    ////////////////////////////////////////////////////////////////////////////
    file_t* get()
    { return shared_ptr<file_t>::get(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Dereference the pointer to obtain the file (read-only).
    ////////////////////////////////////////////////////////////////////////////
    const file_t& operator*() const
    { return *get(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Dereference the pointer to obtain the file.
    ////////////////////////////////////////////////////////////////////////////
    file_t& operator*()
    { return *get(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Member access (read-only) to the file.
    ////////////////////////////////////////////////////////////////////////////
    const file_t* operator->() const
    { return get(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Member access for the underlying file.
    ////////////////////////////////////////////////////////////////////////////
    file_t* operator->()
    { return get(); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy of a shared file.
    ///
    /// \remark This new file is a temporary file and must be marked persisted
    ///         to be kept existing beyond the last reference to it is gone.
    ////////////////////////////////////////////////////////////////////////////
    static shared_file_ptr<file_t> copy(const shared_file_ptr<file_t> &f)
    {
      return make_shared<file_t>(file_t::copy(*f));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared ownership of a file containing elements of the given type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  using shared_file = shared_file_ptr<file<elem_t>>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new (temporary) file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  inline shared_file<elem_t> make_shared_file() {
    return adiar::make_shared<file<elem_t>>();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Open a persisted file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  inline shared_file<elem_t> make_shared_file(const std::string &p) {
    return adiar::make_shared<file<elem_t>>(p);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared ownership of a levelized file.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool split_on_levels = false>
  using shared_levelized_file = shared_file_ptr<levelized_file<elem_t, split_on_levels>>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new (temporary) levelized file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool split_on_levels = false>
  inline shared_levelized_file<elem_t, split_on_levels>
  make_shared_levelized_file() {
    return adiar::make_shared<levelized_file<elem_t, split_on_levels>>();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Open a persisted file with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool split_on_levels = false>
  inline shared_levelized_file<elem_t, split_on_levels>
  make_shared_levelized_file(const std::string &p) {
    return adiar::make_shared<levelized_file<elem_t, split_on_levels>>(p);
  }
}

#endif // ADIAR_INTERNAL_IO_SHARED_FILE_PTR_H
