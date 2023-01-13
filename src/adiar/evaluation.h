#ifndef ADIAR_EVALUATION_H
#define ADIAR_EVALUATION_H

namespace adiar
{
  enum class evaluation_value : char {
    FALSE     = 0, // false
    TRUE      = 1, // true
    DONT_CARE = 2
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A <tt>(x,v)</tt> tuple representing the variable \f$x\f$ evaluated
  ///        to the value \f$v\f$.
  //////////////////////////////////////////////////////////////////////////////
  using evaluation = internal::var_mapping<evaluation_value>;
}

#endif // ADIAR_EVALUATION_H
