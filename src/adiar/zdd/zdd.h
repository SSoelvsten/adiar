#ifndef ADIAR_ZDD_ZDD_H
#define ADIAR_ZDD_ZDD_H

#include <adiar/data.h>
#include <adiar/tuple.h>
#include <adiar/file.h>
#include <adiar/internal/decision_diagram.h>

namespace adiar {
  // Class declarations to be able to reference it
  class zdd;

  //////////////////////////////////////////////////////////////////////////////
  /// \internal \brief A (possibly non-reduced) Ordered Zero-suppressed Decision
  /// Diagram.
  ///
  /// \relates zdd
  ///
  /// \extends __decision_diagram
  ///
  /// \copydoc __decision_diagram
  //////////////////////////////////////////////////////////////////////////////
  class __zdd : public __decision_diagram {
  public:
    __zdd();
    __zdd(const node_file &f);
    __zdd(const arc_file &f);
    __zdd(const zdd &zdd);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduced Ordered Zero-suppressed Decision Diagram.
  ///
  /// \extends decision_diagram
  ///
  /// \copydoc decision_diagram
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
    zdd(bool v);

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

    static inline tuple reduction_rule_inv(const ptr_t &child)
    {
      return { child, create_sink_ptr(false) };
    }

  public:
    static inline void compute_cofactor(bool on_curr_level, ptr_t &, ptr_t &high)
    {
      if (!on_curr_level) { high = create_sink_ptr(false); }
    }
  };
}

#endif // ADIAR_ZDD_ZDD_H
