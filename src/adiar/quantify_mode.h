#ifndef ADIAR_QUANTIFY_MODE_H
#define ADIAR_QUANTIFY_MODE_H

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__adiar
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Quantify Mode
  ///
  /// Adiar’s supports multiple approaches to compute the quantification of
  /// multiple variables.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Which strategy Adiar should use to quantify/project variables.
  ///
  /// \details
  ///   | Enum Value  | Effect                                                |
  ///   |-------------|-------------------------------------------------------|
  ///   | `Auto`      | Automatically decide on the approach to use.          |
  ///   | `Nested`    | Use the *nested sweeping* framework.                  |
  ///   | `Partial`   | Use repeated *partial quantification*.                |
  ///   | `Singleton` | Quantify each variable *independently* one at a time. |
  ///
  /// \remark Not all approaches can apply to each algorithm. If the desired
  ///         approach does not apply, then an approach that is *less* than it
  ///         is used instead, e.g. if `Partial` is picked but not possible then
  ///         `INDIVIDUAL` is used.
  //////////////////////////////////////////////////////////////////////////////
  enum class quantify_mode_t { Auto, Nested, Partial, Singleton };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The current quantification mode (default: `Auto`).
  ///
  /// \details
  ///   If you want to force *Adiar* to a specific quantify approach then you
  ///   should set the global variable `adiar::quantify_mode` to one of the
  ///   above four values. For example, one can force *Adiar* always use nested
  ///   sweeping with the following piece of code.
  ///   ```cpp
  ///   adiar::quantify_mode = adiar::quantify_mode_t::Nested
  ///   ```
  //////////////////////////////////////////////////////////////////////////////
  extern quantify_mode_t quantify_mode;

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_MEMORY_H
