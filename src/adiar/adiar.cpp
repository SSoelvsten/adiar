#include "adiar.h"

#include <tpie/tempname.h>

namespace adiar
{
  void adiar_init(size_t memory_limit_bytes, std::string temp_dir)
  {
    tpie::tpie_init();

    // Memory usage
    set_limit(memory_limit_bytes);

    // Temporary files
    tpie::tempname::set_default_base_name("ADIAR");
    tpie::tempname::set_default_extension("adiar"); // does this do anything?

    if (temp_dir != "") {
      tpie::tempname::set_default_path(temp_dir);
    }
  }

  void set_limit(size_t memory_limit_bytes)
  {
    adiar_assert(memory_limit_bytes >= 128 * 1024 * 1024,
                 "ADIAR requires at least 128 MiB of memory");
    tpie::get_memory_manager().set_limit(memory_limit_bytes);
  }

  void adiar_deinit()
  {
    tpie::tpie_finish();
  }
}
