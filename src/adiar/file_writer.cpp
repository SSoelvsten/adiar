////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#include "file_writer.h"

namespace adiar {
  node_writer& operator<< (node_writer &nw, const node_t &n)
  {
    nw.push(n);
    return nw;
  }
}
