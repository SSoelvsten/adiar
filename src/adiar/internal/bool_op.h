#ifndef ADIAR_INTERNAL_BOOL_OP_H
#define ADIAR_INTERNAL_BOOL_OP_H

#include <adiar/bool_op.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right,
  ///        i.e. `op(T,t) == op(F,t)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  can_right_shortcut(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(internal::ptr_uint64(false), terminal) == op(internal::ptr_uint64(true), terminal);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left,
  ///        i.e. `op(t, T) == op(t, F)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  can_left_shortcut(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == op(terminal, internal::ptr_uint64(true));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the right,
  ///        i.e. `op(X, t) == X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_irrelevant(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(internal::ptr_uint64(false), terminal) == internal::ptr_uint64(false)
      && op(internal::ptr_uint64(true), terminal) == internal::ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the left,
  ///        i.e. `op(t, X) == X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_irrelevant(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(false)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the right, i.e. `op(X, t) == ~X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_negating(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(true)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the left, i.e. `op(t, X) == ~X`
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_negating(const bool_op& op, const internal::ptr_uint64& terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(true)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, `op(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool
  is_commutative(const bool_op& op)
  {
    internal::ptr_uint64 terminal_T = internal::ptr_uint64(true);
    internal::ptr_uint64 terminal_F = internal::ptr_uint64(false);

    return op(terminal_T, terminal_F) == op(terminal_F, terminal_T);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Return the flipped operator, i.e. `op'(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool_op
  flip(const bool_op& op)
  {
    if (is_commutative(op)) return op;

    return [&op](const internal::ptr_uint64& a, const internal::ptr_uint64& b) { return op(b, a); };
  }
}

#endif // ADIAR_INTERNAL_BOOL_OP_H
