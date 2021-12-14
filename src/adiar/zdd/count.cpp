/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

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
