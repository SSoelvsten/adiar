#include "zdd.h"

#include <memory>

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/reduce.h>

#include <adiar/zdd/build.h>
#include <adiar/zdd/pred.h>
#include <adiar/zdd/set_op.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  zdd reduce(__zdd &&maybe_reduced)
  {
    if (maybe_reduced.has<arc_file>()) {
      return reduce(maybe_reduced.get<arc_file>(), reduction_rule_zdd);
    }
    return zdd(maybe_reduced.get<node_file>(), maybe_reduced.negate);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Constructors
  __zdd::__zdd() : __decision_diagram() { }
  __zdd::__zdd(const node_file &f) : __decision_diagram(f) { }
  __zdd::__zdd(const arc_file &f) : __decision_diagram(f) { }

  __zdd::__zdd(const zdd &dd) : __decision_diagram(dd) { }

  zdd::zdd(const node_file &f, bool negate) : decision_diagram(f, negate) { }

  zdd::zdd() : zdd(zdd_empty()) { }

  zdd::zdd(const zdd &o) : decision_diagram(o) { }
  zdd::zdd(zdd &&o) : decision_diagram(o) { }

  zdd::zdd(__zdd &&o) : decision_diagram(reduce(std::forward<__zdd>(o))) { }

  //////////////////////////////////////////////////////////////////////////////
  // Operators
#define __zdd_oper(out_t, op)                                              \
  out_t operator op (__zdd &&lhs, __zdd &&rhs) {                           \
    return zdd(std::forward<__zdd>(lhs)) op zdd(std::forward<__zdd>(rhs)); \
  }                                                                        \
                                                                           \
  out_t operator op (const zdd &lhs, __zdd &&rhs) {                        \
    return lhs op zdd(std::forward<__zdd>(rhs));                           \
  }                                                                        \
                                                                           \
  out_t operator op (__zdd &&lhs, const zdd &rhs) {                        \
    return zdd(std::forward<__zdd>(lhs)) op rhs;                           \
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

  //////////////////////////////////////////////////////////////////////////////
  zdd& zdd::operator= (const zdd &other)
  {
    this -> negate = other.negate;
    this -> file = other.file;
    return *this;
  }

  zdd& zdd::operator= (__zdd &&other)
  {
    free();
    return (*this = reduce(std::forward<__zdd>(other)));
  }

  zdd& zdd::operator&= (const zdd &other)
  {
    return (*this = zdd_intsec(*this, other));
  }

  zdd& zdd::operator&= (zdd &&other)
  {
    __zdd&& temp = zdd_intsec(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  zdd& zdd::operator|= (const zdd &other)
  {
    return (*this = zdd_union(*this, other));
  }

  zdd& zdd::operator|= (zdd &&other)
  {
    __zdd&& temp = zdd_union(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  zdd& zdd::operator-= (const zdd &other)
  {
    return (*this = zdd_diff(*this, other));
  }

  zdd& zdd::operator-= (zdd &&other)
  {
    __zdd&& temp = zdd_diff(*this, other);
    other.free();
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

  __zdd operator& (const zdd& lhs, const zdd& rhs) { return zdd_intsec(lhs, rhs); }
  __zdd operator| (const zdd& lhs, const zdd& rhs) { return zdd_union(lhs, rhs); }
  __zdd operator- (const zdd& lhs, const zdd& rhs) { return zdd_diff(lhs, rhs); }
}
