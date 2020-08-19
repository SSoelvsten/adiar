#ifndef COOM_DEBUG_ASSIGNMENT_H
#define COOM_DEBUG_ASSIGNMENT_H

#include <tpie/file_stream.h>

#include "assignment.h"

namespace coom { namespace debug {
    void println_file_stream(tpie::file_stream<assignment> &in_assignment,
                             std::string name);
    void println_file_stream(tpie::file_stream<assignment> &in_assignment);
  }
}

#endif // COOM_DEBUG_ASSIGNMENT_H
