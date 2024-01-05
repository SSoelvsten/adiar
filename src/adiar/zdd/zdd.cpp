#include "zdd.h"

#include <adiar/zdd.h>

#include <memory>

#include <adiar/domain.h>
#include <adiar/exception.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/algorithms/convert.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // '__zdd' Constructors
  __zdd::__zdd()
    : internal::__dd()
  { }

  __zdd::__zdd(const shared_node_file_type &f)
    : internal::__dd(f)
  { }

  __zdd::__zdd(const shared_arc_file_type &f, const exec_policy &ep)
    : internal::__dd(f, ep)
  { }

  __zdd::__zdd(const zdd &dd)
    : internal::__dd(dd)
  { }

  //////////////////////////////////////////////////////////////////////////////
  // 'zdd' Constructors
  zdd::zdd()
    : zdd(zdd_empty())
  { }

  zdd::zdd(terminal_type t)
    : zdd(zdd_terminal(t))
  { }

  zdd::zdd(const shared_node_file_type &A, bool negate)
    : internal::dd(A, negate)
  {
    if (negate) { throw invalid_argument("ZDDs cannot be negated"); }
  }

  zdd::zdd(const zdd &A)
    : internal::dd(A)
  { }

  zdd::zdd(zdd &&A)
    : internal::dd(A)
  { }

  zdd::zdd(__zdd &&A)
    : internal::dd(internal::reduce<zdd_policy>(std::move(A)))
  { }

  //////////////////////////////////////////////////////////////////////////////
  // Operators
#define __zdd_oper(out_t, op)                                              \
  out_t operator op (__zdd &&lhs, __zdd &&rhs) {                           \
    return zdd(std::move(lhs)) op zdd(std::move(rhs));                     \
  }                                                                        \
                                                                           \
  out_t operator op (const zdd &lhs, __zdd &&rhs) {                        \
    return lhs op zdd(std::move(rhs));                                     \
  }                                                                        \
                                                                           \
  out_t operator op (__zdd &&lhs, const zdd &rhs) {                        \
    return zdd(std::move(lhs)) op rhs;                                     \
  }

  __zdd_oper(__zdd, &)
  __zdd_oper(__zdd, |)
  __zdd_oper(__zdd, -)

  __zdd_oper(bool, ==)
  __zdd_oper(bool, !=)
  __zdd_oper(bool, <=)
  __zdd_oper(bool, >=)
  __zdd_oper(bool, <)
  __zdd_oper(bool, >)

  zdd& zdd::operator= (const zdd &other)
  {
    this -> negate = other.negate;
    this -> file = other.file;
    return *this;
  }

  zdd& zdd::operator= (__zdd &&other)
  {
    deref();
    return (*this = internal::reduce<zdd_policy>(std::move(other)));
  }

  __zdd operator~ (const zdd &A)
  {
    return zdd_complement(A);
  }

  __zdd operator~ (__zdd &&A)
  {
    return ~zdd(std::move(A));
  }

  __zdd operator& (const zdd& lhs, const zdd& rhs)
  {
    return zdd_intsec(lhs, rhs);
  }

  zdd& zdd::operator&= (const zdd &other)
  {
    return (*this = zdd_intsec(*this, other));
  }

  zdd& zdd::operator&= (zdd &&other)
  {
    __zdd&& temp = zdd_intsec(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __zdd operator| (const zdd& lhs, const zdd& rhs)
  {
    return zdd_union(lhs, rhs);
  }

  zdd& zdd::operator|= (const zdd &other)
  {
    return (*this = zdd_union(*this, other));
  }

  zdd& zdd::operator|= (zdd &&other)
  {
    __zdd&& temp = zdd_union(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __zdd operator- (const zdd& lhs, const zdd& rhs)
  {
    return zdd_diff(lhs, rhs);
  }

  zdd& zdd::operator-= (const zdd &other)
  {
    return (*this = zdd_diff(*this, other));
  }

  zdd& zdd::operator-= (zdd &&other)
  {
    __zdd&& temp = zdd_diff(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  bool operator== (const zdd& lhs, const zdd& rhs)
  {
    return zdd_equal(lhs, rhs);
  }

  bool operator!= (const zdd& lhs, const zdd& rhs) {
    return zdd_unequal(lhs, rhs);
  }

  bool operator<= (const zdd& lhs, const zdd& rhs)
  {
    return zdd_subseteq(lhs, rhs);
  }

  bool operator>= (const zdd& lhs, const zdd& rhs) {
    return zdd_subseteq(rhs, lhs);
  }

  bool operator< (const zdd& lhs, const zdd& rhs)
  {
    return zdd_subset(lhs, rhs);
  }

  bool operator> (const zdd& lhs, const zdd& rhs) {
    return zdd_subset(rhs, lhs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Input variables
  zdd::label_type zdd_topvar(const zdd &A)
  {
    return internal::dd_topvar(A);
  }

  zdd::label_type zdd_minvar(const zdd &A)
  {
    return internal::dd_minvar(A);
  }

  zdd::label_type zdd_maxvar(const zdd &A)
  {
    return internal::dd_maxvar(A);
  }

  void zdd_support(const zdd &A, const consumer<zdd::label_type> &cb)
  {
    return internal::dd_support(A, cb);
  }
}
