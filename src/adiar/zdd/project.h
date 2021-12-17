////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#ifndef ADIAR_ZDD_PROJECT_H
#define ADIAR_ZDD_PROJECT_H

#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given a ZDD projects it to a subset of the variables.
  ///
  /// \param zdd  ZDD to apply with the other.
  ///
  /// \param dom  Domain
  ///
  /// \return     A ZDD that represents the same sets as in zdd, but with only
  ///             the variables in dom.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_project(const zdd &dd, const label_file &dom);

  zdd zdd_project(zdd &&dd, const label_file &dom);
}

#endif // ADIAR_ZDD_PROJECT_H
