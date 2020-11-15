#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include "negate.h"

#include <coom/file_stream.h>
#include <coom/file_writer.h>

#include <coom/assert.h>

namespace coom
{
  node_file bdd_not(const node_file &in)
  {
    node_file out;
    node_writer out_writer(out);

    meta_stream<node_t, 1, true> meta_stream(in);
    while (meta_stream.can_pull()) {
      out_writer.unsafe_push(meta_stream.pull());
    }

    node_stream<true> node_stream(in);
    while (node_stream.can_pull()) {
      out_writer.unsafe_push(!node_stream.pull());
    }

    return out;
  }
}

#endif // COOM_NEGATE_CPP
