#ifndef COOM_CPP
#define COOM_CPP

#include "coom.h"

#include <tpie/tempname.h>

namespace coom
{
  void coom_init(size_t memory_limit_mb, std::string temp_dir)
  {
    tpie::tpie_init();

    // Memory usage
    set_limit(memory_limit_mb);

    // Temporary files
    tpie::tempname::set_default_base_name("COOM");
    tpie::tempname::set_default_extension("coom"); // does this do anything?

    if (temp_dir != "") {
      tpie::tempname::set_default_path(temp_dir);
    }
  }

  void set_limit(size_t memory_limit_mb)
  {
    coom_assert(memory_limit_mb >= 128, "COOM requires at least 128 MB of memory");
    tpie::get_memory_manager().set_limit(memory_limit_mb * 1024 * 1024);
  }

  void coom_deinit()
  {
    tpie::tpie_finish();
  }
}


#endif // COOM_CPP
