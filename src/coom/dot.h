#ifndef COOM_DOT_H
#define COOM_DOT_H

#include <tpie/file_stream.h>

#include "data.h"

namespace coom {
  void println_dot(tpie::file_stream<node>& nodes);
}

#endif // COOM_DOT_H
