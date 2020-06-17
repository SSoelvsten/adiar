#ifndef COOM_EVALUATE_CPP
#define COOM_EVALUATE_CPP

#include <tpie/file_stream.h>
#include "data.h"

#include "evaluate.h"

namespace coom
{
  bool evaluate(tpie::file_stream<node>* nodes_ptr,
                tpie::file_stream<bool>* assignment_ptr)
  {
    auto nodes = *nodes_ptr;
    auto assignment = *assignment_ptr;

    return false;
  }
}

#endif // COOM_EVALUATE_CPP
