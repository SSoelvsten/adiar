#ifndef ADIAR_EVALUATION_H
#define ADIAR_EVALUATION_H

#include <adiar/map.h>
#include <adiar/internal/dd.h>

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
  using evaluation = map_pair<internal::dd::label_t, evaluation_value>;
}

#endif // ADIAR_EVALUATION_H
