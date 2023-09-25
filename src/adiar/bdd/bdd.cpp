#include "bdd.h"

#include <adiar/bdd.h>

#include <memory>

#include <adiar/domain.h>
#include <adiar/internal/dot.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/algorithms/convert.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // '__bdd' Constructors
  __bdd::__bdd()
    : internal::__dd()
  { }

  __bdd::__bdd(const internal::__dd::shared_nodes_t &f)
    : internal::__dd(f)
  { }

  __bdd::__bdd(const internal::__dd::shared_arcs_t &f)
    : internal::__dd(f)
  { }

  __bdd::__bdd(const bdd &dd)
    : internal::__dd(dd)
  { }

  //////////////////////////////////////////////////////////////////////////////
  // 'bdd' Constructors
  bdd::bdd(const internal::dd::shared_nodes_t &f, bool negate)
    : internal::dd(f, negate)
  { }

  bdd::bdd(const bdd &o)
    : internal::dd(o)
  { }

  bdd::bdd(bdd &&o)
    : internal::dd(o)
  { }

  bdd::bdd(__bdd &&o)
    : internal::dd(internal::reduce<bdd_policy>(std::forward<__bdd>(o)))
  { }

  bdd::bdd(bool v)
    : bdd(bdd_terminal(v))
  { }

  bdd::bdd()
    : bdd(false)
  { }

  //////////////////////////////////////////////////////////////////////////////
  // Operators
  bdd operator~ (__bdd &&in) { return ~bdd(std::forward<__bdd>(in)); }

#define __bdd_oper(out_t, op)                                              \
  out_t operator op (__bdd &&lhs, __bdd &&rhs) {                           \
    return bdd(std::forward<__bdd>(lhs)) op bdd(std::forward<__bdd>(rhs)); \
  }                                                                        \
                                                                           \
  out_t operator op (const bdd &lhs, __bdd &&rhs) {                        \
    return lhs op bdd(std::forward<__bdd>(rhs));                           \
  }                                                                        \
                                                                           \
  out_t operator op (__bdd &&lhs, const bdd &rhs) {                        \
    return bdd(std::forward<__bdd>(lhs)) op rhs;                           \
  }

  __bdd_oper(__bdd, &)
  __bdd_oper(__bdd, |)
  __bdd_oper(__bdd, ^)
  __bdd_oper(bool, ==)
  __bdd_oper(bool, !=)

  bdd& bdd::operator= (const bdd &other)
  {
    this -> negate = other.negate;
    this -> file = other.file;
    return *this;
  }

  bdd& bdd::operator= (__bdd &&other)
  {
    deref();
    return (*this = internal::reduce<bdd_policy>(std::forward<__bdd>(other)));
  }

  bdd& bdd::operator&= (const bdd &other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd& bdd::operator&= (bdd &&other)
  {
    __bdd&& temp = bdd_and(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator|= (const bdd &other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd& bdd::operator|= (bdd &&other)
  {
    __bdd&& temp = bdd_or(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator^= (const bdd &other)
  {
    return (*this = bdd_xor(*this, other));
  }

  bdd& bdd::operator^= (bdd &&other)
  {
    __bdd&& temp = bdd_xor(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  bool operator== (const bdd &lhs, const bdd &rhs)
  {
    return bdd_equal(lhs, rhs);
  }

  bool operator!= (const bdd &lhs, const bdd &rhs)
  { return bdd_unequal(lhs, rhs); }

  bdd operator~ (const bdd &f)
  { return bdd_not(f); }

  bdd operator~ (bdd &&f)
  { return bdd_not(std::forward<bdd>(f)); }

  __bdd operator& (const bdd &lhs, const bdd &rhs)
  { return bdd_and(lhs, rhs); }

  __bdd operator| (const bdd &lhs, const bdd &rhs)
  { return bdd_or(lhs, rhs); }

  __bdd operator^ (const bdd &lhs, const bdd &rhs)
  { return bdd_xor(lhs, rhs); }

  //////////////////////////////////////////////////////////////////////////////
  // Input variables
  void bdd_varprofile(const bdd &f, const consumer<bdd::label_t> &cb)
  {
    return internal::dd_varprofile(f, cb);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Conversion
  __bdd bdd_from(const zdd &A, const generator<bdd::label_t> &dom)
  {
    return internal::intercut<internal::convert_dd_policy<bdd_policy, zdd_policy>>
      (A, dom);
  }

  __bdd bdd_from(const zdd &A)
  {
    const shared_file<bdd::label_t> dom = adiar_get_domain();
    internal::file_stream<domain_var_t> ds(dom);

    return internal::intercut<internal::convert_dd_policy<bdd_policy, zdd_policy>>
      (A, make_generator(ds));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Debug
  void bdd_printdot(const bdd &f, std::ostream &out)
  {
    internal::output_dot<bdd>(f, out);
  }

  void bdd_printdot(const bdd &f, const std::string &file_name)
  {
    internal::output_dot<bdd>(f, file_name);
  }
}
