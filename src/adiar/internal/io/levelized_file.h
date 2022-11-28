#ifndef ADIAR_INTERNAL_IO_META_FILE_H
#define ADIAR_INTERNAL_IO_META_FILE_H

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/level_info.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief       File(s) with meta information.
  ///
  /// \param elem_type Type of the file's content.
  ///
  /// \details The entities of Adiar that represents DAGs, which are represented
  ///          by one or more files of type <tt>T</tt>. To optimise several
  ///          algorithms, these files also carry around 'meta' information.
  ///          This information is stored in the variables and in a levelized
  ///          fashion depending on the granularity of the information.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  class __levelized_file : public FILE_CONSTANTS<elem_type>::stats
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file's elements.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

  private:
    static constexpr size_t FILES = FILE_CONSTANTS<elem_t>::files;
    static_assert(0 < FILES, "The number of files must be positive");

  public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The number of false and true terminals in the file.
    ///        Index 0 gives the number of false terminals and index 1 gives the
    ///        number of true terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals[2] = { 0, 0 };

  public: // TODO: privatize
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Meta information on a level by level granularity.
    ////////////////////////////////////////////////////////////////////////////
    file<level_info> _level_info_file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Files describing the directed acyclic graph.
    ////////////////////////////////////////////////////////////////////////////
    file<elem_t> _files [FILES];

  public:
    __levelized_file()
    { }

  public:
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
    size_t levels() const
    {
      return _level_info_file.size();
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief File(s) with 'meta' information
  ///
  /// \param T     Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename elem_type>
  using levelized_file = shared_file<__levelized_file<elem_type>>;
}

#endif // ADIAR_INTERNAL_IO_META_FILE_H
