#ifndef ADIAR_INTERNAL_BOOL_OP_H
#define ADIAR_INTERNAL_BOOL_OP_H

#include <adiar/bool_op.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right, i.e. `op(T,t) == op(F,t)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  can_right_shortcut(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == op(true, terminal);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left, i.e. `op(t, T) == op(t, F)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  can_left_shortcut(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == op(terminal, true);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal value is idempotent from the right with respect to an operator,
  ///        i.e. `op(X, t) == X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_idempotent(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == false && op(true, terminal) == true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal value is idempotent from the left with respect to an operator,
  ///        i.e. `op(t, X) == X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_idempotent(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == false && op(terminal, true) == true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other from the right,
  ///        i.e. `op(X, t) == ~X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_negating(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == true && op(true, terminal) == false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other from the left,
  ///        i.e. `op(t, X) == ~X`
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_negating(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == true && op(terminal, true) == false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, `op(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_commutative(const predicate<bool, bool>& op)
  {
    return op(true, false) == op(false, true);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Return the flipped operator, i.e. `op'(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline predicate<bool, bool>
  flip(const predicate<bool, bool>& op)
  {
    if (is_commutative(op)) return op;
    return [&op](const bool a, const bool b) -> bool { return op(b, a); };
  }
}

#endif // ADIAR_INTERNAL_BOOL_OP_H
