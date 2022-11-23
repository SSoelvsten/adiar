#ifndef ADIAR_INTERNAL_IO_SIMPLE_FILE_H
#define ADIAR_INTERNAL_IO_SIMPLE_FILE_H

#include <adiar/assignment.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/io/file.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// The <tt>file</tt> and <tt>__meta_file</tt> classes are hidden behind a
  /// shared pointer, such that we can parse it around. That is, all actual
  /// files we are going to deal with are a <tt>__shared_file<x_file<T>></tt>.
  ///
  /// \param T Type of the file's content
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  using simple_file = __shared_file<file<T>>;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A file of assignments (label, value)
  ////////////////////////////////////////////////////////////////////////////
  typedef simple_file<assignment_t> assignment_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A file of variable labels
  ////////////////////////////////////////////////////////////////////////////
  typedef simple_file<ptr_uint64::label_t> label_file;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Provides sorting for <tt>simple_file</tt>.
  ///
  /// TODO: Move to 'sorter.h'?
  ////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, typename pred_t = std::less<elem_t>>
  class simple_file_sorter
  {
  public:
    static void sort(simple_file<elem_t> f, pred_t pred = pred_t())
    {
      adiar_assert(!f.is_read_only(), "Cannot sort file after read-access");

      tpie::file_stream<elem_t> fs;
      fs.open(f->_tpie_file);

      tpie::progress_indicator_null pi;
      tpie::sort(fs, pred, pi);
    }
  };
}

#endif // ADIAR_INTERNAL_IO_SIMPLE_FILE_H
