#ifndef ADIAR_MEMORY_MODE_H
#define ADIAR_MEMORY_MODE_H

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__adiar
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Memory Mode
  ///
  /// Adiar’s Algorithms delay recursion through use of auxiliary data
  /// structures. These auxiliary data structures can be optimised for
  /// internal memory, and so have a high performance on very small instances,
  /// or they can be designed for external memory such that they can handle
  /// decision diagrams much larger than the available memory.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar should exclusively use internal or external memory or
  ///        automatically pick either type based on size of input.
  ///
  /// \details
  ///   | Enum Value | Effect                                         |
  ///   |------------|------------------------------------------------|
  ///   | `Auto`     | Automatically decide on type of memory to use. |
  ///   | `Internal` | Always use *internal* memory.                  |
  ///   | `External` | Always use *external* memory.                  |
  //////////////////////////////////////////////////////////////////////////////
  enum class memory_mode_t { Auto, Internal, External };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The current memory mode (default: `Auto`).
  ///
  /// \details
  ///   If you want to force *Adiar* to a specific memory mode then you should set the
  ///   global variable `adiar::memory_mode` to one of the above three values. For
  ///   example, one can force *Adiar* always use internal memory with the
  ///   following piece of code.
  ///   ```cpp
  ///   adiar::memory_mode = adiar::memory_mode_t::Internal
  ///   ```
  ///
  /// \warning Using `Internal` may lead to crashes if an input or output is
  ///          too large!
  //////////////////////////////////////////////////////////////////////////////
  extern memory_mode_t memory_mode;

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_MEMORY_H
