#include "adiar.h"

#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/memory.h>

namespace adiar
{
  bool _adiar_initialized = false;

  void adiar_init(size_t memory_limit_bytes, std::string temp_dir)
  {
    if (_adiar_initialized) {
      std::cerr << "Adiar has already been initialized!" << std::endl;
      return;
    }
    _adiar_initialized = true;

    adiar_statsreset();

    // Initialise TPIE
    tpie::tpie_init();

    // External Memory
    // - Temporary files and their directory
    tpie::tempname::set_default_base_name("ADIAR");
    tpie::tempname::set_default_extension("adiar");

    if (temp_dir != "") {
      tpie::tempname::set_default_path(temp_dir);
    }

    // - Block size
    internal::set_block_size(internal::recommended_block_size(memory_limit_bytes));

    // Internal Memory
    // - Memory Mode
    memory_mode = memory_mode_t::AUTO;

    // - Amount of internal memory
    internal::memory_set_limit(memory_limit_bytes);
  }

  bool adiar_initialized() noexcept
  {
    return _adiar_initialized;
  }

  void adiar_deinit()
  {
    if (_adiar_initialized) tpie::tpie_finish();
    _adiar_initialized = false;
  }
}
