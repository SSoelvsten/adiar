////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
