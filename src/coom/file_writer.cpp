#ifndef COOM_FILE_WRITER_CPP
#define COOM_FILE_WRITER_CPP

#include "file_writer.h"

namespace coom {
  node_writer& operator<< (node_writer &nw, const node_t &n)
  {
    nw.push(n);
    return nw;
  }
}

#endif // COOM_FILE_WRITER_CPP
