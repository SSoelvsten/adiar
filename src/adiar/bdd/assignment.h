////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#ifndef ADIAR_BDD_ASSIGNMENT_H
#define ADIAR_BDD_ASSIGNMENT_H

#include <adiar/file.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically smallest x such that f(x) = 1.
  ///
  /// \param f A non-sink BDD.
  ///
  /// \return An assignment_file containing the lexicographically smallest
  /// assignment for the variables with a populated level in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmin(const bdd &f);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically largest x such that f(x) = 1.
  ///
  /// The returned assignment file will have a value for every populated level
  /// in the BDD.
  //////////////////////////////////////////////////////////////////////////////
  assignment_file bdd_satmax(const bdd &f);
}

#endif // ADIAR_BDD_ASSIGNMENT_H
