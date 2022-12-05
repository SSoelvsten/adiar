#ifndef ADIAR_FILE_H
#define ADIAR_FILE_H

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  // Lifting 'adiar/internal/io/file_stream' to the 'adiar' namespace.

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::file_stream
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool REVERSE = false>
  using file_stream = internal::file_stream<elem_t, REVERSE>;

  // Lifting 'adiar/internal/io/file_writer' to the 'adiar' namespace.

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::file_writer
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  using file_writer = internal::file_writer<elem_t>;

  // Lifting 'adiar/internal/io/shared_file_ptr' to the 'adiar' namespace.

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::shared_file
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  using shared_file = internal::shared_file<elem_t>;

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::make_shared_file()
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  inline shared_file<elem_t> make_shared_file() {
    return internal::make_shared_file<elem_t>();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::make_shared_file(const std::string &p)
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t>
  inline shared_file<elem_t> make_shared_file(const std::string &p) {
    return internal::make_shared_file<elem_t>(p);
  }
}

#endif // ADIAR_FILE_H
