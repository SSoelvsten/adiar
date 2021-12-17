////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
