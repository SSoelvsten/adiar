#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include "negate.h"

#include <coom/assert.h>

namespace coom
{
  // TODO: One should notice, that this literally just is a negation-pipe on a
  //       stream. We can completely hide this with in the pipelining network
  //       or with a Decorator Pattern on the TPIE file_stream.
  void negate(tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_input_stream(in_nodes);
    assert::is_valid_output_stream(out_nodes);

    in_nodes.seek(0);

    if (is_sink(in_nodes.peek())) {
      out_nodes.write(create_sink(!value_of(in_nodes.read())));
      return;
    }

    while (in_nodes.can_read()) {
      node_t n = in_nodes.read();

      ptr_t low_ptr = is_sink_ptr(n.low)
        ? create_sink_ptr(!value_of(n.low))
        : n.low;

      ptr_t high_ptr = is_sink_ptr(n.high)
        ? create_sink_ptr(!value_of(n.high))
        : n.high;

      out_nodes.write({ n.uid, low_ptr, high_ptr });
    }
  }
}

#endif // COOM_NEGATE_CPP
