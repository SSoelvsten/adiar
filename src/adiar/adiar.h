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
#include <adiar/memory_mode.h>

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
  /// \brief Minimum value of 128 MiB for the memory limit.
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t MINIMUM_MEMORY = 128 * 1024 * 1024;

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
  ///
  /// \throws std::runtime_error    If `adiar_init()` has been called after
  ///                               `adiar_deinit()`.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_init(size_t memory_limit_bytes, std::string temp_dir = "");

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar is initialized.
  //////////////////////////////////////////////////////////////////////////////
  bool adiar_initialized() noexcept;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Closes and cleans up everything by Adiar
  ///
  /// \warning All of Adiar's file objects must be destructed **before** this
  ///          functions i called. That is, any \ref bdd \ref bdd_builder, \ref
  ///          zdd \ref zdd_builder or any \ref shared_file objects you may be
  ///          using.
  ///
  /// \throws std::runtime_error If compiled with *debug flag* and one of
  ///                            Adiar's objects have *not* been destructed.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_deinit();

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_H
