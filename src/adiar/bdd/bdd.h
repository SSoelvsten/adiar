#ifndef ADIAR_BDD_BDD_H
#define ADIAR_BDD_BDD_H

#include <adiar/internal/decision_diagram.h>
#include <adiar/file.h>

namespace adiar {
  // Class declarations to be able to reference it
  class bdd;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based BDD in a node_file or a yet to-be
  /// reduced BDD in an arc_file.
  //////////////////////////////////////////////////////////////////////////////
  class __bdd : public __decision_diagram {
  public:
    __bdd(const node_file &f);
    __bdd(const arc_file &f);
    __bdd(const bdd &bdd);
  };

  // operators to allow __bdd&& arguments on one or two sides of an expression
  bdd operator~ (__bdd&&);
  __bdd operator& (__bdd &&, __bdd &&);
  __bdd operator| (__bdd &&, __bdd &&);
  __bdd operator^ (__bdd &&, __bdd &&);
  bool operator== (__bdd &&, __bdd &&);
  bool operator!= (__bdd &&, __bdd &&);
  bool operator== (const bdd &, __bdd &&);
  bool operator!= (const bdd &, __bdd &&);
  bool operator== (__bdd &&, const bdd &);
  bool operator!= (__bdd &&, const bdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// A BDD is the reduced node-based representation of a decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  class bdd : public decision_diagram
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend __bdd;

    // |- functions
    friend bdd bdd_not(const bdd&);
    friend bdd bdd_not(bdd&&);
    friend size_t bdd_nodecount(const bdd&);
    friend label_t bdd_varcount(const bdd&);

    // |- operators
    friend bool operator== (const bdd& lhs, const bdd& rhs);
    friend bool operator!= (const bdd& lhs, const bdd& rhs);

    friend bdd operator~ (const bdd& bdd);
    friend bdd operator~ (bdd&& bdd);

    friend class apply_prod_policy;
    friend __bdd bdd_ite(const bdd &bdd_if, const bdd &bdd_then, const bdd &bdd_else);
    friend __bdd operator& (const bdd &lhs, const bdd &rhs);
    friend __bdd operator| (const bdd &lhs, const bdd &rhs);
    friend __bdd operator^ (const bdd &lhs, const bdd &rhs);

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    bdd(const node_file &f, bool negate = false);

    bdd(const bdd &o);
    bdd(bdd &&o);

    bdd(__bdd &&o);

    bdd(bool v);

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    bdd& operator= (const bdd &other);
    bdd& operator= (__bdd &&other);

    bdd& operator&= (const bdd &other);
    bdd& operator&= (bdd &&other);

    bdd& operator|= (const bdd &other);
    bdd& operator|= (bdd &&other);

    bdd& operator^= (const bdd &other);
    bdd& operator^= (bdd &&other);
  };
}

#endif // ADIAR_BDD_BDD_H
