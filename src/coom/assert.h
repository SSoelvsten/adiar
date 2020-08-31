#ifndef COOM_ASSERT_H
#define COOM_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include "data.h"

namespace coom { namespace assert {
    void is_valid_input_stream(tpie::file_stream<node_t>& in);
    void is_valid_input_stream(tpie::file_stream<arc_t>& in);
    template <typename T> void is_valid_output_stream(tpie::file_stream<T>& out);
  }
}

#endif // COOM_ASSERT_H
