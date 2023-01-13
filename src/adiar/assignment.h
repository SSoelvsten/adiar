#ifndef ADIAR_ASSIGNMENT_H
#define ADIAR_ASSIGNMENT_H

#include <adiar/map.h>
#include <adiar/internal/dd.h>

namespace adiar
{
  enum class assignment : char
  {
    FALSE = 0, // false
    TRUE  = 1, // true
    NONE  = -1
  };
}

#endif // ADIAR_ASSIGNMENT_H
