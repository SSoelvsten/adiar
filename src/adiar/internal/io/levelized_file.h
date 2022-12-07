#ifndef ADIAR_INTERNAL_IO_META_FILE_H
#define ADIAR_INTERNAL_IO_META_FILE_H

#include <array>
#include <sstream>
#include <string>
#include <stdexcept>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>

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
  ///                        per level. Doing so provides support for even
  ///                        larger decision diagrams and a variable swapping
  ///                        operation.
  ///
  /// \details The entities of Adiar that represents DAGs are represented by one
  ///          or more files of type `elem_type`. To optimise several
  ///          algorithms, these files also carry around 'meta' information.
  ///          This information is stored in the variables and in a levelized
  ///          fashion depending on the granularity of the information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type, bool split_on_levels = false>
  class levelized_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Files that combined represent a DAG.
  ///
  /// \param elem_type       Type of the file's content.
  ////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  class levelized_file<elem_type, false> : public file_traits<elem_type>::stats
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of files to collectively represent a DAG.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t FILES = file_traits<elem_t>::files;
    static_assert(0 < FILES, "The number of files must be positive");

  private:
    ///////////////////////////////////////////////////////////////////////////
    inline void throw_if_bad_idx(const size_t idx) const
    {
      if (FILES <= idx)
        throw std::out_of_range("Index must be within interval [0;FILES-1]");
    }

    ///////////////////////////////////////////////////////////////////////////
    inline void throw_if_persistent() const
    {
      if (is_persistent())
        throw std::runtime_error("'"+_level_info_file.path()+"' is persisted.");
    }

  private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Derives the canonical path for one of the file.
    ///////////////////////////////////////////////////////////////////////////
    static std::string canonical_file_path(const std::string &path_prefix,
                                           const size_t idx)
    {
      std::stringstream ss;
      ss << path_prefix << ".file_" << idx;
      return ss.str();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Derives the canonical path for the level information file.
    ///////////////////////////////////////////////////////////////////////////
    static std::string canonical_levels_path(const std::string &path_prefix)
    {
      std::stringstream ss;
      ss << path_prefix << ".levels";
      return ss.str();
    }

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
    /// \brief Whether the file paths in '_level_info_file' and '_files' are
    ///        canonical, i.e. match the paths created by `canonical_file_path`
    ///        and `canonical_levels_path`.
    ////////////////////////////////////////////////////////////////////////////
    bool _canonical = false;

    ////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to these variables.
    template <typename tparam__elem_t, bool tparam__REVERSE>
    friend class levelized_file_stream;

    template <typename tparam__elem_t>
    friend class levelized_file_writer;

    template <bool tparam__REVERSE>
    friend class level_info_stream;

  public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a new unammed \em temporary file.
    ///////////////////////////////////////////////////////////////////////////
    levelized_file()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a prior named \em persisted file.
    ///
    /// \throws std::runtime_error If one or more files are missing in relation
    ///                            to the given path-prefix.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file(const std::string path_prefix)
    {
      // Set up '_files[idx]'
      for (size_t idx = 0; idx < FILES; idx++) {
        const std::string path = canonical_file_path(path_prefix, idx);
        if (std::filesystem::exists(path)) {
          _files[idx].set_path(path);
          _files[idx].make_persistent();
        } else { // '_files[idx]' is missing
          throw std::runtime_error("'"+path+"' does not exist");
        }
      }

      { // Set up '_level_info_file'
        const std::string path = canonical_levels_path(path_prefix);
        if (std::filesystem::exists(path)) {
          _level_info_file.set_path(path);
          _level_info_file.make_persistent();
        } else { // '_level_info_file' is missing
          throw std::runtime_error("'"+path+"' does not exist");
        }
      }

      _canonical = true;

      // TODO: load data from header
    }

  public:
    bool canonical_paths()
    { return _canonical; }

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
    /// \brief Make the file(s) persistent: if the `file<>` object is later
    ///        destructed, the physical file on disk will not be deleted.
    ///
    /// \pre   `canonical_paths() == true` (use `move()` to make them so).
    ///
    /// \throws std::runtime_error If `canonical_paths() == false`.
    ///
    /// \sa is_persistent, canonical_paths, move
    ////////////////////////////////////////////////////////////////////////////
    void make_persistent()
    {
      if (!canonical_paths()) {
        throw std::runtime_error("Cannot persist a file with non-canonical paths");
      }
      for (size_t idx = 0u; idx < FILES; idx++) {
        _files[idx].make_persistent();
      }
      _level_info_file.make_persistent();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Make the file(s) persistent at a given path: if the `file<>`
    ///        object is later destructed, the physical file on disk will not be
    ///        deleted.
    ///
    /// \pre   `can_move() == true`
    ///
    /// \throws std::runtime_error If `can_move() == false` or
    ///                            `move(path_prefix)` operation fails.
    ///
    /// \sa is_persistent, move
    ////////////////////////////////////////////////////////////////////////////
    void make_persistent(const std::string &path_prefix)
    {
      move(path_prefix);
      make_persistent();
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
    /// \brief The number of elements in a specific file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size(const size_t idx) const
    {
      throw_if_bad_idx(idx);
      return _files[idx].size();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the file(s).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t sum_size = 0u;
      for(size_t idx = 0u; idx < FILES; idx++)
        sum_size += size(idx);
      return sum_size;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the levelized meta information file
    ////////////////////////////////////////////////////////////////////////////
    size_t levels() const
    { return _level_info_file.size(); }

  public:
    //////////////////////////////////////////////////////////////////////////////
    /// \brief The first level to be encountered, i.e. the last level pushed.
    ///
    /// \pre `levels() > 0`
    //////////////////////////////////////////////////////////////////////////////
    size_t first_level() const
    {
      adiar_precondition(this->levels() > 0u);
      file_stream<level_info, true> fs(this->_level_info_file);
      return label_of(fs.pull());
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The last level to be encountered, i.e. the first level pushed.
    ///
    /// \pre `levels() > 0`
    //////////////////////////////////////////////////////////////////////////////
    size_t last_level() const
    {
      adiar_precondition(this->levels() > 0u);
      file_stream<level_info, false> fs(this->_level_info_file);
      return label_of(fs.pull());
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether all file(s) are empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    { return size() == 0u; }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Creates the file(s) on disk, if they do not yet already do.
    ////////////////////////////////////////////////////////////////////////////
    void __touch() const
    {
      if (exists()) return;

      for (size_t idx = 0u; idx < FILES; idx++) { _files[idx].__touch(); }
      _level_info_file.__touch();
    }

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
    /// \brief Whether this file can be moved.
    ////////////////////////////////////////////////////////////////////////////
    bool can_move()
    {
      const bool res = _level_info_file.can_move();
#ifndef NDEBUG
      for (size_t idx = 0; idx < FILES; idx++) {
        adiar_debug(_files[idx].can_move() == res,
                    "'can_move()' ought to be synchronised.");
      }
#endif
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
      throw_if_persistent();

      // Disallow moving the files on-top of an existing path-prefix.
      //
      // TODO: allow this? the path-prefix might be an independently created,
      //       but intendedly related, file made by the user.
      if (std::filesystem::exists(path_prefix))
        throw std::runtime_error("path-prefix '"+path_prefix+"' exists.");

      // Disallow moving a file on-top of another.
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

      // Set canonicity flag
      _canonical = true;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the content of one of the files in relation to a given
    ///        predicate.
    ///
    /// \pre `is_persistent() == false` and no stream nor writer is attached to
    ///      this file.
    ////////////////////////////////////////////////////////////////////////////
    template <typename pred_t = std::less<elem_t>>
    void sort(size_t idx, pred_t pred = pred_t())
    {
      throw_if_persistent();
      throw_if_bad_idx(idx);

      _files[idx].sort(pred);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy of a levelized file.
    ///
    /// \remark This new file is a temporary file and must be marked persisted
    ///         to be kept existing beyond the object's lifetime.
    ////////////////////////////////////////////////////////////////////////////
    static levelized_file<elem_t, false> copy(const levelized_file<elem_t> &lf)
    {
      levelized_file<elem_t, false> lf_copy;

      for (size_t idx = 0; idx < FILES; idx++)
        lf_copy._files[idx] = file<elem_t>::copy(lf._files[idx]);
      lf_copy._level_info_file = file<level_info_t>::copy(lf._level_info_file);

      return lf_copy;
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Files (split on each level) that combined represent a DAG.
  ///
  /// \param elem_type       Type of the file's content.
  ////////////////////////////////////////////////////////////////////////////
  //template <typename elem_type>
  //class levelized_file<elem_type, true> : public file_traits<elem_type>::stats
  //{
  //  TODO
  //}
}

#endif // ADIAR_INTERNAL_IO_META_FILE_H
