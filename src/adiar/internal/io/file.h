#ifndef ADIAR_INTERNAL_IO_FILE_H
#define ADIAR_INTERNAL_IO_FILE_H

#include <filesystem>
#include <limits> // TODO <-- remove?
#include <string>

#include <adiar/exception.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>
#include <tpie/tempname.h>
#include <tpie/tpie.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Provides compile-time known settings and meta information variables used in
  ///          `file<elem_type>` and `levelized_file<elem_type>`.
  ///
  /// \details Files also contain various pieces of meta information. Instead of creating a larger
  ///          object-oriented hierarchy, we provide this `file_traits` template to provide at
  ///          compile-time the settings and meta information to include.
  ///
  /// \tparam T
  ///    Element type in the file.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  struct file_traits;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     A file on disk.
  ///
  /// \details A shallow wrapper TPIE's <tt>temp_file</tt> class to ensure the file's content is
  ///            part of the type and to provide auxiliary functions.
  ///
  /// \tparam  T
  ///    Type of the file's content.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T> // <-- Header
  class file
  // : public file_traits<T>::stats
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using value_type = T;

    static_assert(std::is_pod<value_type>::value, "File content must be a POD");

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Access flags to be used privately within reader's and writers.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using access_type = tpie::access_type;

    static constexpr access_type r_access  = tpie::access_type::access_read;
    static constexpr access_type w_access  = tpie::access_type::access_write;
    static constexpr access_type rw_access = tpie::access_type::access_read_write;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  The underlying TPIE file
    ///
    /// \remark This variable is made 'mutable' to allow it to be used with 'non-const' operations
    //          (opening a read-only stream) in a 'const' context.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    mutable tpie::temp_file _tpie_file;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: atomic read-write counter?

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to these variables.
    template <typename value_type, bool REVERSE>
    friend class ifstream;

    template <typename value_type>
    friend class ofstream;

    template <typename value_type>
    friend class iofstream;

    template <typename value_type, bool split_on_levels>
    friend class levelized_file;

    // Remove?
    template <typename value_type>
    friend class levelized_ofstream;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a new unammed \em temporary file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    file()
      : _tpie_file()
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a prior named \em persisted file.
    ///
    /// \throws runtime_error If the path does not point to an \em existing file on disk.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    file(const std::string& path)
      : _tpie_file(path, true)
    {
      if (!exists()) { throw runtime_error("'" + path + "' not found."); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this file is persistent or temporary, i.e. the file on disk will \em not be
    ///        removed when this object is destructed.
    ///
    /// \see make_persistent
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_persistent() const
    {
      return _tpie_file.is_persistent();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Make the file persistent: if the `file<>` object is destructed, the physical file on
    ///        disk will not be deleted.
    ///
    /// \see is_persistent
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    make_persistent()
    {
      _tpie_file.set_persistent(true);
      if (!exists()) { touch(); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \copydoc is_persistent
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_temp() const
    {
      return !is_persistent();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file actually exists on disk.
    ///
    /// \see path
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    exists() const
    {
      return std::filesystem::exists(path());
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size() const
    {
      if (!exists()) { return 0u; }

      tpie::file_stream<value_type> fs;
      fs.open(_tpie_file, r_access);
      return fs.size();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty() const
    {
      return size() == 0u;
    }

  private:
    void
    __touch() const
    {
      if (exists()) return;

      // The file exists on disk, after opening it with write access.
      tpie::file_stream<value_type> fs;
      fs.open(_tpie_file, w_access);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Creates the file on disk, if it does not yet exist.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    touch()
    {
      __touch();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the size of the file header.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // static size_t header_size() constexpr
    //{ TODO }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Set the contents of the file header.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // void set_header(const std::string &p)
    //{ TODO }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the contents of the file header.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // header_t get_header() const
    //{ TODO }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the path for this file.
    ///
    /// \see exists move
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const std::string&
    path() const
    {
      return _tpie_file.path();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Set the path for this file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    set_path(const std::string& p)
    {
      _tpie_file.set_path(p);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this file can be moved.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_move()
    {
      return is_temp();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move file from one place to another.
    ///
    /// \param new_path Path to move the file to.
    ///
    /// \pre `can_move()`and `new_path` names a yet non-existing file. Neither should any
    ///      `ifstream`s or `ofstream`s be hooked into this file.
    ///
    /// \throws runtime_error Preconditions are violated
    ///
    /// \throws runtime_error Moving the file is unsuccessful.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    move(const std::string& new_path)
    {
      // Disallow moving the file, if it is persisted
      if (is_persistent()) { throw runtime_error("'" + path() + "' is persisted."); }

      // Disallow moving the file on-top of another.
      if (std::filesystem::exists(new_path)) {
        throw runtime_error("'" + new_path + "' already exists.");
      }

      // Move the file on disk, if it exists.
      if (exists()) {
        try { // Try to move it in O(1) time.
          std::filesystem::rename(path(), new_path);
        } catch (std::filesystem::filesystem_error& e1) {
#ifndef NDEBUG
          std::cerr << "Adiar: unable to move file<value_type> in O(1) time" << std::endl
                    << "       what(): " << e1.what() << std::endl;
#endif
          // Did the file disappear and everything just is in shambles?
          if (!std::filesystem::exists(path())) { throw static_cast<system_error>(e1); }

          try {
            // Most likely, this catch-case is an "Invalid cross-device link": try to copy-delete it
            // instead in O(N) time.
            std::filesystem::copy(path(), new_path);
            std::filesystem::remove(path());
          } catch (std::filesystem::filesystem_error& e2) {
#ifndef NDEBUG
            std::cerr << "Adiar: unable to copy-delete file<value_type> in O(N) time" << std::endl
                      << "       what(): " << e2.what() << std::endl;
#endif
            throw static_cast<system_error>(e2);
          }
        }
      }

      // Set the path on the TPIE object (which does not attempt to delete the old path).
      set_path(new_path);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the content of this file in relation to a given predicate.
    ///
    /// \pre `is_persistent() == false` and `ifstream` nor `ofstream` is attached to this
    ///      file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename pred_t = std::less<value_type>>
    void
    sort(pred_t pred = pred_t())
    {
      // Disallow sorting a persistent file
      if (is_persistent()) throw runtime_error("'" + path() + "' is persisted.");

      // If empty, just skip all the work
      if (size() == 0u) return;

      // Use TPIE's file sorting.
      tpie::file_stream<value_type> fs;
      fs.open(_tpie_file);

      tpie::progress_indicator_null pi;
      tpie::sort(fs, pred, pi);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy of a file.
    ///
    /// \remark This new file is a temporary file and must be marked persisted to be kept existing
    ///         beyond the object's lifetime.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static file<value_type>
    copy(const file<value_type>& f)
    {
      if (!f.exists()) { return file<value_type>(); }

      file<value_type> ret;
      std::filesystem::copy(f.path(), ret.path());
      return ret;
    }
  };
}

#endif // ADIAR_INTERNAL_IO_FILE_H
