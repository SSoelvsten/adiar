#include "zdd.h"

#include <memory>

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/isomorphism.h>
#include <adiar/internal/reduce.h>

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

  zdd::zdd(const zdd &o) : decision_diagram(o) { }
  zdd::zdd(zdd &&o) : decision_diagram(o) { }

  zdd::zdd(__zdd &&o) : decision_diagram(reduce(std::forward<__zdd>(o))) { }

  //////////////////////////////////////////////////////////////////////////////
  // Operators
  bool operator== (__zdd &&lhs, __zdd &&rhs) {
    return zdd(std::forward<__zdd>(lhs)) == zdd(std::forward<__zdd>(rhs));
  }

  bool operator!= (__zdd &&lhs, __zdd &&rhs) {
    return zdd(std::forward<__zdd>(lhs)) != zdd(std::forward<__zdd>(rhs));
  }

  bool operator== (const zdd &lhs, __zdd &&rhs) {
    return lhs == zdd(std::forward<__zdd>(rhs));
  }

  bool operator!= (const zdd &lhs, __zdd &&rhs) {
    return lhs != zdd(std::forward<__zdd>(rhs));
  }

  bool operator== (__zdd &&lhs, const zdd &rhs) {
    return zdd(std::forward<__zdd>(lhs)) == rhs;
  }

  bool operator!= (__zdd &&lhs, const zdd &rhs) {
    return zdd(std::forward<__zdd>(lhs)) != rhs;
  }

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

  bool operator== (const zdd& lhs, const zdd& rhs)
  {
    return is_isomorphic(lhs.file, rhs.file);
  }

  bool operator!= (const zdd& lhs, const zdd& rhs) { return !(lhs == rhs); }
}
