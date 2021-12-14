/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

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
