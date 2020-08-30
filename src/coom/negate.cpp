#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "debug.h"
#include "debug_data.h"

#include "assert.h"

#include "negate.h"

namespace coom
{
  // TODO: One should notice, that this literally just is a negation-pipe on a
  //       stream. We can completely hide this with in the pipelining network
  //       or with a Decorator Pattern on the TPIE file_stream.
  void negate(tpie::file_stream<node> &in_nodes,
              tpie::file_stream<node> &out_nodes)
  {
    debug::println_algorithm_start("NEGATE");

    assert::is_valid_input_stream(in_nodes);
    debug::println_file_stream(in_nodes, "in_nodes");

    assert::is_valid_output_stream(out_nodes);

    in_nodes.seek(0);

    if (is_sink(in_nodes.peek())) {
      out_nodes.write(create_sink(!value_of(in_nodes.read())));
      return;
    }

    while (in_nodes.can_read()) {
      node n = in_nodes.read();

      ptr_t low_ptr = is_sink_ptr(n.low)
        ? create_sink_ptr(!value_of(n.low))
        : n.low;

      ptr_t high_ptr = is_sink_ptr(n.high)
        ? create_sink_ptr(!value_of(n.high))
        : n.high;

      out_nodes.write({ n.uid, low_ptr, high_ptr });
    }

    debug::println_file_stream(out_nodes, "out_nodes");
    debug::println_algorithm_end("NEGATE");
  }
}

#endif // COOM_NEGATE_CPP
