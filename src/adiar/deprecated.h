#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

namespace adiar
{
  // LCOV_EXCL_START

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Existential quantification of multiple variables.
  ///
  /// \details    Repeatedly calls `bdd_exists` for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified.
  ///
  /// \param vars Variables to quantify in f (in order of their quantification)
  ///
  /// \returns    \f$ \exists x_{i_1}, \dots, x_{i_k} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced with using 'predicates', 'generators' etc. for multiple variables")]]
    __bdd bdd_exists(const bdd &f, const shared_file<bdd::label_t> &vars);

  /// \cond
  [[deprecated("Replaced with using 'predicates', 'generators' etc. for multiple variables")]]
    __bdd bdd_exists(bdd &&f, const shared_file<bdd::label_t> &vars);
  /// \endcond


  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Forall quantification of multiple variables.
  ///
  /// \details    Repeatedly calls `bdd_forall` for the given variables
  ///             while optimising garbage collecting intermediate results.
  ///
  /// \param f    BDD to be quantified.
  ///
  /// \param vars Variables to quantify in f (in order of their quantification)
  ///
  /// \returns    \f$ \forall x_{i_1}, \dots, x_{i_k} : f \f$
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced with using 'predicates', 'generators' etc. for multiple variables")]]
  __bdd bdd_forall(const bdd &f, const shared_file<bdd::label_t> &vars);

  /// \cond
  [[deprecated("Replaced with using 'predicates', 'generators' etc. for multiple variables")]]
  __bdd bdd_forall(bdd &&f, const shared_file<bdd::label_t> &vars);
  /// \endcond

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
  [[deprecated("Replaced with using 'predicates' for multiple variables")]]
  __zdd zdd_project(const zdd &A, const shared_file<zdd::label_t> &dom);

  /// \cond
  [[deprecated("Replaced with using 'predicates' for multiple variables")]]
  __zdd zdd_project(zdd &&A, const shared_file<zdd::label_t> &dom);
  /// \endcond

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
