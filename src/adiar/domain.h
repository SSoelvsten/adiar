#ifndef ADIAR_DOMAIN_H
#define ADIAR_DOMAIN_H

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__domain Variable Domain
///
/// \brief Global domain of variables.
///
/// Some operations relate to the entire variable domain. Instead of passing
/// this around explicitly, you may set it once and then Adiar will take care of
/// using it when needed.
////////////////////////////////////////////////////////////////////////////////

#include <adiar/exception.h>
#include <adiar/file.h>
#include <adiar/functional.h>

// TODO: Make 'domain_var_t' independent of node type. Then remove this include.
#include <adiar/internal/data_types/node.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__domain
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The variable type of a domain variable.
  //////////////////////////////////////////////////////////////////////////////
  using domain_var_t = internal::node::label_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximum supported domain variable.
  //////////////////////////////////////////////////////////////////////////////
  constexpr domain_var_t MAX_DOMAIN_VAR = internal::node::max_label;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Set the domain globally for all of Adiar to be [*0, varcount*).
  //////////////////////////////////////////////////////////////////////////////
  void adiar_set_domain(const domain_var_t varcount);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Set the domain globally for all of Adiar to be the variables
  ///        produced by the given generator function.
  ///
  /// \param dom Generator that produces variables to be quantified in
  ///            *ascending* order. When none are left to-be quantified, it must
  ///            return a value greater than `MAX_DOMAIN_VAR`.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_set_domain(const generator<domain_var_t> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Set the domain globally for all of Adiar to be the variables
  ///        in the given range of iterators.
  ///
  /// \param begin Iterator that provides variables to be quantified in
  ///              *ascending* order.
  ///
  /// \param end   Iterator that marks the end for `begin`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  void adiar_set_domain(IT begin, IT end)
  { return adiar_set_domain(make_generator(begin, end)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Set the domain globally for all of Adiar.
  ///
  /// \param dom A \ref shared_file containing all labels of the problem domain
  ///            in ascending order.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_set_domain(const shared_file<domain_var_t> &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Removes any globally shared domain variables (if any).
  ///
  /// \sa adiar_set_domain
  //////////////////////////////////////////////////////////////////////////////
  void adiar_unset_domain();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar has a global domain.
  //////////////////////////////////////////////////////////////////////////////
  bool adiar_has_domain();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Returns the global domain if `adiar_has_domain() == true`.
  ///
  /// \throws domain_error If no domain is yet set, i.e.
  ///         `adiar_has_domain() == false`.
  //////////////////////////////////////////////////////////////////////////////
  shared_file<domain_var_t> adiar_get_domain();

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_DOMAIN_H
