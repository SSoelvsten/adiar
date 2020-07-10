#ifndef COOM_DOT_H
#define COOM_DOT_H

#include <tpie/file_stream.h>

#include "data.h"

namespace coom {
  void output_dot(tpie::file_stream<node>& nodes,
                  std::string filename);
}

#endif // COOM_DOT_H
