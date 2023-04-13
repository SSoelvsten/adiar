#ifndef ADIAR_ACCESS_MODE_H
#define ADIAR_ACCESS_MODE_H

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__adiar
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Access Mode
  ///
  /// Adiar’s Algorithms delay recursion through use of auxiliary data
  /// structures. These auxiliary data structures are redundant when the width
  /// of one or more inputs fits in the internal memory. In these cases it is
  /// much faster to use random access.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar should exclusively use random access (RA) or
  ///        priority queues (PQ) or automatically pick either way based on
  ///        size of input.
  ///
  /// \details
  ///   | Enum Value | Effect                                         |
  ///   |------------|------------------------------------------------|
  ///   | `AUTO`     | Automatically decide on type of access to use. |
  ///   | `RA`       | Always use *random access*.                    |
  ///   | `PQ`       | Always use *priority queues*.                  |
  //////////////////////////////////////////////////////////////////////////////
  enum class access_mode_t { AUTO, RA, PQ };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The current access mode (default: `AUTO`).
  ///
  /// \details
  ///   If you want to force *Adiar* to a specific access mode then you should
  ///   set the global variable `adiar::access_mode` to one of the above three
  ///   values. For example, one can force *Adiar* always use random access with
  ///   the following piece of code.
  ///   ```cpp
  ///   adiar::access_mode = adiar::access_mode_t::RA
  ///   ```
  ///
  /// \warning Using `RA` may lead to crashes if all inputs are too wide or
  ///          input to random access is not canonical!
  //////////////////////////////////////////////////////////////////////////////
  extern access_mode_t access_mode;

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_ACCESS_MODE_H
