/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "bdd.h"

#include <memory>

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/pred.h>
#include <adiar/internal/reduce.h>

#include <adiar/bdd/apply.h>
#include <adiar/bdd/build.h>
#include <adiar/bdd/negate.h>

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

  bdd operator~ (const bdd &in_bdd) { return bdd_not(in_bdd); }
  bdd operator~ (bdd &&in_bdd) { return bdd_not(std::forward<bdd>(in_bdd)); }

  __bdd operator& (const bdd &lhs, const bdd &rhs) { return bdd_and(lhs, rhs); }
  __bdd operator| (const bdd &lhs, const bdd &rhs) { return bdd_or(lhs, rhs); }
  __bdd operator^ (const bdd &lhs, const bdd &rhs) { return bdd_xor(lhs, rhs); }
}
