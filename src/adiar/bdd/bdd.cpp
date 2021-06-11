#include "bdd.h"

#include <memory>

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/isomorphism.h>
#include <adiar/internal/reduce.h>

#include <adiar/bdd/apply.h>
#include <adiar/bdd/build.h>
#include <adiar/bdd/negate.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  bdd reduce(__bdd &&maybe_reduced)
  {
    if (maybe_reduced.has<arc_file>()) {
      return reduce(maybe_reduced.get<arc_file>(), reduction_rule_bdd);
    }
    return bdd(maybe_reduced.get<node_file>(), maybe_reduced.negate);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Constructors
  __bdd::__bdd() : __decision_diagram() { }
  __bdd::__bdd(const node_file &f) : __decision_diagram(f) { }
  __bdd::__bdd(const arc_file &f) : __decision_diagram(f) { }

  __bdd::__bdd(const bdd &dd) : __decision_diagram(dd) { }

  bdd::bdd(const node_file &f, bool negate) : decision_diagram(f, negate) { }

  bdd::bdd() : bdd(false) { }

  bdd::bdd(const bdd &o) : decision_diagram(o) { }
  bdd::bdd(bdd &&o) : decision_diagram(o) { }

  bdd::bdd(__bdd &&o) : decision_diagram(reduce(std::forward<__bdd>(o))) { }

  bdd::bdd(bool v) : bdd(bdd_sink(v)) { }

  //////////////////////////////////////////////////////////////////////////////
  // Operators
  bdd operator~ (__bdd &&in) { return ~bdd(std::forward<__bdd>(in)); }

  __bdd operator& (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) & bdd(std::forward<__bdd>(rhs));
  }

  __bdd operator| (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) | bdd(std::forward<__bdd>(rhs));
  }

  __bdd operator^ (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) ^ bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) == bdd(std::forward<__bdd>(rhs));
  }

  bool operator!= (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) != bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (const bdd &lhs, __bdd &&rhs) {
    return lhs == bdd(std::forward<__bdd>(rhs));
  }

  bool operator!= (const bdd &lhs, __bdd &&rhs) {
    return lhs != bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (__bdd &&lhs, const bdd &rhs) {
    return bdd(std::forward<__bdd>(lhs)) == rhs;
  }

  bool operator!= (__bdd &&lhs, const bdd &rhs) {
    return bdd(std::forward<__bdd>(lhs)) != rhs;
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd& bdd::operator= (const bdd &other)
  {
    this -> negate = other.negate;
    this -> file = other.file;
    return *this;
  }

  bdd& bdd::operator= (__bdd &&other)
  {
    free();
    return (*this = reduce(std::forward<__bdd>(other)));
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

  bool operator== (const bdd& lhs, const bdd& rhs)
  {
    return is_isomorphic(lhs.file, rhs.file, lhs.negate, rhs.negate);
  }

  bool operator!= (const bdd& lhs, const bdd& rhs) { return !(lhs == rhs); }

  bdd operator~ (const bdd &in_bdd) { return bdd_not(in_bdd); }
  bdd operator~ (bdd &&in_bdd) { return bdd_not(std::forward<bdd>(in_bdd)); }

  __bdd operator& (const bdd& lhs, const bdd& rhs) { return bdd_and(lhs, rhs); }
  __bdd operator| (const bdd& lhs, const bdd& rhs) { return bdd_or(lhs, rhs); }
  __bdd operator^ (const bdd& lhs, const bdd& rhs) { return bdd_xor(lhs, rhs); }
}
