#ifndef ADIAR_INTERNAL_IO_META_FILE_H
#define ADIAR_INTERNAL_IO_META_FILE_H

#include <array>
#include <sstream>
#include <stdexcept>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/level_info.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief File representing a (levelized) DAG.
  ///
  /// \param elem_type       Type of the file's content.
  ///
  /// \param split_on_levels Whether each file type should be split in a file
  ///                        per level. This can be support even larger decision
  ///                        diagrams and the variable swapping operation.
  ///
  /// \details The entities of Adiar that represents DAGs are represented by one
  ///          or more files of type `elem_type`. To optimise several
  ///          algorithms, these files also carry around 'meta' information.
  ///          This information is stored in the variables and in a levelized
  ///          fashion depending on the granularity of the information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type, bool split_on_levels = false>
  class __levelized_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Files that combined represent a DAG.
  ///
  /// \param elem_type       Type of the file's content.
  ////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  class __levelized_file<elem_type, false> : public FILE_CONSTANTS<elem_type>::stats
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

  public:
    static constexpr size_t FILES = FILE_CONSTANTS<elem_t>::files;
    static_assert(0 < FILES, "The number of files must be positive");

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Meta information on a level by level granularity.
    ////////////////////////////////////////////////////////////////////////////
    file<level_info> _level_info_file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Files describing the directed acyclic graph.
    ////////////////////////////////////////////////////////////////////////////
    file<elem_t> _files [FILES];

    ////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to these variables.
    template <typename elem_t, size_t file_idx, bool REVERSE>
    friend class levelized_file_stream;

    template <typename elem_t>
    friend class levelized_file_writer;

    template <typename elem_t, bool REVERSE>
    friend class level_info_stream;

  public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The number of false and true terminals in the file.
    ///        Index 0 gives the number of false terminals and index 1 gives the
    ///        number of true terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals[2] = { 0, 0 };

  private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Derives the caninical path for one of the file.
    ///////////////////////////////////////////////////////////////////////////
    static std::string canonical_file_path(const std::string &path_prefix,
                                           const size_t idx)
    {
      std::stringstream ss;
      ss << path_prefix << ".file_" << idx;
      return ss.str();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Derives the caninical path for the level information file.
    ///////////////////////////////////////////////////////////////////////////
    static std::string canonical_levels_path(const std::string &path_prefix)
    {
      std::stringstream ss;
      ss << path_prefix << ".levels";
      return ss.str();
    }

  public:
    __levelized_file()
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file(s) are persistent or temporary, i.e. the file(s)
    ///        on disk will \em not be removed when this object is destructed.
    ///
    /// \sa make_persistent
    ////////////////////////////////////////////////////////////////////////////
    bool is_persistent() const
    {
      const bool res = _level_info_file.is_persistent();
#ifndef NDEBUG
      for (size_t idx = 0; idx < FILES; idx++) {
        adiar_debug(_files[idx].is_persistent() == res,
                    "Persistence ought to be synchronised.");
      }
#endif
      return res;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make the file(s) persistent: if the `file<>` object is
    ///        destructed, the physical file on disk will not be deleted.
    ///
    /// \sa is_persistent
    ////////////////////////////////////////////////////////////////////////////
    void make_persistent()
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _files[idx].make_persistent();
      }
      _level_info_file.make_persistent();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \copydoc is_persistent
    ////////////////////////////////////////////////////////////////////////////
    bool is_temp() const
    { return !is_persistent(); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file(s) actually exists on disk.
    ///
    /// \sa path
    ////////////////////////////////////////////////////////////////////////////
    bool exists() const
    {
      const bool res = std::filesystem::exists(_level_info_file.path());
#ifndef NDEBUG
      for (size_t idx = 0; idx < FILES; idx++) {
        adiar_debug(std::filesystem::exists(_files[idx].path()) == res,
                    "Persistence ought to be synchronised.");
      }
#endif
      return res;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the file(s).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t sum_size = 0u;
      for(size_t idx = 0u; idx < FILES; idx++)
        sum_size += _files[idx].size();
      return sum_size;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in a specific file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size(const size_t idx) const
    {
      if (FILES <= idx) {
        throw std::out_of_range("Index must be within interval [0;FILES-1]");
      }
      return _files[idx].size();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the levelized meta information file
    ////////////////////////////////////////////////////////////////////////////
    size_t levels() const
    { return _level_info_file.size(); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether all file(s) are empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    { return size() == 0u; }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Creates the file(s) on disk, if they do not yet already do.
    ////////////////////////////////////////////////////////////////////////////
    void touch()
    {
      if (exists()) return;

      for (size_t idx = 0u; idx < FILES; idx++) { _files[idx].touch(); }
      _level_info_file.touch();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the paths for all files comprising this levelized file.
    ///
    /// \sa exists, move
    ////////////////////////////////////////////////////////////////////////////
    std::array<std::string, FILES+1> paths() const
    {
      std::array<std::string, FILES+1> res;
      for (size_t idx = 0u; idx < FILES; idx++) {
        res[idx] = _files[idx].path();
      }
      res[FILES] = _level_info_file.path();
      return res;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move file from one place to another.
    ///
    /// \param path_prefix Prefix of the path used for the underlying files.
    ///
    /// \pre `is_persistent() == false` (other's might depend on it) and
    ///      `path_prefix` names a yet non-existing file. Neither should a
    ///      `file_stream` nor a `file_writer` be hooked into this file.
    ///
    /// \throws std::runtime_error Preconditions are violated.
    ////////////////////////////////////////////////////////////////////////////
    void move(const std::string &path_prefix)
    {
      // Disallow moving the file, if it is persisted
      if (is_persistent()) {
        throw std::runtime_error("'"+_level_info_file.path()+"' is persisted.");
      }

      // Disallow moving the file on-top of another.
      {
        std::string path = canonical_levels_path(path_prefix);
        if (std::filesystem::exists(path))
          throw std::runtime_error("'"+path+"' already exists.");
      }
      for (size_t idx = 0; idx < FILES; idx++) {
        std::string path = canonical_file_path(path_prefix, idx);
        if (std::filesystem::exists(path))
          throw std::runtime_error("'"+path+"' already exists.");
      }

      // Move files
      {
        std::string path = canonical_levels_path(path_prefix);
        _level_info_file.move(path);
      }
      for (size_t idx = 0; idx < FILES; idx++) {
        std::string path = canonical_file_path(path_prefix, idx);
        _files[idx].move(path);
      }
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Files (split on each level) that combined represent a DAG.
  ///
  /// \param elem_type       Type of the file's content.
  ////////////////////////////////////////////////////////////////////////////
  //template <typename elem_type>
  //class __levelized_file<elem_type, true> : public FILE_CONSTANTS<elem_type>::stats
  //{
  //  TODO
  //}

  ////////////////////////////////////////////////////////////////////////////
  /// \brief File(s) with 'meta' information
  ///
  /// \param T     Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename elem_type>
  using levelized_file = shared_file<__levelized_file<elem_type>>;
}

#endif // ADIAR_INTERNAL_IO_META_FILE_H
