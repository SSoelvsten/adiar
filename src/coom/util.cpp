#ifndef COOM_UTIL_CPP
#define COOM_UTIL_CPP

#include <tpie/file_stream.h>

#include "data.h"

namespace coom
{
  template<typename T>
  inline void copy(tpie::file_stream<T> &in,
                   tpie::file_stream<T> &out)
  {
    in.seek(0);
    while(in.can_read()) {
      out.write(in.read());
    }
  }
}

#endif // COOM_UTIL_CPP
