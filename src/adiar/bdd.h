#ifndef ADIAR_BDD_H
#define ADIAR_BDD_H

#include <string>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  /* =========================== BDD CONSTRUCTION =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing a constant value.
  ///
  /// \param value The constant sink value
  ///
  /// \sa          bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_sink(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant false BDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_false();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant true BDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_true();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The BDD representing the i'th variable.
  ///
  /// \details   Creates a BDD of a single node with label <tt>var</tt> and the
  ///            children false and true. The given label must be smaller than
  ///            <tt>MAX_LABEL</tt>.
  ///
  /// \param var The label of the desired variable
  ///
  /// \returns   \f$ x_{var} \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_ithvar(label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     The BDD representing the negation of the i'th variable.
  ///
  /// \details   Creates a BDD of a single node with label <tt>var</tt> and the
  ///            children true and false. The given label must be smaller than
  ///            or equal to <tt>MAX_LABEL</tt>.
  ///
  /// \param var Label of the desired variable
  ///
  /// \returns   \f$ \neg x_{var} \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_nithvar(label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'and' of all the given
  ///             variables.
  ///
  /// \details    Creates a BDD with a chain of nodes on the 'high' arc to the
  ///             true child, and false otherwise. The given labels must be
  ///             smaller than or equal to <tt>MAX_LABEL</tt>.
  ///
  /// \param vars Labels of the desired variables (in ascending order)
  ///
  /// \returns    \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_and(const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'or' of all the given
  ///             variables.
  ///
  /// \details    Creates a BDD with a chain of nodes on the 'low' arc to the
  ///             true child, and false otherwise. The given labels must be
  ///             smaller than or equal to <tt>MAX_LABEL</tt>.
  ///
  /// \param vars Labels of the desired variables (in ascending order)
  ///
  /// \returns    \f$ \bigvee_{x \in \mathit{vars}} x \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_or(const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief           The BDD representing the function that is true exactly if
  ///                  a certain number of variables in an interval are true.
  ///
  /// \param min_var   The minimum label (inclusive) to start counting from
  /// \param max_var   The maximum label (inclusive) to end counting at
  /// \param threshold The threshold number of variables set to true
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_counter(label_t min_var, label_t max_var, label_t threshold);

  /* =========================== BDD MANIPULATION =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Negation of a BDD.
  ///
  /// \details Flips the negation flag such that reading nodes with
  ///          <tt>node_stream<></tt> will on-the-fly change the false sink into
  ///          the true sink.
  ///
  /// \returns \f$ \neg f \f$
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_not(const bdd &f);
  bdd bdd_not(bdd &&f);

  bdd operator~ (const bdd& f);
  bdd operator~ (__bdd&& f);
  bdd operator~ (bdd&& f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Apply a binary operator between two BDDs.
  ///
  /// \param f  BDD for the left-hand-side of the operator
  /// \param g  BDD for the right-hand-side of the operator
  /// \param op Binary boolean operator to be applied. See 'adiar/data.h'
  ///
  /// \returns  The product construction of <tt>f</tt> <tt>op</tt> <tt>g</tt>.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &f, const bdd &g, const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'and' operator.
  ///
  /// \returns \f$ f \land g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_and(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, and_op); }

  __bdd operator& (const bdd &lhs, const bdd &rhs);
  __bdd operator& (const bdd &, __bdd &&);
  __bdd operator& (__bdd &&, const bdd &);
  __bdd operator& (__bdd &&, __bdd &&);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nand' operator.
  ///
  /// \returns \f$ \neg (f \land g) \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_nand(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, nand_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'or' operator.
  ///
  /// \returns \f$ f \lor g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_or(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, or_op); };

  __bdd operator| (const bdd &lhs, const bdd &rhs);
  __bdd operator| (const bdd &, __bdd &&);
  __bdd operator| (__bdd &&, const bdd &);
  __bdd operator| (__bdd &&, __bdd &&);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nor' operator.
  ///
  /// \returns \f$ \neg (f \lor g) \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_nor(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, nor_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xor' operator.
  ///
  /// \returns \f$ f \oplus g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_xor(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, xor_op); };

  __bdd operator^ (const bdd &lhs, const bdd &rhs);
  __bdd operator^ (const bdd &, __bdd &&);
  __bdd operator^ (__bdd &&, const bdd &);
  __bdd operator^ (__bdd &&, __bdd &&);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xnor' operator.
  ///
  /// \returns \f$ \neg (f \oplus g) \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_xnor(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, xnor_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'implication' operator.
  ///
  /// \returns \f$ f \implies g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_imp(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, imp_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'inverse implication' operator.
  ///
  /// \returns \f$ f \impliedby g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_invimp(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, invimp_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'equivalence' operator.
  ///
  /// \returns \f$ f = g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_equiv(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, equiv_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'difference'  operator.
  ///
  /// \returns \f$ f \setminus g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_diff(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, diff_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'less than' operator.
  ///
  /// \returns \f$ f < g \f$
  //////////////////////////////////////////////////////////////////////////////
  inline __bdd bdd_less(const bdd &f, const bdd &g)
  { return bdd_apply(f, g, less_op); };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   If-Then-Else operator.
  ///
  /// \details Computes the BDD expressing \f$ f ? g : h \f$ more efficient than
  ///          computing \f$ (f \land g) \lor (\neg f \land h) \f$ with
  ///          <tt>bdd_apply</tt>.
  ///
  /// \param f BDD for the if conditional
  ///
  /// \param g BDD for the then case
  ///
  /// \param h BDD for the else case
  ///
  /// \returns \f$ f ? g : h \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_ite(const bdd &f, const bdd &g, const bdd &h);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Restrict a subset of variables to constant values.
  ///
  /// \details  For each tuple (i,v) in the assignment <tt>xs</tt> the variable
  ///           with label i is set to the constant value v. This binds the
  ///           scope of the variables in <tt>xs</tt>, i.e. any later mention of
  ///           a variable i is not the same as variable i in <tt>xs</tt>.
  ///
  /// \param f  BDD to restrict
  ///
  /// \param xs Assignments (i,v) to variables in (in ascending order)
  ///
  /// \returns  \f$ f|_{(i,v) \in xs : x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_restrict(const bdd &f, const assignment_file &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Existentially quantify a single variable.
  ///
  /// \details   Computes the BDD for \f$ \exists x_{i} : f \f$ faster than
  ///            computing \f$ f|_{x_i = \bot} \lor f|_{x_i = \top} \f$ using
  ///            <tt>bdd_apply</tt> and <tt>bdd_restrict</tt>.
  ///
  /// \param f   BDD to be quantified
  /// \param var Variable to quantify in f
  ///
  /// \returns   \f$ \exists x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &f, label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Existentially quantify multiple variables.
  ///
  /// \details    Repeatedly calls <tt>bdd_exists</tt> for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified
  /// \param vars Variables to quantify in f (in order of their quantification)
  ///
  /// \returns    \f$ \exists x_{i_1}, \dots, x_{i_k} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &f, const label_file &vars);
  __bdd bdd_exists(bdd &&f, const label_file &vars);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Forall quantify a single variable.
  ///
  /// \details   Computes the BDD for \f$ \forall x_{i} : f \f$ faster than
  ///            computing \f$ f|_{x_i = \bot} \land f|_{x_i = \top} \f$ using
  ///            <tt>bdd_apply</tt> and <tt>bdd_restrict</tt>.
  ///
  /// \param f   BDD to be quantified
  /// \param var Variable to quantify in f
  ///
  /// \returns   \f$ \forall x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &f, label_t var);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Forall quantify multiple variables.
  ///
  /// \details    Repeatedly calls <tt>bdd_exists</tt> for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified
  /// \param vars Variables to quantify in f (in order of their quantification)
  ///
  /// \returns    \f$ \forall x_{i_1}, \dots, x_{i_k} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &f, const label_file &vars);
  __bdd bdd_forall(bdd &&f, const label_file &vars);

  __bdd bdd_reorder(const bdd &dd, const std::vector<label_t> permutation);
  

  /* ============================ BDD PREDICATES ============================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether f and g represent the same function.
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_equal(const bdd& f, const bdd& g);

  bool operator== (const bdd& f, const bdd& g);
  bool operator== (__bdd &&f, const bdd &g);
  bool operator== (const bdd &f, __bdd &&g);
  bool operator== (__bdd &&f, __bdd &&g);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether f and g represent different functions.
  //////////////////////////////////////////////////////////////////////////////
  inline bool bdd_unequal(const bdd& f, const bdd& g) 
    { return !bdd_equal(f, g); }

  bool operator!= (const bdd& f, const bdd& g);
  bool operator!= (const bdd &f, __bdd &&g);
  bool operator!= (__bdd &&f, const bdd &g);
  bool operator!= (__bdd &&f, __bdd &&g);

  /* ============================= BDD COUNTING ============================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of (internal) nodes used to represent the function.
  //////////////////////////////////////////////////////////////////////////////
  size_t bdd_nodecount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of variables that influence the outcome of f, i.e. the
  ///        number of levels in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  label_t bdd_varcount(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to the true
  ///        sink.
  ///
  /// \returns The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_pathcount(const bdd &f);

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
  //////////////////////////////////////////////////////////////////////////////
  uint64_t bdd_satcount(const bdd &f, label_t varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  ///
  /// \details Same as <tt>bdd_satcount(bdd, varcount)</tt>, with varcount set
  ///          to be <tt>varcount(bdd)</tt>.
  //////////////////////////////////////////////////////////////////////////////
  inline uint64_t bdd_satcount(const bdd &f)
  { return bdd_satcount(f, bdd_varcount(f)); };

  /* ========================= BDD  INPUT VARIABLES ========================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically smallest x such that f(x) is true.
  ///
  /// \details Outputs the trace of the low-most path to the true sink. The
  ///          resulting assignment is lexicographically smallest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns An <tt>assignment_file</tt> with an assignment to every variable
  ///          mentioned by the given BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmin(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically largest x such that f(x) is true.
  ///
  /// \details Outputs the trace of the high-most path to the true sink. The
  ///          resulting assignment is lexicographically largest, where every
  ///          variable is treated as a digit and \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns An <tt>assignment_file</tt> with an assignment to every variable
  ///          mentioned by the given BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmax(const bdd &f);

  typedef std::function<bool(label_t)> assignment_func;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Evaluate a BDD according to an assignment
  ///
  /// \details  The given assignment function may assume/abuse that it is only
  ///           called with the labels in a strictly increasing order.
  ///
  /// \param f  The BDD to evaluate
  ///
  /// \param xs An assignment function of the type <tt>label_t -> bool</tt>
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &f, const assignment_func &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Evaluate a BDD according to an assignment
  ///
  /// \param f  The BDD to evaluate
  ///
  /// \param xs A list of tuples<tt>(i,v)</tt> in ascending order
  //////////////////////////////////////////////////////////////////////////////
  bool bdd_eval(const bdd &f, const assignment_file &xs);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief    Get the labels of the levels of the BDD
  //////////////////////////////////////////////////////////////////////////////
  label_file bdd_varprofile(const bdd &f);

  /* ============================== CONVERSION ============================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the BDD that represents the same function/set as the
  ///            given ZDD within the given domain.
  ///
  /// \param A   Family of a set (within the given domain)
  ///
  /// \param dom Domain of all variables (in ascending order)
  ///
  /// \returns   BDD that is true for the exact same assignments to variables in
  ///            the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const zdd &A, const label_file &dom);

  /* ================================= DEBUG ================================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing a decision diagram to an output stream or a
  ///        file with the given file name.
  //////////////////////////////////////////////////////////////////////////////
  void output_dot(const bdd &f, const std::string &file_name);
}

#endif // ADIAR_BDD_H
