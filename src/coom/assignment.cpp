#ifndef COOM_ASSIGNMENT_CPP
#define COOM_ASSIGNMENT_CPP

#include "assignment.h"

namespace coom
{
  assignment_t create_assignment(label_t label, bool value)
  {
#if COOM_ASSERT
    assert (label <= MAX_LABEL);
#endif
    return {label, value};
  }

  bool operator< (const assignment& a, const assignment& b)
  {
    return a.label < b.label || (a.label == b.label && a.value < b.value);
  }

  bool operator> (const assignment& a, const assignment& b)
  {
    return a.label > b.label || (a.label == b.label && a.value > b.value);
  }

  bool operator== (const assignment& a, const assignment& b)
  {
    return a.label == b.label && a.value == b.value;
  }

  bool operator!= (const assignment& a, const assignment& b)
  {
    return !(a==b);
  }
}

#endif // COOM_ASSIGNMENT_CPP
