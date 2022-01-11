#ifndef ADIAR_ZDD_H
#define ADIAR_ZDD_H

#include <optional>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  /* =========================== ZDD CONSTRUCTION =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The ZDD of only a single sink.
  ///
  /// \param value The constant sink value.
  ///
  /// \sa          zdd_empty zdd_null
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_sink(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The empty family, i.e. Ø .
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_empty();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The family only with the empty set, i.e. { Ø } .
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_null();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { {i} } .
  ///
  /// \details   Creates a ZDD of a single node with label <tt>var</tt> and the
  ///            children false and true. The given label must be smaller than
  ///            <tt>MAX_LABEL</tt>.
  ///
  /// \param var The label of the desired variable to include
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_ithvar(label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { { 1, 2, ..., k } }.
  ///
  /// \details   Creates a ZDD with a chain of nodes on the 'high' arc to the
  ///            true child, and false otherwise. The given labels must be
  ///            smaller than or equal to <tt>MAX_LABEL</tt>.
  ///
  /// \param vars Labels of the desired variables (in ascending order)
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_vars(const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { {1}, {2}, ..., {k} }.
  ///
  /// \details   Creates a ZDD with a chain of nodes on the 'low' arc to the
  ///            true child, and false otherwise. The given labels must be
  ///            smaller than or equal to <tt>MAX_LABEL</tt>.
  ///
  /// \param vars Labels of the desired variables (in ascending order)
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singletons(const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The powerset of all given variables.
  ///
  /// \details   Creates a ZDD with a don't care chain of nodes to the true
  ///            child. The given labels must be smaller than or equal to
  ///            <tt>MAX_LABEL</tt>.
  ///
  /// \param vars Labels of the desired variables (in ascending order)
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_powerset(const label_file &vars);

  // For templated constructors see 'adiar/zdd/build.h'

  /* =========================== ZDD MANIPULATION =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Apply a binary operator between the sets of two families.
  ///
  ///
  /// \param A   ZDD for the left-hand-side of the operator
  /// \param B   ZDD for the right-hand-side of the operator
  /// \param op  Binary boolean operator to be applied. See 'adiar/data.h'
  ///
  /// \return Product construction of the two that represents the boolean
  ///         operator applied to the two family of sets.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_binop(const zdd &A, const zdd &B, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The union of two families of sets.
  ///
  /// \returns
  /// \f$ A \cup B \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __zdd zdd_union(const zdd &A, const zdd &B)
  { return zdd_binop(A, B, or_op); };

  __zdd operator| (const zdd &lhs, const zdd &rhs);
  __zdd operator| (__zdd &&, __zdd &&);
  __zdd operator| (const zdd &, __zdd &&);
  __zdd operator| (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The intersection of two families of sets.
  ///
  /// \returns
  /// \f$ A \cap B \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __zdd zdd_intsec(const zdd &A, const zdd &B)
  { return zdd_binop(A, B, and_op); };

  __zdd operator& (const zdd &lhs, const zdd &rhs);
  __zdd operator& (__zdd &&, __zdd &&);
  __zdd operator& (const zdd &, __zdd &&);
  __zdd operator& (__zdd &&, const __zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The set difference of two families of sets.
  ///
  /// \returns
  /// \f$ A \setminus B \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __zdd zdd_diff(const zdd &A, const zdd &B)
  { return zdd_binop(A, B, diff_op); };

  __zdd operator- (const zdd &lhs, const zdd &rhs);
  __zdd operator- (__zdd &&, __zdd &&);
  __zdd operator- (const zdd &, __zdd &&);
  __zdd operator- (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The symmetric difference between each set in the family and the
  ///            given set of variables.
  ///
  /// \param A    ZDD to apply with the other.
  ///
  /// \param vars Labels that should be flipped
  ///
  /// \returns
  /// \f$ \{ \mathit{vars} \Delta a \mid a \in A \} \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_change(const zdd &A, const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Complement of A within the given domain.
  ///
  /// \param A   family of sets to complement
  ///
  /// \param dom Labels of the domain (in ascending order)
  ///
  /// \returns
  /// \f$ 2^{\mathit{dom}} \setminus A \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_complement(const zdd &A, const label_file &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Expands the domain of the given ZDD to also include the given
  ///             set of labels.
  ///
  /// \details    Adds don't care nodes on each levels in <tt>vars</tt>. The
  ///             variables in <tt>vars</tt> may \em not be present in
  ///             <tt>A</tt>.
  ///
  /// \param A    Family of set to expand
  ///
  /// \param vars Labels of variables to expand with (in ascending order). This
  ///             set of labels may \em not occur in A
  ///
  /// \returns
  /// \f$ \bigcup_{a \in A, i \in 2^{vars}} (a \cup i) \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_expand(const zdd &A, const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Subset that do \em not include the given set of variables.
  ///
  /// \param A    Family of set
  ///
  /// \param vars Label of the variables to filter on (in ascending order)
  ///
  /// \returns
  /// \f$ \{ a \in A \mid \forall i \in \mathit{vars} : i \not\in a \} \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_offset(const zdd &A, const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Subset that \em do include the given set of variables.
  ///
  /// \param A    Family of set
  ///
  /// \param vars Label of the variables to filter on (in ascending order)
  ///
  /// \returns
  /// \f$ \{ a \in A \mid \forall i \in \mathit{vars} : i \in a \} \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_onset(const zdd &A, const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Project family of sets onto a domain, i.e. remove from every
  ///            set all variables not mentioned.
  ///
  /// \param A   Family of sets to project
  ///
  /// \param dom The domain to project onto (in ascending order)
  ///
  /// \returns
  /// \f$ \prod_{\mathit{dom}}(A) = \{ a \setminus \mathit{dom}^c \mid a \in A \} \f$
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_project(const zdd &A, const label_file &dom);
  zdd zdd_project(zdd &&A, const label_file &dom);

  /* ============================ ZDD PREDICATES ============================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether they represent the same family.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_equal(const zdd &A, const zdd &B);

  bool operator== (const zdd& lhs, const zdd& rhs);
  bool operator== (__zdd &&, __zdd &&);
  bool operator== (const zdd &, __zdd &&);
  bool operator== (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether they represent two different families.
  //////////////////////////////////////////////////////////////////////////////
  inline bool zdd_unequal(const zdd &A, const zdd &B)
  { return !zdd_equal(A,B); };

  bool operator!= (const zdd& lhs, const zdd& rhs);
  bool operator!= (__zdd &&, __zdd &&);
  bool operator!= (const zdd &, __zdd &&);
  bool operator!= (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether one family is a subset or equal to the other.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_subseteq(const zdd &A, const zdd &B);

  bool operator<= (const zdd& lhs, const zdd& rhs);
  bool operator<= (__zdd &&, __zdd &&);
  bool operator<= (const zdd &, __zdd &&);
  bool operator<= (__zdd &&, const zdd &);

  bool operator>= (const zdd& lhs, const zdd& rhs);
  bool operator>= (__zdd &&, __zdd &&);
  bool operator>= (const zdd &, __zdd &&);
  bool operator>= (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether one family is a strict subset of the other.
  //////////////////////////////////////////////////////////////////////////////
  inline bool zdd_subset(const zdd &A, const zdd &B)
  { return zdd_subseteq(A,B) && zdd_unequal(A,B); };

  bool operator< (const zdd& lhs, const zdd& rhs);
  bool operator< (__zdd &&, __zdd &&);
  bool operator< (const zdd &, __zdd &&);
  bool operator< (__zdd &&, const zdd &);

  bool operator> (const zdd& lhs, const zdd& rhs);
  bool operator> (__zdd &&, __zdd &&);
  bool operator> (const zdd &, __zdd &&);
  bool operator> (__zdd &&, const zdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two families are disjoint.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_disjoint(const zdd &A, const zdd &B);

  /* ============================= ZDD COUNTING ============================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of (internal) nodes used to represent the family of
  ///        sets.
  //////////////////////////////////////////////////////////////////////////////
  size_t zdd_nodecount(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of variables that exist in the family of sets, i.e. the
  ///        number of levels in the ZDD.
  //////////////////////////////////////////////////////////////////////////////
  label_t zdd_varcount(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of sets in the family of sets.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t zdd_size(const zdd &A);

  /* =========================== ZDD SET ELEMENTS =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Whether the family includes the given set of labels
  ///
  /// \returns Whether \f$ a \in A \f$
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_contains(const zdd &A, const label_file &a);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically smallest set a in A.
  ///
  /// \details Outputs the trace of the low-most path to the true sink. The
  ///          resulting assignment is lexicographically smallest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  //////////////////////////////////////////////////////////////////////////////
  std::optional<label_file> zdd_minelem(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically largest set a in A.
  ///
  /// \details Outputs the trace of the high-most path to the true sink. The
  ///          resulting assignment is lexicographically largest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  //////////////////////////////////////////////////////////////////////////////
  std::optional<label_file> zdd_maxelem(const zdd &A);
}

#include <adiar/zdd/build.h>

#endif // ADIAR_ZDD_H
