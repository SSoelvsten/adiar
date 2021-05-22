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
    __zdd(const node_file &f);
    __zdd(const arc_file &f);
    __zdd(const zdd &zdd);
  };

  // operators to allow __zdd&& arguments on one or two sides of an expression
  bool operator== (__zdd &&, __zdd &&);
  bool operator!= (__zdd &&, __zdd &&);
  bool operator== (const zdd &, __zdd &&);
  bool operator!= (const zdd &, __zdd &&);
  bool operator== (__zdd &&, const zdd &);
  bool operator!= (__zdd &&, const zdd &);

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
    friend uint64_t zdd_nodecount(const zdd&);
    friend uint64_t zdd_varcount(const zdd&);

    // |- operators
    friend bool operator== (const zdd& lhs, const zdd& rhs);
    friend bool operator!= (const zdd& lhs, const zdd& rhs);

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    zdd(const node_file &f, bool negate = false);

    zdd(const zdd &o);
    zdd(zdd &&o);

    zdd(__zdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    zdd& operator= (const zdd &other);
    zdd& operator= (__zdd &&other);
  };
}

#endif // ADIAR_ZDD_ZDD_H
