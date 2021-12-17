////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#ifndef ADIAR_BDD_QUANTIFY_H
#define ADIAR_BDD_QUANTIFY_H

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
  __bdd bdd_exists(bdd &&bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an BDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &bdd, const label_t &label);

  __bdd bdd_forall(const bdd &bdd, const label_file &labels);
  __bdd bdd_forall(bdd &&bdd, const label_file &labels);
}

#endif // ADIAR_BDD_QUANTIFY_H
