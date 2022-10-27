#ifndef ADIAR_BOOL_OP_H
#define ADIAR_BOOL_OP_H

#include <functional>

#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar {
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
  typedef std::function<uid_t(ptr_uint64,ptr_uint64)> bool_op;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'and' operator, i.e. the truth table: [1,0,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op and_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return unflag(terminal1 & terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not and' operator, i.e. the truth table: [0,1,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nand_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return negate(and_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'or' operator, i.e. the truth table: [1,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op or_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return unflag(terminal1 | terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not or' operator, i.e. the truth table: [0,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nor_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return negate(or_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [0,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xor_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return unflag(terminal1 ^ terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [1,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xnor_op  = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return negate(xor_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,0,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op imp_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return ptr_uint64(unflag(terminal1) <= unflag(terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,1,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op invimp_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return ptr_uint64(unflag(terminal2) <= unflag(terminal1));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'equivalence' operator, i.e. the 'xnor' operator
  //////////////////////////////////////////////////////////////////////////////
  const bool_op equiv_op = xnor_op;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'set difference' operator, i.e. the truth table [0,1,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op diff_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return and_op(terminal1, negate(terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less' operator, i.e. the truth table [0,0,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op less_op = [](ptr_uint64 terminal1, ptr_uint64 terminal2) -> ptr_uint64
  {
    return and_op(negate(terminal1), terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right,
  ///        i.e. op(T,terminal) = op(F,terminal).
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_right_shortcut(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(ptr_uint64(false), terminal) == op(ptr_uint64(true), terminal);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left,
  ///        i.e. op(terminal, T) = op(terminal, F).
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_left_shortcut(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(terminal, ptr_uint64(false)) == op(terminal, ptr_uint64(true));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the right,
  ///        i.e. op(X, terminal) = X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_irrelevant(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(ptr_uint64(false), terminal) == ptr_uint64(false)
      && op(ptr_uint64(true), terminal) == ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the left,
  ///        i.e. op(terminal, X) = X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_irrelevant(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(terminal, ptr_uint64(false)) == ptr_uint64(false)
      && op(terminal, ptr_uint64(true)) == ptr_uint64(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the right, i.e. op(X, terminal) = ~X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_negating(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(terminal, ptr_uint64(false)) == ptr_uint64(true)
      && op(terminal, ptr_uint64(true)) == ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the left, i.e. op(terminal, X) = ~X
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_negating(const bool_op &op, const ptr_uint64 terminal)
  {
    return op(terminal, ptr_uint64(false)) == ptr_uint64(true)
      && op(terminal, ptr_uint64(true)) == ptr_uint64(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, op(X, Y) = op(Y, X).
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_commutative(const bool_op &op)
  {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    return op(terminal_T, terminal_F) == op(terminal_F, terminal_T);
  }
}

#endif // ADIAR_BOOL_OP_H
