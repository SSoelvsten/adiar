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

#include "negate.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/assert.h>

namespace adiar
{
  bdd bdd_not(const bdd &in)
  {
    bdd out = in;
    out.negate = !in.negate;
    return out;
  }

  bdd bdd_not(bdd &&b)
  {
    b.negate = !b.negate;
    return b;
  }
}
