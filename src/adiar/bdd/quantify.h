#ifndef ADIAR_QUANTIFY_H
#define ADIAR_QUANTIFY_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &bdd, const label_t &label);

  __bdd bdd_exists(const bdd &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &bdd, const label_t &label);

  __bdd bdd_forall(const bdd &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Uniquely quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_unique(const bdd &bdd, const label_t &label);

  __bdd bdd_unique(const bdd &bdd, const label_file &labels);
}

#endif // ADIAR_QUANTIFY_H
