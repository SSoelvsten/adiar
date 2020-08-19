#ifndef COOM_DEBUG_ASSIGNMENT_CPP
#define COOM_DEBUG_ASSIGNMENT_CPP

#include <tpie/file_stream.h>

#include "debug_assignment.h"

namespace coom { namespace debug {
    inline void println_file_stream([[maybe_unused]] tpie::file_stream<assignment> &assignment,
                                    [[maybe_unused]] std::string name)
    {
#if COOM_DEBUG
      auto original_pos = assignment.get_position();
      assignment.seek(0);

      tpie::log_info() << name << " {" << std::endl;
      while (assignment.can_read()) {
        auto a = assignment.read();
        tpie::log_info() << "\t" << a.label
                         << " -> " << a.value << std::endl;
      }
      tpie::log_info() << "}" << std::endl;

      assignment.set_position(original_pos);
#endif
    }

    inline void println_file_stream([[maybe_unused]] tpie::file_stream<assignment> &assignment)
    {
      println_file_stream(assignment, "file_stream<assignment>");
    }
  }
}

#endif // COOM_DEBUG_ASSIGNMENT_CPP
