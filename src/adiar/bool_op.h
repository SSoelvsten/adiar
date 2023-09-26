#ifndef ADIAR_BOOL_OP_H
#define ADIAR_BOOL_OP_H

#include <adiar/functional.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Computes the unique identifier of the resulting terminal based on
  ///          the pointers to two terminals.
  ///
  /// \details By abusing our knowledge of the bit-layout, we can implement all
  ///          common operations merely as a few fast bit operations.
  ///
  /// \remark  For each operator, we provide the truth table
  ///          [(1,1), (1,0), (0,1), (0,0)].
  //////////////////////////////////////////////////////////////////////////////
  using bool_op = function<internal::uid_uint64(const internal::ptr_uint64&, const internal::ptr_uint64&)>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'and' operator, i.e. the truth table: [1,0,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op and_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return essential(a & b);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not and' operator, i.e. the truth table: [0,1,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nand_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return negate(and_op(a, b));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'or' operator, i.e. the truth table: [1,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op or_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return essential(a | b);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not or' operator, i.e. the truth table: [0,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nor_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return negate(or_op(a, b));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [0,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xor_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return essential(a ^ b);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [1,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xnor_op  = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return negate(xor_op(a, b));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,0,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op imp_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::uid_uint64
  {
    return internal::ptr_uint64(essential(a) <= essential(b));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,1,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op invimp_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::ptr_uint64
  {
    return internal::ptr_uint64(essential(b) <= essential(a));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'equivalence' operator, i.e. the 'xnor' operator
  //////////////////////////////////////////////////////////////////////////////
  const bool_op equiv_op = xnor_op;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'set difference' operator, i.e. the truth table [0,1,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op diff_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::ptr_uint64
  {
    return and_op(a, negate(b));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less' operator, i.e. the truth table [0,0,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op less_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::ptr_uint64
  {
    return and_op(negate(a), b);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right,
  ///        i.e. `op(T,t) == op(F,t)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_right_shortcut(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(internal::ptr_uint64(false), terminal) == op(internal::ptr_uint64(true), terminal);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left,
  ///        i.e. `op(t, T) == op(t, F)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_left_shortcut(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == op(terminal, internal::ptr_uint64(true));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the right,
  ///        i.e. `op(X, t) == X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_irrelevant(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(internal::ptr_uint64(false), terminal) == internal::ptr_uint64(false)
      && op(internal::ptr_uint64(true), terminal) == internal::ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the left,
  ///        i.e. `op(t, X) == X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_irrelevant(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(false)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the right, i.e. `op(X, t) == ~X`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_negating(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(true)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the left, i.e. `op(t, X) == ~X`
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_negating(const bool_op &op, const internal::ptr_uint64 &terminal)
  {
    adiar_assert(terminal.is_terminal());
    return op(terminal, internal::ptr_uint64(false)) == internal::ptr_uint64(true)
      && op(terminal, internal::ptr_uint64(true)) == internal::ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, `op(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_commutative(const bool_op &op)
  {
    internal::ptr_uint64 terminal_T = internal::ptr_uint64(true);
    internal::ptr_uint64 terminal_F = internal::ptr_uint64(false);

    return op(terminal_T, terminal_F) == op(terminal_F, terminal_T);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Return the flipped operator, i.e. `op'(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////
  inline bool_op flip(const bool_op &op)
  {
    if (is_commutative(op)) return op;

    return [&op](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b)
           {return op(b, a);};
  }
}

#endif // ADIAR_BOOL_OP_H
