#ifndef COOM_UTIL_H
#define COOM_UTIL_H

#include <tpie/file_stream.h>

#include <coom/data.h>

namespace coom
{
  namespace internal {
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
}

#endif // COOM_UTIL_H
