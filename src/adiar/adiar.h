#ifndef ADIAR_H
#define ADIAR_H

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__adiar Adiar Kernel
///
/// \brief Set up of Adiar and its core settings.
////////////////////////////////////////////////////////////////////////////////

#include <string>

////////////////////////////////////////////////////////////////////////////////
/// Core
#include <adiar/assignment.h>
#include <adiar/builder.h>
#include <adiar/domain.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

////////////////////////////////////////////////////////////////////////////////
/// Decision Diagrams
#include <adiar/bdd.h>
#include <adiar/zdd.h>

////////////////////////////////////////////////////////////////////////////////
/// Statistics
#include <adiar/statistics.h>

////////////////////////////////////////////////////////////////////////////////
/// Deprecated Functions
#include <adiar/deprecated.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__adiar
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Package Initialisation
  ///
  /// After having linked the C++ source file with Adiar, one needs to include
  /// the `<adiar/adiar.h>` header, initialise the library before using any of
  /// its data structures, and finally remember to deinitialise the library
  /// again before the program terminates.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initiates Adiar with the given amount of memory (given in bytes)
  ///
  /// \param memory_limit_bytes
  ///   The amount of internal memory (in bytes) that Adiar is allowed to use.
  ///   This has to be at least MINIMUM_BYTES.
  ///
  /// \param temp_dir
  ///   The directory in which to place all temporary files. Default on Linux is
  ///   the */tmp* library.
  ///
  /// \throws std::invalid_argument If `memory_limit_bytes` is set to a value
  ///                               less than the MINIMUM_BYTES required.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_init(size_t memory_limit_bytes, std::string temp_dir = "");

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar is initialized.
  //////////////////////////////////////////////////////////////////////////////
  bool adiar_initialized() noexcept;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Closes and cleans up everything by Adiar
  ///
  /// \warning This will invalidate any temporary files of Adiar, e.g. any
  ///          \ref bdd or \ref zdd objects you may be using.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_deinit();

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_H
