#ifndef ADIAR_INTERNAL_IO_META_FILE_H
#define ADIAR_INTERNAL_IO_META_FILE_H

#include <adiar/internal/io/file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// We also provide not-so simple files for some type of elements 'T'. These
  /// files include meta information and split the file content into one or more
  /// files. All of these constants are provided as \c static and \c constexpr
  /// or are mere type declarations.
  ///
  /// \param T Element type in the file
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  struct FILE_CONSTANTS;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       File(s) with meta information.
  ///
  /// \param T     Type of the file's content
  ///
  /// \details     The entities of Adiar that represents DAGs, which are
  ///              represented by one or more files of type <tt>T</tt>. To
  ///              optimise several algorithms, these files also carry around
  ///              'meta' information. This information is stored in the
  ///              variables and in a levelized fashion depending on the
  ///              granularity of the information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class __levelized_file : public FILE_CONSTANTS<T>::stats
  {
    static constexpr size_t FILES = FILE_CONSTANTS<T>::files;

    static_assert(0 < FILES, "The number of files must be positive");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef T elem_t;

  public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The number of false and true terminals in the file.
    ///        Index 0 gives the number of false terminals and index 1 gives the
    ///        number of true terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals[2] = { 0, 0 };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Meta information on a level by level granularity.
    ////////////////////////////////////////////////////////////////////////////
    file<level_info> _level_info_file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Files describing the directed acyclic graph.
    ////////////////////////////////////////////////////////////////////////////
    file<T> _files [FILES];

  public:
    __levelized_file() {
      adiar_debug(!is_read_only(), "Should be writable on creation");
    }
  public:
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
      for (size_t idx = 0u; idx < FILES; idx++) {
        _files[idx].make_read_only();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the file is read-only.
    ////////////////////////////////////////////////////////////////////////////
    bool is_read_only() const
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (!_files[idx].is_read_only()) {
          return false;
        };
      }
      return _level_info_file.is_read_only();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the file(s)
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t sum_size = 0u;
      for(size_t idx = 0; idx < FILES; idx++) {
        sum_size += _files[idx].size();
      }
      return sum_size;
    }

    bool empty() const
    {
      return size() == 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the levelized meta information file
    ////////////////////////////////////////////////////////////////////////////
    size_t meta_size() const
    {
      return _level_info_file.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The size of the file(s) in bytes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      return size() * sizeof(elem_t) + meta_size() * sizeof(level_info_t);
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief File(s) with 'meta' information
  ///
  /// \param T     Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  using levelized_file = __shared_file<__levelized_file<T>>;
}

#endif // ADIAR_INTERNAL_IO_META_FILE_H
