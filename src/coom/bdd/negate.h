#ifndef COOM_NEGATE_H
#define COOM_NEGATE_H

#include <tpie/file_stream.h>
#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate a given node-based OBDD.
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_not(const node_file &f);
}

#endif // COOM_NEGATE_H
