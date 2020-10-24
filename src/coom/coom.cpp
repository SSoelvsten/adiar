#ifndef COOM_CPP
#define COOM_CPP

#include "coom.h"

namespace coom
{
  void coom_init(size_t memory_limit_mb)
  {
    tpie::tpie_init();
    set_limit(memory_limit_mb);
  }

  void set_limit(size_t memory_limit_mb)
  {
    coom_assert(memory_limit_mb > 512, "COOM requires at least 512 MB of memory");
    tpie::get_memory_manager().set_limit(memory_limit_mb * 1024 * 1024);
  }

  void coom_deinit()
  {
    tpie::tpie_finish();
  }
}


#endif // COOM_CPP
