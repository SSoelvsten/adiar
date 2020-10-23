#ifndef COOM_ASSERT_H
#define COOM_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <coom/data.h>

namespace coom { namespace assert {
    template <typename T>
    void is_valid_input_stream([[maybe_unused]] tpie::file_stream<T>& in);

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

#endif // COOM_ASSERT_H
