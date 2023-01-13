#ifndef ADIAR_ASSIGNMENT_H
#define ADIAR_ASSIGNMENT_H

#include <adiar/internal/data_types/var_map.h>

namespace adiar
{
  enum class assignment_value : char {
    FALSE     = 0, // false
    TRUE      = 1, // true
    NONE      = -1
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A <tt>(x,v)</tt> tuple representing the single assignment
  ///        \f$ x \mapsto v \f$.
  //////////////////////////////////////////////////////////////////////////////
  using assignment = internal::var_mapping<assignment_value>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Assignment function which provides for each variable label its
  ///        assigned value.
  //////////////////////////////////////////////////////////////////////////////
  using assignment_func = internal::var_func_map<assignment_value>;
}

#endif // ADIAR_ASSIGNMENT_H
