#include "adiar.h"

#include <tpie/tpie.h>

#include <adiar/internal/assert.h>
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

    // Memory management
    memory::set_path(temp_dir);
    memory::set_limit(memory_limit_bytes);
    memory::set_block_size(memory::recommended_block_size(memory_limit_bytes));
  }

  bool adiar_initialized()
  {
    return _adiar_initialized;
  }

  void adiar_deinit()
  {
    if (_adiar_initialized) tpie::tpie_finish();
    _adiar_initialized = false;
  }
}
