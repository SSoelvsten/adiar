////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#include "count.h"

#include <adiar/data.h>
#include <adiar/internal/count.h>

#include <adiar/assert.h>

namespace adiar
{
  size_t zdd_nodecount(const zdd &zdd)
  {
    return nodecount(zdd.file);
  }

  label_t zdd_varcount(const zdd &zdd)
  {
    return varcount(zdd.file);
  }

  uint64_t zdd_size(const zdd &zdd)
  {
    return is_sink(zdd)
      ? is_sink(zdd, is_true)
      : count<path_count_policy, path_sum>(zdd, zdd_varcount(zdd));
  }
}
