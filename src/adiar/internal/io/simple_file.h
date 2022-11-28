#ifndef ADIAR_INTERNAL_IO_SIMPLE_FILE_H
#define ADIAR_INTERNAL_IO_SIMPLE_FILE_H

// TODO: remove this file.

#include <adiar/assignment.h>
#include <adiar/internal/data_types/ptr.h>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar::internal
{
  // TODO: remove this entire file...
  // should be replaced by 'shared_file<assignment_t>' and 'shared_file<label_t>'

  // Definition in 'shared_file'.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A file of assignments (label, value)
  //////////////////////////////////////////////////////////////////////////////
  typedef simple_file<assignment_t> assignment_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief File streams for assignments (label, value).
  ///
  /// \param REVERSE Whether the reading direction should be reversed
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  using assignment_stream = file_stream<assignment_t, REVERSE>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A file of variable labels
  //////////////////////////////////////////////////////////////////////////////
  typedef simple_file<ptr_uint64::label_t> label_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief File streams for variable labels.
  ///
  /// \param REVERSE Whether the reading direction should be reversed
  //////////////////////////////////////////////////////////////////////////////
  template<bool REVERSE = false>
  using label_stream = file_stream<ptr_uint64::label_t, REVERSE>;
}

#endif // ADIAR_INTERNAL_IO_SIMPLE_FILE_H
