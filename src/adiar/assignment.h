#ifndef ADIAR_ASSIGNMENT_H
#define ADIAR_ASSIGNMENT_H

#include <adiar/map.h>
#include <adiar/internal/dd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Possible values to assign a variable.
  //////////////////////////////////////////////////////////////////////////////
  enum class assignment : char
  {
    False = 0, // false
    True  = 1, // true
    None  = -1
  };
}

#endif // ADIAR_ASSIGNMENT_H
