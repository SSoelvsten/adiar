/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADIAR_ZDD_ZDD_H
#define ADIAR_ZDD_ZDD_H

#include <adiar/internal/decision_diagram.h>
#include <adiar/file.h>

namespace adiar {
  // Class declarations to be able to reference it
  class zdd;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based ZDD in a node_file or a yet to-be
  /// reduced ZDD in an arc_file.
  //////////////////////////////////////////////////////////////////////////////
  class __zdd : public __decision_diagram {
  public:
    __zdd();
    __zdd(const node_file &f);
    __zdd(const arc_file &f);
    __zdd(const zdd &zdd);
  };

  // operators to allow __zdd&& arguments on one or two sides of an expression
  bool operator== (__zdd &&, __zdd &&);
  bool operator== (const zdd &, __zdd &&);
  bool operator== (__zdd &&, const zdd &);

  bool operator!= (__zdd &&, __zdd &&);
  bool operator!= (const zdd &, __zdd &&);
  bool operator!= (__zdd &&, const zdd &);

  bool operator<= (__zdd &&, __zdd &&);
  bool operator<= (const zdd &, __zdd &&);
  bool operator<= (__zdd &&, const zdd &);

  bool operator>= (__zdd &&, __zdd &&);
  bool operator>= (const zdd &, __zdd &&);
  bool operator>= (__zdd &&, const zdd &);

  bool operator< (__zdd &&, __zdd &&);
  bool operator< (const zdd &, __zdd &&);
  bool operator< (__zdd &&, const zdd &);

  bool operator> (__zdd &&, __zdd &&);
  bool operator> (const zdd &, __zdd &&);
  bool operator> (__zdd &&, const zdd &);

  __zdd operator& (__zdd &&, __zdd &&);
  __zdd operator& (const zdd &, __zdd &&);
  __zdd operator& (__zdd &&, const __zdd &);

  __zdd operator| (__zdd &&, __zdd &&);
  __zdd operator| (const zdd &, __zdd &&);
  __zdd operator| (__zdd &&, const zdd &);

  __zdd operator- (__zdd &&, __zdd &&);
  __zdd operator- (const zdd &, __zdd &&);
  __zdd operator- (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// A ZDD is the reduced node-based representation of a decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  class zdd : public decision_diagram
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend __zdd;

    // |- functions
    friend size_t zdd_nodecount(const zdd&);
    friend label_t zdd_varcount(const zdd&);

    friend bool zdd_subseteq(const zdd&, const zdd&);
    friend bool zdd_disjoint(const zdd &, const zdd &);

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    zdd(const node_file &f, bool negate = false);

    zdd();
    zdd(const zdd &o);
    zdd(zdd &&o);

    zdd(__zdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    zdd& operator= (const zdd &other);
    zdd& operator= (__zdd &&other);

    zdd& operator&= (const zdd &other);
    zdd& operator&= (zdd &&other);

    zdd& operator|= (const zdd &other);
    zdd& operator|= (zdd &&other);

    zdd& operator-= (const zdd &other);
    zdd& operator-= (zdd &&other);
  };

  bool operator== (const zdd& lhs, const zdd& rhs);
  bool operator!= (const zdd& lhs, const zdd& rhs);

  bool operator<= (const zdd& lhs, const zdd& rhs);
  bool operator>= (const zdd& lhs, const zdd& rhs);

  bool operator< (const zdd& lhs, const zdd& rhs);
  bool operator> (const zdd& lhs, const zdd& rhs);

  __zdd operator& (const zdd &lhs, const zdd &rhs);
  __zdd operator| (const zdd &lhs, const zdd &rhs);
  __zdd operator- (const zdd &lhs, const zdd &rhs);

  //////////////////////////////////////////////////////////////////////////////
  class zdd_policy
  {
  public:
    typedef zdd reduced_t;
    typedef __zdd unreduced_t;

  public:
    static inline ptr_t reduction_rule(const node_t &n)
    {
      if (is_sink(n.high) && !value_of(n.high)) { return n.low; }
      return n.uid;
    }

  public:
    static inline void compute_cofactor(bool on_curr_level, ptr_t &, ptr_t &high)
    {
      if (!on_curr_level) { high = create_sink_ptr(false); }
    }
  };
}

#endif // ADIAR_ZDD_ZDD_H
