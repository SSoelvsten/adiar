#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include "negate.h"

#include <coom/assert.h>

namespace coom
{
  node_file bdd_not(const node_file &in)
  {
    node_file out;
    node_writer out_writer(out);

    file_stream<meta_t, false> meta_stream(in._meta_file);
    while (meta_stream.can_pull()) {
      out_writer.unsafe_push(meta_stream.pull());
    }

    file_stream<node_t, false> node_stream(in._files[0]);
    while (node_stream.can_pull()) {
      out_writer.unsafe_push(!node_stream.pull());
    }

    return out;
  }
}

#endif // COOM_NEGATE_CPP
