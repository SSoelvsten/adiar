#ifndef COOM_ASSERT_H
#define COOM_ASSERT_H

#include <tpie/file_stream.h>

#include <stdint.h>

#include "data.h"

namespace coom { namespace assert {
    void is_valid_input_stream(tpie::file_stream<node>& in);
    void is_valid_input_stream(tpie::file_stream<arc>& in);
    template <typename T> void is_valid_output_stream(tpie::file_stream<T>& out);
  }
}

#endif // COOM_ASSERT_H
