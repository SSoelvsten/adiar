#ifndef ADIAR_FILE_WRITER_CPP
#define ADIAR_FILE_WRITER_CPP

#include "file_writer.h"

namespace adiar {
  node_writer& operator<< (node_writer &nw, const node_t &n)
  {
    nw.push(n);
    return nw;
  }
}

#endif // ADIAR_FILE_WRITER_CPP
