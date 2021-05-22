#include "build.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/assert.h>

namespace adiar
{
  zdd zdd_sink(bool value)
  {
    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_sink(value));
    return nf;
  }

  zdd zdd_empty()
  {
    return zdd_sink(false);
  }

  zdd zdd_null()
  {
    return zdd_sink(true);
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
}
