#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "negate.h"

namespace coom
{
  // TODO: One should notice, that this literally just is a negation-pipe on a
  //       stream. We can completely hide this with in the pipelining network
  //       or with a Decorator Pattern on the TPIE file_stream.
  void negate(tpie::file_stream<node> &in_nodes,
              tpie::file_stream<node> &out_nodes)
  {
#if COOM_ASSERT
    assert(in_nodes.size() > 0);
    assert(out_nodes.size() > 0);
#endif
    in_nodes.seek(0);

    if (is_sink_node(in_nodes.peek()))
      {
        out_nodes.write(create_sink_node(!value_of(in_nodes.read())));
        return;
      }

    while (in_nodes.can_read())
      {
        node n = in_nodes.read();

        uint64_t low = is_sink(n.low)
          ? create_sink(!value_of(n.low))
          : n.low;

        uint64_t high = is_sink(n.high)
          ? create_sink(!value_of(n.high))
          : n.high;

        out_nodes.write({ n.node_ptr, low, high });
      }
  }
}

#endif // COOM_NEGATE_CPP
