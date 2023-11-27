#ifndef ADIAR_BDD_H
#define ADIAR_BDD_H

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__bdd Binary Decision Diagrams
///
/// \brief A Binary Decision Diagram (BDD) represents a boolean function
/// \f$ \{ 0,1 \}^n \rightarrow \{ 0,1 \} \f$ over a finite domain of
/// \f$ n \f$ boolean input variables.
///
/// The \ref bdd class takes care of reference counting and optimal garbage
/// collection of the underlying files. To ensure the most disk-space is
/// available, try to garbage collect the \ref bdd objects as quickly as
/// possible and/or minimise the number of lvalues of said type.
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>

#include <adiar/bool_op.h>
#include <adiar/exec_policy.h>
#include <adiar/functional.h>
#include <adiar/types.h>

#include <adiar/bdd/bdd.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__bdd
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Basic BDD Constructors
  ///
  /// To construct a more complex but well-structured \ref bdd by hand, please
  /// use the \ref bdd_builder (see \ref builder) instead.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the given constant value.
  ///
  /// \param value The constant Boolean value
  ///
  /// \see         bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_const(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the given constant value.
  ///
  /// \param value The constant Boolean (terminal) value
  ///
  /// \see         bdd_const bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_terminal(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant `false`.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_false();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The bottom of the powerset lattice.
  ///
  /// \see bdd_false
  //////////////////////////////////////////////////////////////////////////////
  inline bdd bdd_bot()
  { return bdd_false(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant `true`.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_true();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The top of the powerset lattice.
  ///
  /// \see bdd_true
  //////////////////////////////////////////////////////////////////////////////
  inline bdd bdd_top()
  { return bdd_true(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The BDD representing the i'th variable.
  ///
  /// \param var The label of the desired variable. This value must be smaller
  ///            or equals to `bdd::max_label`.
  ///
  /// \returns   \f$ x_{var} \f$
  ///
  /// \throws invalid_argument If `var` is a too large value.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_ithvar(bdd::label_type var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The BDD representing the negation of the i'th variable.
  ///
  /// \param var The label of the desired variable. This value must be smaller
  ///            or equals to `bdd::max_label`.
  ///
  /// \returns   \f$ \neg x_{var} \f$
  ///
  /// \throws invalid_argument If `var` is a too large value.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_nithvar(bdd::label_type var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'and' of all the given
  ///             variables, i.e. a *term* of variables.
  ///
  /// \details    Any negative labels provided by the generator are interpreted
  ///             as the negation of said variable.
  ///
  /// \param vars Generator of labels of variables in \em descending order.
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_and(const generator<int> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'and' of all the given
  ///             variables, i.e. a *term* of variables.
  ///
  /// \param vars Generator of pairs (label, negated) in \em descending order.
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_and(const generator<pair<bdd::label_type, bool>> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the logical 'and' of all the given
  ///              variables, i.e. a *term* of variables.
  ///
  /// \details     Any negative labels provided by the generator are interpreted
  ///              as the negation of said variable.
  ///
  /// \param begin Single-pass forward iterator that provides the variables in
  ///              \em descending order. All its values should be smaller than
  ///              or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          descending order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  bdd bdd_and(ForwardIt begin, ForwardIt end)
  { return bdd_and(make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'or' of all the given
  ///             variables, i.e. a *clause* of variables.
  ///
  /// \details    Any negative labels provided by the generator are interpreted
  ///             as the negation of said variable.
  ///
  /// \param vars Generator of labels of variables in \em descending order. When
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigvee_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_or(const generator<int> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'or' of all the given
  ///             variables, i.e. a *clause* of variables.
  ///
  /// \param vars Generator of pairs (label, negated) in \em descending order.
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigvee_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_or(const generator<pair<bdd::label_type, bool>> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the logical 'or' of all the given
  ///              variables, i.e. a *clause* of variables.
  ///
  /// \details     Any negative labels provided by the generator are interpreted
  ///              as the negation of said variable.
  ///
  /// \param begin Single-pass forward iterator that provides the variables in
  ///              \em descending order. All its values should be smaller than
  ///              or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          descending order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  bdd bdd_or(ForwardIt begin, ForwardIt end)
  { return bdd_or(make_generator(begin, end)); }


  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the cube of all the given variables.
  ///
  /// \details    Any negative labels provided by the generator are interpreted
  ///             as the negation of said variable.
  ///
  /// \param vars Generator of labels of variables in \em descending order.
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_cube(const generator<int> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the cube of all the given variables.
  ///
  /// \param vars Generator of pairs (label, negated) in \em descending order.
  ///             These values can at most be `bdd::max_label`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_cube(const generator<pair<bdd::label_type, bool>> &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the cube of all the given variables.
  ///
  /// \details     Any negative labels provided by the generator are interpreted
  ///              as the negation of said variable.
  ///
  /// \param begin Single-pass forward iterator that provides the variables in
  ///              \em descending order. All its values should be smaller than
  ///              or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in
  ///                          descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  bdd bdd_cube(ForwardIt begin, ForwardIt end)
  { return bdd_cube(make_generator(begin, end)); }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Basic BDD Manipulation
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Negation of a BDD.
  ///
  /// \details Flips the negation flag such that reading nodes with a
  ///          `node_stream` within Adiar's algorithms will on-the-fly change
  ///          the `false` terminals into the `true` terminals and vica versa.
  ///
  /// \returns \f$ \neg f \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_not(const bdd &f);

  /// \cond
  bdd bdd_not(bdd &&f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_not
  //////////////////////////////////////////////////////////////////////////////
  bdd operator~ (const bdd& f);

  /// \cond
  bdd operator~ (bdd&& f);
  bdd operator~ (__bdd&& f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Apply a binary operator between two BDDs.
  ///
  /// \param f  BDD for the left-hand-side of the operator
  /// \param g  BDD for the right-hand-side of the operator
  /// \param op Binary boolean operator to be applied.
  ///
  /// \returns  The product construction of \f$ f \mathbin{\mathit{op}} g\f$
  ///
  /// \see bool_op
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &f, const bdd &g, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Apply a binary operator between two BDDs.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const exec_policy &ep,
                  const bdd &f,
                  const bdd &g,
                  const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'and' operator.
  ///
  /// \returns \f$ f \land g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'and' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const exec_policy &ep, const bdd &f, const bdd &g);

  /// \cond
  ///
  /// \see bdd_and
  ///
  /// \remark Since `bdd_and<ForwardIt>(begin, end)` has precedence over the
  ///         implicit conversion from `bdd::shared_node_file_type` to `bdd`, we
  ///         have to do it explicitly ourselves.
  inline __bdd bdd_and(const bdd::shared_node_file_type &f,
                       const bdd::shared_node_file_type &g)
  { return bdd_and(bdd(f), bdd(g)); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////
  __bdd operator& (const bdd &lhs, const bdd &rhs);

  /// \cond
  __bdd operator& (const bdd &, __bdd &&);
  __bdd operator& (__bdd &&, const bdd &);
  __bdd operator& (__bdd &&, __bdd &&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nand' operator.
  ///
  /// \returns \f$ \neg (f \land g) \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_nand(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nand' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_nand(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'or' operator.
  ///
  /// \returns \f$ f \lor g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_or(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'or' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_or(const exec_policy &ep, const bdd &f, const bdd &g);

  /// \cond
  ///
  /// \see bdd_or
  ///
  /// \remark Since `bdd_or<ForwardIt>(begin, end)` has precedence over the
  ///         implicit conversion from `bdd::shared_node_file_type` to `bdd`, we
  ///         have to do it explicitly ourselves.
  inline __bdd bdd_or(const bdd::shared_node_file_type &f,
                      const bdd::shared_node_file_type &g)
  { return bdd_or(bdd(f), bdd(g)); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_or
  //////////////////////////////////////////////////////////////////////////////
  __bdd operator| (const bdd &lhs, const bdd &rhs);

  /// \cond
  __bdd operator| (const bdd &, __bdd &&);
  __bdd operator| (__bdd &&, const bdd &);
  __bdd operator| (__bdd &&, __bdd &&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nor' operator.
  ///
  /// \returns \f$ \neg (f \lor g) \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_nor(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nor' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_nor(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xor' operator.
  ///
  /// \returns \f$ f \oplus g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_xor(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xor' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_xor(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_xor
  //////////////////////////////////////////////////////////////////////////////
  __bdd operator^ (const bdd &lhs, const bdd &rhs);

  /// \cond
  __bdd operator^ (const bdd &, __bdd &&);
  __bdd operator^ (__bdd &&, const bdd &);
  __bdd operator^ (__bdd &&, __bdd &&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xnor' operator.
  ///
  /// \returns \f$ \neg (f \oplus g) \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_xnor(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xnor' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_xnor(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'implication' operator.
  ///
  /// \returns \f$ f \rightarrow g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_imp(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'implication' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_imp(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'inverse implication' operator.
  ///
  /// \returns \f$ f \leftarrow g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_invimp(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'inverse implication' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_invimp(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'equivalence' operator.
  ///
  /// \returns \f$ f = g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_equiv(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'equivalence' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_equiv(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'difference'  operator.
  ///
  /// \returns \f$ f \setminus g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_diff(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'difference'  operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_diff(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'less than' operator.
  ///
  /// \returns \f$ f < g \f$
  ///
  /// \see     bdd_apply
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_less(const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'less than' operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_less(const exec_policy &ep, const bdd &f, const bdd &g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   If-Then-Else operator.
  ///
  /// \details Computes the BDD expressing \f$ f ? g : h \f$ more efficient than
  ///          computing \f$ (f \land g) \lor (\neg f \land h) \f$ with
  ///          `bdd_apply`.
  ///
  /// \param f BDD for the if conditional
  ///
  /// \param g BDD for the then case
  ///
  /// \param h BDD for the else case
  ///
  /// \returns \f$ f ? g : h \f$
  ///
  /// \remark In other BDD packages, this function is good for manually
  ///         constructing a BDD bottom-up. But, here you should use the \ref
  ///         bdd_builder class (see \ref builder) instead
  ///
  /// \see    bdd_apply builder bdd_builder
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_ite(const bdd &f, const bdd &g, const bdd &h);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   If-Then-Else operator.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_ite(const exec_policy &ep, const bdd &f, const bdd &g, const bdd &h);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Restrict a single variable to a constant value.
  ///
  /// \details   The variable `i` is restructed to the value `v`
  ///
  /// \param f   BDD to restrict.
  ///
  /// \param var Variable to assign
  ///
  /// \param val Value assigned
  ///
  /// \returns  \f$ f|_{x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &f, bdd::label_type var, bool val);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a single variable to a constant value.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd &f,
                     bdd::label_type var,
                     bool val);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Restrict a subset of variables to constant values.
  ///
  /// \details  For each tuple (i,v) in the assignment `xs`, the variable
  ///           with label i is set to the constant value v. This binds the
  ///           scope of the variables in `xs`, i.e. any later mention of
  ///           a variable i is not the same as variable i in `xs`.
  ///
  /// \param f  BDD to restrict.
  ///
  /// \param xs Assignments (i,v) to variables in (in ascending order).
  ///
  /// \returns  \f$ f|_{(i,v) \in xs : x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &f,
                     const generator<pair<bdd::label_type, bool>> &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a subset of variables to constant values.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd &f,
                     const generator<pair<bdd::label_type, bool>> &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       Restrict a subset of variables to constant values.
  ///
  /// \details     For each tuple (i,v) provided by the iterator, the variable
  ///              with label i is set to the constant value v.
  ///
  /// \param f     BDD to restrict
  ///
  /// \param begin Single-pass forward iterator that provides the to-be
  ///              restricted variables in \em ascending order. All variables
  ///              should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns  \f$ f|_{(i,v) \in xs : x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_restrict(const bdd& f, ForwardIt begin, ForwardIt end)
  { return bdd_restrict(f, make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a subset of variables to constant values.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_restrict(const exec_policy &ep,
                     const bdd& f,
                     ForwardIt begin,
                     ForwardIt end)
  { return bdd_restrict(ep, f, make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Restrict the root to `false`, i.e. follow its low edge.
  ///
  /// \remark In other BDD packages, this function is good for traversing a BDD.
  ///         But, here this is not a constant-time operation but constructs an
  ///         entire new BDD of up-to linear size.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  ///
  /// \see bdd_restrict
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_low(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict the root to `false`, i.e. follow its low edge.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_low(const exec_policy &ep, const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Restrict the root to `true`, i.e. follow its high edge.
  ///
  /// \remark In other BDD packages, this function is good for traversing a BDD.
  ///         But, here this is not a constant-time operation but constructs
  ///         an entire new BDD of up-to linear size.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  ///
  /// \see bdd_restrict
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_high(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict the root to `true`, i.e. follow its high edge.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_high(const exec_policy &ep, const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of a single variable.
  ///
  /// \details   Computes the BDD for \f$ \exists x_{i} : f \f$ faster than
  ///            computing \f$ f|_{x_i = \bot} \lor f|_{x_i = \top} \f$ using
  ///            `bdd_apply` and `bdd_restrict`.
  ///
  /// \param f   BDD to be quantified
  /// \param var Variable to quantify in f
  ///
  /// \returns   \f$ \exists x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &f, bdd::label_type var);

  /// \cond
  inline __bdd bdd_exists(bdd &&f, bdd::label_type var)
  { return bdd_exists(f, var); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const exec_policy &ep, const bdd &f, bdd::label_type var);

  /// \cond
  inline __bdd bdd_exists(const exec_policy &ep, bdd &&f, bdd::label_type var)
  { return bdd_exists(ep, f, var); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Existential quantification of multiple variables.
  ///
  /// \details    Repeatedly calls `bdd_exists` for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified.
  ///
  /// \param vars Predicate to identify the variables to quantify in f. You may
  ///             abuse the fact, that this predicate will only be invoked in
  ///             ascending/descending order of the levels in `f` (but, with
  ///             retraversals).
  ///
  /// \returns    \f$ \exists x_i \in \texttt{vars} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &f, const predicate<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_exists(bdd &&f, const predicate<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const exec_policy &ep,
                   const bdd &f,
                   const predicate<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_exists(const exec_policy &ep,
                   bdd &&f,
                   const predicate<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \details   Repeatedly calls `bdd_exists` for the given variables
  ///            while optimising garbage collecting intermediate results.
  ///
  /// \param f   BDD to be quantified.
  ///
  /// \param vars Generator function, that produces variables to be quantified in
  ///             \em descending order. These values have to be smaller than or
  ///             equals to `bdd::max_label`.
  ///
  /// \returns   \f$ \exists x_i \in \texttt{gen()} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &f, const generator<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_exists(bdd &&f, const generator<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const exec_policy &ep,
                   const bdd &f,
                   const generator<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_exists(const exec_policy &ep,
                   bdd &&f,
                   const generator<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       Existential quantification of multiple variables.
  ///
  /// \details     Repeatedly calls `bdd_exists` for the given variables
  ///              while optimising garbage collecting intermediate results.
  ///
  /// \param f     BDD to be quantified.
  ///
  /// \param begin Single-pass forward iterator that provides the to-be
  ///              quantified variables in \em descending order. All variables
  ///              should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns     \f$ \exists x_i \in \texttt{begin} ... \texttt{end} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_exists(const bdd &f, ForwardIt begin, ForwardIt end)
  { return bdd_exists(f, make_generator(begin, end)); }

  /// \cond
  template<typename ForwardIt>
  __bdd bdd_exists(bdd &&f, ForwardIt begin, ForwardIt end)
  { return bdd_exists(std::move(f), make_generator(begin, end)); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_exists(const exec_policy &ep,
                   const bdd &f,
                   ForwardIt begin,
                   ForwardIt end)
  { return bdd_exists(ep, f, make_generator(begin, end)); }

  /// \cond
  template<typename ForwardIt>
  __bdd bdd_exists(const exec_policy &ep,
                   bdd &&f,
                   ForwardIt begin,
                   ForwardIt end)
  { return bdd_exists(ep, std::move(f), make_generator(begin, end)); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Forall quantification of a single variable.
  ///
  /// \details   Computes the BDD for \f$ \forall x_{i} : f \f$ faster than
  ///            computing \f$ f|_{x_i = \bot} \land f|_{x_i = \top} \f$ using
  ///            `bdd_apply` and `bdd_restrict`.
  ///
  /// \param f   BDD to be quantified.
  ///
  /// \param var Variable to quantify in f
  ///
  /// \returns   \f$ \forall x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &f, bdd::label_type var);

  /// \cond
  inline __bdd bdd_forall(bdd &&f, bdd::label_type var)
  { return bdd_forall(f, var); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const exec_policy &ep, const bdd &f, bdd::label_type var);

  /// \cond
  inline __bdd bdd_forall(const exec_policy &ep, bdd &&f, bdd::label_type var)
  { return bdd_forall(ep, f, var); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Forall quantification of multiple variables.
  ///
  /// \details    Repeatedly calls `bdd_forall` for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified.
  ///
  /// \param vars Predicate to identify the variables to quantify in f. You may
  ///             abuse the fact, that this predicate will only be invoked in
  ///             ascending/descending order of the levels in `f` (but,
  ///             with retraversals).
  ///
  /// \returns    \f$ \forall x_i \in \texttt{vars} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &f, const predicate<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_forall(bdd &&f, const predicate<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const exec_policy &ep,
                   const bdd &f,
                   const predicate<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_forall(const exec_policy &ep,
                   bdd &&f,
                   const predicate<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Forall quantification of multiple variables.
  ///
  /// \details   Repeatedly calls `bdd_forall` for the given variables
  ///            while optimising garbage collecting intermediate results.
  ///
  /// \param f   BDD to be quantified.
  ///
  /// \param gen Generator function, that produces variables to be quantified in
  ///            \em descending order. These values have to be smaller than or
  ///             equals to `bdd::max_label`.
  ///
  /// \returns   \f$ \forall x_i \in \texttt{gen()} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &f, const generator<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_forall(bdd &&f, const generator<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const exec_policy &ep,
                   const bdd &f,
                   const generator<bdd::label_type> &vars);

  /// \cond
  __bdd bdd_forall(const exec_policy &ep,
                   bdd &&f,
                   const generator<bdd::label_type> &vars);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       Forall quantification of multiple variables.
  ///
  /// \details     Repeatedly calls `bdd_forall` for the given variables
  ///              while optimising garbage collecting intermediate results.
  ///
  /// \param f     BDD to be quantified.
  ///
  /// \param begin Single-pass forward iterator that provides the to-be
  ///              quantified variables in \em descending order. All values
  ///              should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns     \f$ \forall x_i \in \texttt{begin} ... \texttt{end} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_forall(const bdd &f, ForwardIt begin, ForwardIt end)
  { return bdd_forall(f, make_generator(begin, end)); }

  /// \cond
  template<typename ForwardIt>
  __bdd bdd_forall(bdd &&f, ForwardIt begin, ForwardIt end)
  { return bdd_forall(std::forward<bdd>(f), make_generator(begin, end)); }
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_forall(const exec_policy &ep,
                   const bdd &f,
                   ForwardIt begin,
                   ForwardIt end)
  { return bdd_forall(ep, f, make_generator(begin, end)); }

  /// \cond
  template<typename ForwardIt>
  __bdd bdd_forall(const exec_policy &ep,
                   bdd &&f,
                   ForwardIt begin,
                   ForwardIt end)
  { return bdd_forall(ep, std::move(f), make_generator(begin, end)); }
  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name BDD Predicates
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given BDD is canonical.
  ///
  /// \copydetails adiar::internal::dd_iscanonical
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_iscanonical(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a constant.
  ///
  /// \see bdd_isfalse bdd_istrue
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isconst(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a constant (terminal).
  ///
  /// \see bdd_isfalse bdd_istrue
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isterminal(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the constant `false`.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isfalse(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the constant `true`.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_istrue(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a function dependent on a single variable.
  ///
  /// \see bdd_isithvar bdd_isnithvar
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the function of a single positive variable.
  ///
  /// \see bdd_isvar bdd_ithvar
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isithvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the function of a single negative variable.
  ///
  /// \see bdd_isvar bdd_nithvar
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_isnithvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD represents a cube.
  ///
  /// \see bdd_cube
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_iscube(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent the same function.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_equal(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent the same function.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_equal(const exec_policy&, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_equal
  //////////////////////////////////////////////////////////////////////////////
  bool operator== (const bdd& f, const bdd& g);

  /// \cond
  bool operator== (__bdd &&f, const bdd &g);
  bool operator== (const bdd &f, __bdd &&g);
  bool operator== (__bdd &&f, __bdd &&g);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent different functions.
  ///
  /// \see bdd_equal
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_unequal(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent different functions.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_unequal(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////
  /// \see bdd_equal bdd_unequal
  //////////////////////////////////////////////////////////////////////////////
  bool operator!= (const bdd& f, const bdd& g);

  /// \cond
  bool operator!= (const bdd &f, __bdd &&g);
  bool operator!= (__bdd &&f, const bdd &g);
  bool operator!= (__bdd &&f, __bdd &&g);
  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name BDD Counting Operations
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of (internal) nodes used to represent the function.
  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of variables that influence the outcome of f, i.e. the
  ///        number of levels in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd::label_type bdd_varcount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to the true
  ///        terminal.
  ///
  /// \returns The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to the true
  ///        terminal.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const exec_policy &ep, const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief          Count the number of assignments x that make f(x) true.
  ///
  /// \param f        BDD to count within.
  ///
  /// \param varcount The number of variables in the domain of the function.
  ///                 This number should be larger than or equal to the number
  ///                 of levels in the BDD (\see bdd_varcount())
  ///
  /// \returns        The number of unique assignments.
  ///
  /// \throws invalid_argument If varcount is not larger than the number of
  ///                          levels in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &f, bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const exec_policy &ep,
                        const bdd &f,
                        bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  ///
  /// \details Same as `bdd_satcount(f, varcount)`, with varcount set to be the
  ///          size of the global domain or the number of variables within the
  ///          given BDD.
  ///
  /// \see domain_set bdd_varcount
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const exec_policy &ep, const bdd &f);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Input Variables
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get (in \em ascending order) all of the variable labels that occur
  ///        in the BDD.
  ///
  /// \param f  BDD of interest.
  ///
  /// \param cb Callback function that consumes the variable labels.
  //////////////////////////////////////////////////////////////////////////////
  void bdd_support(const bdd &f, const consumer<bdd::label_type> &cb);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Copy all of the variable labels (in \em ascending order) that occur
  ///        in the BDD into the given container.
  ///
  /// \param f     BDD of interest.
  ///
  /// \param begin Single-pass forward iterator for where to place the output.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns     An iterator to the first entry that still is left empty.
  ///
  /// \throws out_of_range If the distance between `begin` and `end` is not big
  ///                      enough to contain all variables in `f`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  ForwardIt bdd_support(const bdd &f, ForwardIt begin, ForwardIt end)
  {
    bdd_support(f, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the root's variable label.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  bdd::label_type bdd_topvar(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring variable in the function's support.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  bdd::label_type bdd_minvar(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring variable in the function's support.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  bdd::label_type bdd_maxvar(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically smallest cube x such that f(x) is true.
  ///
  /// \details Outputs the trace of the low-most path to the true terminal. The
  ///          resulting assignment is lexicographically smallest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the minimal
  ///          assignment.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_satmin(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically smallest x such that f(x) is true.
  ///
  /// \param c Consumer that is called in ascending order of the bdd's levels
  ///          with the (var, value) pairs of the assignment.
  //////////////////////////////////////////////////////////////////////////////
  void bdd_satmin(const bdd &f, const consumer<pair<bdd::label_type, bool>> &c);

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Iterator-based output (fix: ForwardIt::value_type does not match with
  //       consumer<...> argument types)
  //
  // template<typename ForwardIt>
  // bdd_satmin(const bdd &f, ForwardIt begin, ForwardIt end)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically largest cube x such that f(x) is true.
  ///
  /// \details Outputs the trace of the high-most path to the true terminal. The
  ///          resulting assignment is lexicographically largest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the maximal
  ///          assignment.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_satmax(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically largest x such that f(x) is true.
  ///
  /// \param c Consumer that is called in ascending order of the bdd's levels
  ///          with the (var, value) pairs of the assignment.
  //////////////////////////////////////////////////////////////////////////////
  void bdd_satmax(const bdd &f, const consumer<pair<bdd::label_type, bool>> &c);

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Iterator-based output (fix: ForwardIt::value_type does not match with
  //       consumer<...> argument types)
  //
  // template<typename ForwardIt>
  // bdd_satmax(const bdd &f, ForwardIt begin, ForwardIt end)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Evaluate a BDD according to an assignment to its variables.
  ///
  /// \details  The given assignment function may assume/abuse that it is only
  ///           called with the labels in a strictly increasing order.
  ///
  /// \param f  The BDD to evaluate
  ///
  /// \param xs An assignment function of the type \f$ \texttt{label\_t}
  ///           \rightarrow \texttt{bool} \f$.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &f, const predicate<bdd::label_type> &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Evaluate a BDD according to an assignment to its variables.
  ///
  /// \param f  The BDD to evaluate.
  ///
  /// \param xs Assignments (i,v) to variables in (in ascending order).
  ///
  /// \pre      Assignment tuples in `xs` is in ascending order
  ///
  /// \throws out_of_range If traversal of the BDD leads to going beyond the end
  ///                      of the content of `xs`.
  ///
  /// \throws invalid_argument If a level in the BDD does not exist in `xs`.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &f, const generator<pair<bdd::label_type, bool>> &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       Evaluate a BDD according to an assignment to its variables.
  ///
  /// \param f     The BDD to evaluate.
  ///
  /// \param begin Single-pass forward iterator that provides the assignment in
  ///              \em ascending order.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \throws out_of_range If traversal of the BDD leads to going beyond the end
  ///                      of the content of `xs`.
  ///
  /// \throws invalid_argument If a level in the BDD does not exist in `xs`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  bool bdd_eval(const bdd &f, ForwardIt begin, ForwardIt end)
  { return bdd_eval(f, make_generator(begin, end)); }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name Conversion to BDDs
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the BDD that represents the same function/set as the
  ///            given ZDD within the given domain.
  ///
  /// \param A   Family of a set (within the given domain)
  ///
  /// \param dom Generator function of domain variables in \em ascending order.
  ///            These values may not exceed `bdd::max_label`.
  ///
  /// \returns   BDD that is true for the exact same assignments to variables in
  ///            the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const zdd &A, const generator<bdd::label_type> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given
  ///        ZDD within the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const exec_policy &ep,
                 const zdd &A,
                 const generator<bdd::label_type> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       Obtains the BDD that represents the same function/set as the
  ///              given ZDD within the given domain.
  ///
  /// \param A     Family of a set (within the given domain)
  ///
  /// \param begin Single-pass forward iterator that provides the domain's
  ///              variables in \em ascending order. These values may not exceed
  ///              `bdd::max_label`.
  ///
  /// \param end   Marks the end for `begin`.
  ///
  /// \returns     BDD that is true for the exact same assignments to variables
  ///              in the given domain.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_from(const zdd &A, ForwardIt begin, ForwardIt end)
  { return bdd_from(A, make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given
  ///        ZDD within the given domain.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  __bdd bdd_from(const exec_policy &ep,
                 const zdd &A,
                 ForwardIt begin,
                 ForwardIt end)
  { return bdd_from(ep, A, make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the BDD that represents the same function/set as the
  ///            given ZDD within the global domain.
  ///
  /// \param A   Family of a set (within the given global \ref module__domain)
  ///
  /// \returns   BDD that is true for the exact same assignments to variables in
  ///            the global domain.
  ///
  /// \see       domain_set domain_isset
  ///
  /// \pre       The global \ref module__domain is set to a set of variables
  ///            that is equals to or a superset of the variables in `A`.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const zdd &A);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given
  ///        ZDD within the global domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const exec_policy &ep, const zdd &A);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \name DOT Files of BDDs
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a BDD to the given output stream.
  //////////////////////////////////////////////////////////////////////////////
  void bdd_printdot(const bdd &f, std::ostream &out = std::cout);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a BDD to the file with the given name.
  //////////////////////////////////////////////////////////////////////////////
  void bdd_printdot(const bdd &f, const std::string &file_name);

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_BDD_H
