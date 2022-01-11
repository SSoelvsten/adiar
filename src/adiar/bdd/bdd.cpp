#include "bdd.h"

#include <adiar/bdd.h>

#include <memory>

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/convert.h>
#include <adiar/internal/intercut.h>
#include <adiar/internal/reduce.h>

#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  // Constructors
  __bdd::__bdd() : __decision_diagram() { }
  __bdd::__bdd(const node_file &f) : __decision_diagram(f) { }
  __bdd::__bdd(const arc_file &f) : __decision_diagram(f) { }

  __bdd::__bdd(const bdd &dd) : __decision_diagram(dd) { }

  bdd::bdd(const node_file &f, bool negate) : decision_diagram(f, negate) { }

  bdd::bdd(const bdd &o) : decision_diagram(o) { }
  bdd::bdd(bdd &&o) : decision_diagram(o) { }

  bdd::bdd(__bdd &&o) : decision_diagram(reduce<bdd_policy>(std::forward<__bdd>(o))) { }

  bdd::bdd(bool v) : bdd(bdd_sink(v)) { }
  bdd::bdd() : bdd(false) { }

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
    free();
    return (*this = reduce<bdd_policy>(std::forward<__bdd>(other)));
  }

  bdd& bdd::operator&= (const bdd &other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd& bdd::operator&= (bdd &&other)
  {
    __bdd&& temp = bdd_and(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator|= (const bdd &other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd& bdd::operator|= (bdd &&other)
  {
    __bdd&& temp = bdd_or(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator^= (const bdd &other)
  {
    return (*this = bdd_xor(*this, other));
  }

  bdd& bdd::operator^= (bdd &&other)
  {
    __bdd&& temp = bdd_xor(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bool operator== (const bdd &lhs, const bdd &rhs)
  {
    return is_isomorphic(lhs, rhs);
  }

  bool operator!= (const bdd &lhs, const bdd &rhs) { return !(lhs == rhs); }

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
  // Conversion
  __bdd bdd_from(const zdd &dd, const label_file &dom)
  {
    return intercut<convert_decision_diagram_policy<bdd_policy, zdd_policy>>(dd,dom);
  }
}
