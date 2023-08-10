#ifndef ADIAR_EXCEPTION_H
#define ADIAR_EXCEPTION_H

// Include exceptions from STD for aliasing
#include <exception>
#include <stdexcept>
#include <system_error>

namespace adiar
{
  // TODO: Design custom exceptions for Adiar.
  //
  // Notice that common use-cases are:
  //
  // - User has provided a list that does not contain a needed element/too
  //   short. This is quite similar to an `out_of_range` error.
  //
  // - User has provided a list that does not satisfy the required ordering.
  //   Right now, here we throw an `invalid_argument`.
  //
  // - User has provided a label-value that does not fit into the number of
  //   bits. Currently, we here use an `invalid_argument`
  //
  // - User has called a function that is not 'available' in the current state.
  //   Right now, we here use `domain_error`.
  //
  // - I/O errors from Adiar's files or from TPIE.
  //   Here we currently use `runtime_error` / `system_error`.

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Inputs that are invalid.
  ////////////////////////////////////////////////////////////////////////////
  using invalid_argument = std::invalid_argument;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Inputs for which an operation is undefined.
  ////////////////////////////////////////////////////////////////////////////
  using domain_error = std::domain_error;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Attempt to access elements outside of the given range.
  ////////////////////////////////////////////////////////////////////////////
  using out_of_range = std::out_of_range;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Errors beyond the scope of the program.
  ////////////////////////////////////////////////////////////////////////////
  using runtime_error = std::runtime_error;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief System runtime errors with an associated error code.
  ////////////////////////////////////////////////////////////////////////////
  using system_error = std::system_error;
}

#endif // ADIAR_EXCEPTION_H
