#include "file_writer.h"

namespace adiar {
  node_writer& operator<< (node_writer &nw, const node_t &n)
  {
    nw.push(n);
    return nw;
  }
}
