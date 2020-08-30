#ifndef COOM_PRED_CPP
#define COOM_PRED_CPP

#include <stdint.h>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include "data.h"
#include "assert.h"

#include "pred.h"

namespace coom {
  bool is_sink(tpie::file_stream<node>& nodes,
               const sink_pred& sink_pred = is_any)
  {
    assert::is_valid_input_stream(nodes);
    if (nodes.size() != 1) {
      return false;
    }
    node n = nodes.can_read() ? nodes.read() : nodes.read_back();
    return is_sink(n) && sink_pred(n.uid);
  }
}

#endif // COOM_PRED_CPP
