#ifndef ADIAR_ZDD_H
#define ADIAR_ZDD_H

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__zdd Zero-suppressed Decision Diagrams
///
/// \brief A Zero-suppressed Decision Diagram (ZDD) represents a family of a set
/// of \f$ n \f$ numbers, i.e. an \f$ S \subseteq 2^{\{ 0, 1, \dots, n-1 \}} \f$.
///
/// The \ref zdd class takes care of reference counting and optimal garbage
/// collection of the underlying files. To ensure the most disk-space is
/// available, try to garbage collect the \ref zdd objects as quickly as
/// possible and/or minimise the number of lvalues of said type.
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <iostream>
#include <optional>
#include <string>

#include <adiar/bool_op.h>
#include <adiar/file.h>

#include <adiar/internal/dd_func.h>

#include <adiar/zdd/zdd.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__zdd
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Basic ZDD Constructors
  ///
  /// To construct a more complex but well-structured \ref zdd by hand, please
  /// use the \ref zdd_builder (see \ref builder) instead.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The ZDD of only a single terminal.
  ///
  /// \param value The constant terminal value.
  ///
  /// \see         zdd_empty zdd_null
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_terminal(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The empty family, i.e. Ø .
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_empty();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The family only with the empty set, i.e. { Ø } .
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_null();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The set of bitvectors over a given domain where *var* is set to
  ///            true.
  ///
  /// \details This function is (given the same domain of variables)
  ///          semantically equivalent to `bdd_ithvar` even though the ZDD DAG
  ///          does not at all look like the BDD DAG.
  ///
  /// \param var The variable to be forced to true.
  ///
  /// \param dom Generator function of the domain in *descending* order.
  ///
  /// \pre       The variable `var` should occur in `dom`.
  ///
  /// \throws invalid_argument If `dom` is not in *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_ithvar(zdd::label_t var, const std::function<zdd::label_t()> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The set of bitvectors over a given domain where *var* is set to
  ///            true.
  ///
  /// \param var The variable to be forced to true.
  ///
  /// \param begin Iterator that provides the domain in *descending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \pre       The variable `var` should occur in `dom`.
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  zdd zdd_ithvar(zdd::label_t var, IT begin, IT end)
  {
    return zdd_ithvar(var, internal::iterator_gen<zdd::label_t>(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The set of bitvectors over the globally set domain where *var*
  ///            is set to true.
  ///
  /// \param var The variable to be forced to true.
  ///
  /// \pre       `adiar_has_domain() == true` and the variable `var` should
  ///            occur in the global domain.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_ithvar(zdd::label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The set of bitvectors over a given domain where *var* is set to
  ///            false.
  ///
  /// \details   Creates a ZDD with a don't care chain of nodes to the true
  ///            child except for the node for `var`; this one instead is forced
  ///            to be true.
  ///
  /// \param var The variable to be forced to false.
  ///
  /// \param dom Generator function of the domain in *descending* order.
  ///
  /// \pre       The variable `var` should occur in `dom`.
  ///
  /// \throws invalid_argument If `dom` is not in *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_nithvar(zdd::label_t var, const std::function<zdd::label_t()> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The set of bitvectors over a given domain where *var* is set
  ///              to false.
  ///
  /// \param var   The variable to be forced to false.
  ///
  /// \param begin Iterator that provides the domain in *descending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \pre         The variable `var` should occur in `dom`.
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  zdd zdd_nithvar(zdd::label_t var, IT begin, IT end)
  {
    return zdd_nithvar(var, internal::iterator_gen<zdd::label_t>(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The set of bitvectors over the globally set domain where *var*
  ///            is set to false.
  ///
  /// \param var The variable to be forced to false.
  ///
  /// \pre       `adiar_has_domain() == true` and the variable `var` should
  ///            occur in the global domain.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_nithvar(zdd::label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { { 1, 2, ..., k } }.
  ///
  /// \details   Creates a ZDD with a chain of nodes on the 'high' arc to the
  ///            true child, and false otherwise.
  ///
  /// \param vars Generator function of the variables in *descending* order.
  ///
  /// \throws invalid_argument If `vars` are not in *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_vars(const std::function<zdd::label_t()> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The family { { 1, 2, ..., k } }.
  ///
  /// \param begin Iterator that provides the variables in *descending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  zdd zdd_vars(IT begin, IT end)
  {
    return zdd_vars(internal::iterator_gen<zdd::label_t>(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { {i} } .
  ///
  /// \details   Creates a ZDD of a single node with label `var` and the
  ///            children false and true. The given label must be smaller than
  ///            `zdd::MAX_LABEL`.
  ///
  /// \param var The label of the desired variable to include
  ///
  /// \throws invalid_argument If `var` is a too large value.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singleton(zdd::label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The family { {1}, {2}, ..., {k} }.
  ///
  /// \details   Creates a ZDD with a chain of nodes on the 'low' arc to the
  ///            true child, and false otherwise.
  ///
  /// \param vars Generator function of the variables in *descending* order.
  ///
  /// \throws invalid_argument If `vars` are not in *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singletons(const std::function<zdd::label_t()> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The family { {1}, {2}, ..., {k} }.
  ///
  /// \param begin Iterator that provides the variables in *descending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  zdd zdd_singletons(IT begin, IT end)
  {
    return zdd_singletons(internal::iterator_gen<zdd::label_t>(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The powerset of all given variables.
  ///
  /// \details   Creates a ZDD with a don't care chain of nodes to the true
  ///            child.
  ///
  /// \param vars Generator function of the variables in *descending* order.
  ///
  /// \throws invalid_argument If `vars` are not in *ascending* order.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_powerset(const std::function<zdd::label_t()> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The powerset of all given variables.
  ///
  /// \param begin Iterator that provides the variables in *descending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          *descending* order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  zdd zdd_powerset(IT begin, IT end)
  {
    return zdd_powerset(internal::iterator_gen<zdd::label_t>(begin, end));
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Basic ZDD Manipulation
  ///
  /// \{

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

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_union
  //////////////////////////////////////////////////////////////////////////////
  __zdd operator| (const zdd &lhs, const zdd &rhs);

  /// \cond
  __zdd operator| (__zdd &&, __zdd &&);
  __zdd operator| (const zdd &, __zdd &&);
  __zdd operator| (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The intersection of two families of sets.
  ///
  /// \returns
  /// \f$ A \cap B \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __zdd zdd_intsec(const zdd &A, const zdd &B)
  { return zdd_binop(A, B, and_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_intsec
  //////////////////////////////////////////////////////////////////////////////
  __zdd operator& (const zdd &lhs, const zdd &rhs);

  /// \cond
  __zdd operator& (__zdd &&, __zdd &&);
  __zdd operator& (const zdd &, __zdd &&);
  __zdd operator& (__zdd &&, const __zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The set difference of two families of sets.
  ///
  /// \returns
  /// \f$ A \setminus B \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __zdd zdd_diff(const zdd &A, const zdd &B)
  { return zdd_binop(A, B, diff_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_diff
  //////////////////////////////////////////////////////////////////////////////
  __zdd operator- (const zdd &lhs, const zdd &rhs);

  /// \cond
  __zdd operator- (__zdd &&, __zdd &&);
  __zdd operator- (const zdd &, __zdd &&);
  __zdd operator- (__zdd &&, const zdd &);
  /// \endcond

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
  __zdd zdd_change(const zdd &A, const shared_file<zdd::label_t> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Complement of A within the given domain.
  ///
  /// \param A   family of sets to complement
  ///
  /// \param dom Labels of the domain (in ascending order)
  ///
  /// \returns    \f$ 2^{\mathit{dom}} \setminus A \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_complement(const zdd &A, const shared_file<zdd::label_t> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Complement of A within the global \ref module__domain
  ///
  /// \param A   family of sets to complement
  ///
  /// \returns   \f$ 2^{\mathit{dom}} \setminus A \f$
  ///
  /// \see adiar_set_domain adiar_has_domain
  ///
  /// \pre       The global \ref module__domain is set to a set of variables
  ///            that is equals to or a superset of the variables in `A`.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_complement(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_complement
  //////////////////////////////////////////////////////////////////////////////
  __zdd operator~ (const zdd& A);

  /// \cond
  __zdd operator~ (__zdd&& A);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Expands the domain of the given ZDD to also include the given
  ///             set of labels.
  ///
  /// \details    Adds don't care nodes on each levels in `vars`. The variables
  ///             in `vars` may \em not be present in `A`.
  ///
  /// \param A    Family of set to expand
  ///
  /// \param vars Labels of variables to expand with (in ascending order). This
  ///             set of labels may \em not occur in A
  ///
  /// \returns
  /// \f$ \bigcup_{a \in A, i \in 2^{vars}} (a \cup i) \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_expand(const zdd &A, const shared_file<zdd::label_t> &vars);

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
  __zdd zdd_offset(const zdd &A, const shared_file<zdd::label_t> &vars);

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
  __zdd zdd_onset(const zdd &A, const shared_file<zdd::label_t> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Project family of sets onto a domain, i.e. remove from every
  ///            set all variables not within the domain.
  ///
  /// \param A   Family of sets to project
  ///
  /// \param dom Predicate function that defines the domain.
  ///
  /// \returns
  /// \f$ \prod_{\mathit{dom}}(A) = \{ a \setminus \mathit{dom}^c \mid a \in A \} \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_project(const zdd &A, const std::function<bool(zdd::label_t)> &dom);

  /// \cond
  __zdd zdd_project(zdd &&A, const std::function<bool(zdd::label_t)> &dom);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Project family of sets onto a domain, i.e. remove from every
  ///            set all variables not within the domain.
  ///
  /// \param A   Family of sets to project
  ///
  /// \param gen Generator function, that produces the variables of the domain in
  ///            *descending* order. When none are left to-be quantified, it
  ///            returns a value larger than `bdd::MAX_LABEL`, e.g. -1.
  ///
  /// \returns
  /// \f$ \prod_{\mathit{dom}}(A) = \{ a \setminus \mathit{dom}^c \mid a \in A \} \f$
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_project(const zdd &A, const std::function<zdd::label_t()> &dom);

  /// \cond
  __zdd zdd_project(zdd &&A, const std::function<zdd::label_t()> &dom);
  /// \endcond


  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Project family of sets onto a domain, i.e. remove from every
  ///            set all variables not within the domain.
  ///
  /// \param A   Family of sets to project
  ///
  /// \param begin Iterator that provides the domain in *descending* order.
  ///
  /// \returns
  /// \f$ \prod_{\mathit{dom}}(A) = \{ a \setminus \mathit{dom}^c \mid a \in A \} \f$
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  __zdd zdd_project(const zdd &A, IT begin, IT end)
  {
    return zdd_project(A, internal::iterator_gen<bdd::label_t>(begin, end));
  }

  /// \cond
  template<typename IT>
  __zdd zdd_project(zdd &&A, IT begin, IT end)
  {
    return zdd_project(std::forward<zdd>(A),
                       internal::iterator_gen<bdd::label_t>(begin, end));
  }
  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name ZDD Predicates
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::is_canonical
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_canonical(const zdd& A)
  { return internal::is_canonical(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether this ZDD represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(const zdd& A)
  { return internal::is_terminal(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether this ZDD represents false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const zdd& A)
  { return internal::is_false(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it is the empty family, i.e. Ø .
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_empty(const zdd &A)
  { return is_false(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether this BDD represents true terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(const zdd& A)
  { return internal::is_true(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it is the null family, i.e. { Ø } .
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_null(const zdd &A)
  { return is_true(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether they represent the same family.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_equal(const zdd &A, const zdd &B);

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_equal
  //////////////////////////////////////////////////////////////////////////////
  bool operator== (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator== (__zdd &&, __zdd &&);
  bool operator== (const zdd &, __zdd &&);
  bool operator== (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether they represent two different families.
  //////////////////////////////////////////////////////////////////////////////
  inline bool zdd_unequal(const zdd &A, const zdd &B)
  { return !zdd_equal(A,B); };

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_unequal
  //////////////////////////////////////////////////////////////////////////////
  bool operator!= (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator!= (__zdd &&, __zdd &&);
  bool operator!= (const zdd &, __zdd &&);
  bool operator!= (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether one family is a subset or equal to the other.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_subseteq(const zdd &A, const zdd &B);

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_subseteq
  //////////////////////////////////////////////////////////////////////////////
  bool operator<= (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator<= (__zdd &&, __zdd &&);
  bool operator<= (const zdd &, __zdd &&);
  bool operator<= (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_subseteq
  //////////////////////////////////////////////////////////////////////////////
  bool operator>= (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator>= (__zdd &&, __zdd &&);
  bool operator>= (const zdd &, __zdd &&);
  bool operator>= (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether one family is a strict subset of the other.
  //////////////////////////////////////////////////////////////////////////////
  inline bool zdd_subset(const zdd &A, const zdd &B)
  { return zdd_subseteq(A,B) && zdd_unequal(A,B); };

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_subset
  //////////////////////////////////////////////////////////////////////////////
  bool operator< (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator< (__zdd &&, __zdd &&);
  bool operator< (const zdd &, __zdd &&);
  bool operator< (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \see zdd_subset
  //////////////////////////////////////////////////////////////////////////////
  bool operator> (const zdd& lhs, const zdd& rhs);

  /// \cond
  bool operator> (__zdd &&, __zdd &&);
  bool operator> (const zdd &, __zdd &&);
  bool operator> (__zdd &&, const zdd &);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two families are disjoint.
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_disjoint(const zdd &A, const zdd &B);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name ZDD Counting Operations
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of (internal) nodes used to represent the family of
  ///        sets.
  //////////////////////////////////////////////////////////////////////////////
  size_t zdd_nodecount(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of variables that exist in the family of sets, i.e. the
  ///        number of levels in the ZDD.
  //////////////////////////////////////////////////////////////////////////////
  zdd::label_t zdd_varcount(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of sets in the family of sets.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t zdd_size(const zdd &A);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Set Elements
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring variable in this ZDD.
  //////////////////////////////////////////////////////////////////////////////
  inline zdd::label_t min_var(const zdd &A)
  { return internal::min_var(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring variable in this ZDD.
  //////////////////////////////////////////////////////////////////////////////
  inline zdd::label_t max_var(const zdd &A)
  { return internal::max_var(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Whether the family includes the given set of labels
  ///
  /// \returns Whether \f$ a \in A \f$
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_contains(const zdd &A, const shared_file<zdd::label_t> &a);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically smallest set a in A.
  ///
  /// \details Outputs the trace of the low-most path to the true terminal. The
  ///          resulting assignment is lexicographically smallest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_minelem(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically smallest set a in A.
  ///
  /// \param cb Callback function that is called in ascending order of the zdd's
  ///           levels with the variable in the smallest set.
  ///
  /// \pre `A != zdd_empty()`
  //////////////////////////////////////////////////////////////////////////////
  void zdd_minelem(const zdd &A, const std::function<void(zdd::label_t)> &cb);

  //////////////////////////////////////////////////////////////////////////////
  // TODO: wrap into consumer lambda
  //
  // template<typename IT>
  // zdd_minelem(const zdd &A, IT begin, IT end)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically largest set a in A.
  ///
  /// \details Outputs the trace of the high-most path to the true terminal. The
  ///          resulting assignment is lexicographically largest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_maxelem(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Retrieves the lexicographically largest set a in A.
  ///
  /// \param cb Callback function that is called in ascending order of the zdd's
  ///           levels with the variable in the largest set.
  ///
  /// \pre `A != zdd_empty()`
  //////////////////////////////////////////////////////////////////////////////
  void zdd_maxelem(const zdd &A, const std::function<void(zdd::label_t)> &cb);

  //////////////////////////////////////////////////////////////////////////////
  // TODO: wrap into consumer lambda
  //
  // template<typename IT>
  // zdd_maxelem(const zdd &A, IT begin, IT end)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the labels of the levels of the ZDD.
  ///
  /// \param A  ZDD of interest.
  ///
  /// \param cb Callback function that consumes the levels (in ascending order).
  //////////////////////////////////////////////////////////////////////////////
  void zdd_varprofile(const zdd &A, const std::function<void(zdd::label_t)> &cb);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Write the labels of the ZDD's levels into the given container.
  ///
  /// \param A     ZDD of interest.
  ///
  /// \param begin Iterator for the beginning to place the output.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  ///
  /// \returns     An iterator to the first entry that still is left empty.
  ///
  /// \throws out_of_range If the distance between `begin` and `end` is not big
  ///                      enough to contain all variables in `f`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  IT zdd_varprofile(const zdd &A, IT begin, IT end)
  {
    if (std::distance(begin, end) < zdd_varcount(A)) {
      throw std::out_of_range("Distance between 'begin' and 'end' too small");
    }
    zdd_varprofile(A, internal::iterator_consumer<bdd::label_t>(begin, end));
    return begin;
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Conversion to ZDDs
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the ZDD that represents the same function/set as the
  ///            given BDD within the given domain.
  ///
  /// \param f   Boolean function with the given domain
  ///
  /// \param dom Domain of all variables (in ascending order)
  ///
  /// \returns   ZDD that is true for the exact same assignments to variables in
  ///            the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_from(const bdd &f, const shared_file<zdd::label_t> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the ZDD that represents the same function/set as the
  ///            given BDD within the global \ref module__domain.
  ///
  /// \param f   Boolean function with the given domain
  ///
  /// \returns   ZDD that is true for the exact same assignments to variables in
  ///            the global domain.
  ///
  /// \pre       The global \ref module__domain is set to a set of variables
  ///            that is equals to or a superset of the variables in `A`.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_from(const bdd &f);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Dot Files of ZDDs
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a ZDD to the given output stream.
  //////////////////////////////////////////////////////////////////////////////
  void zdd_printdot(const zdd &A, std::ostream &out = std::cout);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a ZDD to the file with the given name.
  //////////////////////////////////////////////////////////////////////////////
  void zdd_printdot(const zdd &A, const std::string &file_name);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_ZDD_H
