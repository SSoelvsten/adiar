#ifndef ADIAR_BOOL_OP_H
#define ADIAR_BOOL_OP_H

#include <adiar/functional.h>

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
    return !(and_op(a, b));
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
    return !(or_op(a, b));
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
    return !(xor_op(a, b));
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
    return and_op(a, !b);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less' operator, i.e. the truth table [0,0,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op less_op = [](const internal::ptr_uint64 &a, const internal::ptr_uint64 &b) -> internal::ptr_uint64
  {
    return and_op(!a, b);
  };
}

#endif // ADIAR_BOOL_OP_H
