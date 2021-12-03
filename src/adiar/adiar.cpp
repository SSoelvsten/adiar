#include "adiar.h"

#include <tpie/tpie.h>

#include <adiar/internal/memory.h>

#include <adiar/assert.h>

namespace adiar
{
  const tpie::flags<tpie::subsystem> TPIE_SUBSYSTEMS =
    tpie::MEMORY_MANAGER
    | tpie::DEFAULT_LOGGING
    | tpie::PROGRESS
    // | tpie::JOB_MANAGER
    // | tpie::CAPTURE_FRACTIONS
    | tpie::STREAMS
    | tpie::TEMPFILE
    | tpie::FILE_MANAGER;

  bool _adiar_initialized = false;

  void adiar_init(size_t memory_limit_bytes, std::string temp_dir)
  {
    if (_adiar_initialized) {
      std::cerr << "Adiar has already been initialized!" << std::endl;
      return;
    }
    _adiar_initialized = true;

    tpie::tpie_init(TPIE_SUBSYSTEMS);

    // Memory management
    memory::init(temp_dir);
    memory::set_limit(memory_limit_bytes);
  }

  bool adiar_initialized()
  {
    return _adiar_initialized;
  }

  void adiar_deinit()
  {
    if (_adiar_initialized) tpie::tpie_finish(TPIE_SUBSYSTEMS);
    _adiar_initialized = false;
  }
}
