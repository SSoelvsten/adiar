#ifndef COOM_DEBUG_CPP
#define COOM_DEBUG_CPP

#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

#include "debug.h"

namespace coom { namespace debug {
    void println_algorithm_start(const std::string algo_name)
    {
#if COOM_DEBUG
      tpie::log_info() << "//===\\\\ " << algo_name << " //===\\\\" << std::endl;
#endif
    }

    void println_algorithm_end(const std::string algo_name)
    {
#if COOM_DEBUG
      tpie::log_info() << "\\\\===// " << algo_name << " \\\\===//" << std::endl;
#endif
    }
  }
}

#endif // COOM_DEBUG_CPP
