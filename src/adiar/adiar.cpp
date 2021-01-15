#ifndef ADIAR_CPP
#define ADIAR_CPP

#include "adiar.h"

#include <tpie/tempname.h>

namespace adiar
{
  void adiar_init(size_t memory_limit_mb, std::string temp_dir)
  {
    tpie::tpie_init();

    // Memory usage
    set_limit(memory_limit_mb);

    // Temporary files
    tpie::tempname::set_default_base_name("ADIAR");
    tpie::tempname::set_default_extension("adiar"); // does this do anything?

    if (temp_dir != "") {
      tpie::tempname::set_default_path(temp_dir);
    }
  }

  void set_limit(size_t memory_limit_mb)
  {
    adiar_assert(memory_limit_mb >= 128, "ADIAR requires at least 128 MB of memory");
    tpie::get_memory_manager().set_limit(memory_limit_mb * 1024 * 1024);
  }

  void adiar_deinit()
  {
    tpie::tpie_finish();
  }
}


#endif // ADIAR_CPP
