#ifndef COOM_NEGATE_CPP
#define COOM_NEGATE_CPP

#include "negate.h"

#include <coom/file_stream.h>
#include <coom/file_writer.h>

#include <coom/assert.h>

namespace coom
{
  bdd bdd_not(const bdd &in)
  {
    bdd out = in;
    out.negate = !in.negate;
    return out;
  }

  bdd bdd_not(bdd &&b)
  {
    b.negate = !b.negate;
    return b;
  }
}

#endif // COOM_NEGATE_CPP
