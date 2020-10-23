#ifndef COOM_ASSERT_CPP
#define COOM_ASSERT_CPP

#include "assert.h"

namespace coom { namespace assert {
    template<>
    void is_valid_input_stream<node_t>([[maybe_unused]] tpie::file_stream<node_t>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
      assert(in.size() > 0);
#endif
    }

    template<>
    void is_valid_input_stream<arc_t>([[maybe_unused]] tpie::file_stream<arc_t>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
#endif
    }

    template<>
    void is_valid_input_stream<meta_t>([[maybe_unused]] tpie::file_stream<meta_t>& in)
    {
#if COOM_ASSERT
      assert(in.is_open());
      assert(in.is_readable());
#endif
    }
  }
}

#endif // COOM_ASSERT_CPP
