#ifndef ADIAR_INTERNAL_IO_FILE_H
#define ADIAR_INTERNAL_IO_FILE_H

#include <string>
#include <exception>
#include <filesystem>
#include <limits> // TODO <-- remove?

#include <tpie/tpie.h>
#include <tpie/file.h>

// TODO: move?
#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Provides compile-time known settings and meta information variables
  ///        used in `file<elem_type>` and `levelized_file<elem_type>`.
  ///
  /// \details Files also contain various pieces of meta information. Instead of
  ///          creating a larger object-oriented hierarchy, we provide this
  ///          `FILE_CONSTANTS` template to provide at compile-time the settings
  ///          and meta information to include.
  ///
  /// \param elem_type Element type in the file.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  struct FILE_CONSTANTS;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   A file on disk.
  ///
  /// \details A shallow wrapper TPIE's <tt>temp_file</tt> class to ensure the
  ///          file's content is part of the type and to provide auxiliary
  ///          functions.
  ///
  /// \param   elem_type Type of the file's content.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  class file // : public FILE_CONSTANTS<elem_type>::stats
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

    static_assert(std::is_pod<elem_t>::value, "File content must be a POD");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access flags to be used privately within reader's and writers.
    ////////////////////////////////////////////////////////////////////////////
    typedef tpie::access_type access_t;

    static constexpr access_t read_access  = tpie::access_type::access_read;
    static constexpr access_t write_access = tpie::access_type::access_write;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The underlying TPIE file
    ///
    /// \remark This variable is made 'mutable' to allow it to be used with
    //          'non-const' operations (opening a read-only stream) in a 'const'
    //          context.
    ////////////////////////////////////////////////////////////////////////////
    mutable tpie::temp_file _tpie_file;

    ////////////////////////////////////////////////////////////////////////////
    // TODO: atomic read-write counter?

    ////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to these variables.
    template <typename elem_t, typename pred_t>
    friend class simple_file_sorter;

    template <typename elem_t, typename Comp>
    friend class simple_file_writer;

    template <typename elem_t>
    friend class levelized_file_writer;

    template <typename elem_t, bool REVERSE>
    friend class file_stream;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a new unammed \em temporary file.
    ////////////////////////////////////////////////////////////////////////////
    file() : _tpie_file()
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a prior named \em persistent file.
    ///
    /// \throws std::runtime_error If the path does not point to an \em existing
    ///                            file on disk.
    ////////////////////////////////////////////////////////////////////////////
    file(const std::string &path) : _tpie_file(path, true)
    {
      if (!exists()) { throw std::runtime_error("'"+path+"' not found."); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this file is persistent or temporary, i.e. the file on
    ///        disk will \em not be removed when this object is destructed.
    ///
    /// \sa make_persistent
    ////////////////////////////////////////////////////////////////////////////
    bool is_persistent() const
    { return _tpie_file.is_persistent(); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make the file persistent: if the `file<>` object is destructed,
    ///        the physical file on disk will not be deleted.
    ///
    /// \sa is_persistent
    ////////////////////////////////////////////////////////////////////////////
    void make_persistent()
    {
      _tpie_file.set_persistent(true);
      if (!exists()) { touch(); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \copydoc is_persistent
    ////////////////////////////////////////////////////////////////////////////
    bool is_temp() const
    { return !is_persistent(); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the path for this file.
    ///
    /// \sa exists, move
    ////////////////////////////////////////////////////////////////////////////
    const std::string & path() const
    { return _tpie_file.path(); }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the path for this file.
    ////////////////////////////////////////////////////////////////////////////
    void set_path(const std::string &p)
    { _tpie_file.set_path(p); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file actually exists on disk.
    ///
    /// \sa path
    ////////////////////////////////////////////////////////////////////////////
    bool exists() const
    { return std::filesystem::exists(path()); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move file from one place to another.
    ///
    /// \param new_path Path to move the file to.
    ///
    /// \pre `is_persistentd() == false` (other's might depend on it) and
    ///      `new_path` names a yet non-existing file.
    ///
    /// \throws std::runtime_error If (a) `is_persisted() == true` and (b)
    ///                            `new_path` names an existing file.
    ////////////////////////////////////////////////////////////////////////////
    void move(const std::string &new_path)
    {
      // Disallow moving the file, if it is persisted
      if (is_persistent()) {
        throw std::runtime_error("'"+path()+"' is persisted.");
      }

      // Disallow moving the file on-top of another.
      if (std::filesystem::exists(new_path)) {
        throw std::runtime_error("'"+new_path+"' already exists.");
      }

      // Move the (existing?) file or at least change the path.
      if (exists()) {
        try { // Try to move it
          std::filesystem::rename(path(), new_path);
        } catch(std::filesystem::filesystem_error& e) {
          // Is the problem that happens
          const std::string err_msg = e.what();
          if (!err_msg.find("Invalid cross-device link")) throw e;

          // Try to copy-delete it.
          std::filesystem::copy(path(), new_path);
          std::filesystem::remove(path());
        }
      }
      set_path(new_path);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Creates the file on disk, if it does not yet exist.
    ////////////////////////////////////////////////////////////////////////////
    void touch()
    {
      if (exists()) return;

      // The file exists on disk, after opening it with write_access.
      tpie::file_stream<elem_t> fs;
      fs.open(_tpie_file, write_access);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      if (!exists()) { return 0u; }

      tpie::file_stream<elem_t> fs;
      fs.open(_tpie_file, read_access);
      return fs.size();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    { return size() == 0u; }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy of another file on disk.
    ///
    /// \remark This new file is a temporary file and must be marked persisted
    ///         to be kept existing beyond the object's lifetime.
    ////////////////////////////////////////////////////////////////////////////
    static file<elem_t> copy(const file<elem_t> &other)
    {
      if (!other.exists()) { return file<elem_t>(); }

      file<elem_t> ret;
      std::filesystem::copy(other.path(), ret.path());
      return ret;
    }
  };
}

#endif // ADIAR_INTERNAL_IO_FILE_H
