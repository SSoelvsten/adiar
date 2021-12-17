/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "build.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/build.h>

#include <adiar/assert.h>

namespace adiar
{
  zdd zdd_sink(bool value)
  {
    return build_sink(value);
  }

  zdd zdd_empty()
  {
    return build_sink(false);
  }

  zdd zdd_null()
  {
    return build_sink(true);
  }

  zdd zdd_ithvar(label_t label)
  {
    adiar_assert(label <= MAX_LABEL, "Cannot represent that large a label");

    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_node(label, MAX_ID,
                               create_sink_ptr(false),
                               create_sink_ptr(true)));
    nw.unsafe_push(create_meta(label,1u));
    return nf;
  }

  zdd zdd_vars(const label_file &labels)
  {
    return build_chain<true, false, true>(labels);
  }

  zdd zdd_singletons(const label_file &labels)
  {
    return build_chain<false, true, false>(labels);
  }

  zdd zdd_powerset(const label_file &labels)
  {
    return build_chain<true, true, true, true>(labels);
  }
}
