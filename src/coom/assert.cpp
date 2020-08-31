#ifndef COOM_ASSERT_CPP
#define COOM_ASSERT_CPP

#include "assert.h"

namespace coom { namespace assert {
    void is_valid_input_stream([[maybe_unused]] tpie::file_stream<node_t>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
      assert(in.size() > 0);
#endif
    }

    void is_valid_input_stream([[maybe_unused]] tpie::file_stream<arc_t>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
#endif
    }

    template <typename T>
    void is_valid_output_stream([[maybe_unused]] tpie::file_stream<T>& out)
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
