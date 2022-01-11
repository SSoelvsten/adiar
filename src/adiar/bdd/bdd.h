#ifndef ADIAR_BDD_BDD_H
#define ADIAR_BDD_BDD_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/tuple.h>

namespace adiar {
  class bdd;

  //////////////////////////////////////////////////////////////////////////////
  /// \internal \brief A (possibly non-reduced) Ordered Binary Decision Diagram.
  ///
  /// \relates bdd
  ///
  /// \extends __decision_diagram
  ///
  /// \copydoc __decision_diagram
  //////////////////////////////////////////////////////////////////////////////
  class __bdd : public __decision_diagram {
  public:
    __bdd();
    __bdd(const node_file &f);
    __bdd(const arc_file &f);
    __bdd(const bdd &bdd);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduced Ordered Binary Decision Diagram.
  ///
  /// \extends decision_diagram
  ///
  /// \copydoc decision_diagram
  //////////////////////////////////////////////////////////////////////////////
  class bdd : public decision_diagram
  {
    friend __bdd;

    friend bdd bdd_not(const bdd&);
    friend bdd bdd_not(bdd&&);
    friend size_t bdd_nodecount(const bdd&);
    friend label_t bdd_varcount(const bdd&);

    friend class apply_prod_policy;
    friend __bdd bdd_ite(const bdd &bdd_if, const bdd &bdd_then, const bdd &bdd_else);

  public:
    bdd();
    bdd(const node_file &f, bool negate = false);
    bdd(const bdd &o);
    bdd(bdd &&o);
    bdd(__bdd &&o);
    bdd(bool v);

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
