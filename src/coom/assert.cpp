#ifndef COOM_ASSERT_CPP
#define COOM_ASSERT_CPP

#include <stdint.h>

#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

#include "debug.h"

namespace coom { namespace assert {
    void is_valid_input_stream(tpie::file_stream<node>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
      assert(in.size() > 0);
#endif
    }

    void is_valid_input_stream(tpie::file_stream<arc>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
#endif
    }

    template <typename T> void is_valid_output_stream(tpie::file_stream<T>& out)
    {
#if COOM_ASSERT
      assert(out.is_open());
      assert(out.is_writable());
      assert(out.size() == 0);
#endif
    }
  }
}

#endif // COOM_ASSERT_CPP
