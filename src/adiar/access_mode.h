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
  /// \brief Whether Adiar should exclusively use random access
  ///        (`Random_Access`) or priority queues (`Priority_Queue`) or
  ///        automatically pick either way based on the width of inputs.
  ///
  /// \details
  ///   | Enum Value       | Effect                                         |
  ///   |------------------|------------------------------------------------|
  ///   | `Auto`           | Automatically decide on type of access to use. |
  ///   | `Random_Access`  | Always use *random access*.                    |
  ///   | `Priority_Queue` | Always use *priority queues*.                  |
  //////////////////////////////////////////////////////////////////////////////
  enum class access_mode_t { Auto, Random_Access, Priority_Queue };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The current access mode (default: `Auto`).
  ///
  /// \details
  ///   If you want to force *Adiar* to a specific access mode then you should
  ///   set the global variable `adiar::access_mode` to one of the above three
  ///   values. For example, one can force *Adiar* always use random access with
  ///   the following piece of code.
  ///   ```cpp
  ///   adiar::access_mode = adiar::access_mode_t::Random_Access
  ///   ```
  ///
  /// \warning Using `Random_Access` may lead to crashes if all inputs are too
  ///          wide or input to random access is not canonical!
  //////////////////////////////////////////////////////////////////////////////
  extern access_mode_t access_mode;

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_ACCESS_MODE_H
