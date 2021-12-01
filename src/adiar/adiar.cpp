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

  void adiar_init(size_t memory_limit_bytes, std::string temp_dir)
  {
    tpie::tpie_init(TPIE_SUBSYSTEMS);

    // Memory management
    memory::init(temp_dir);
    memory::set_limit(memory_limit_bytes);
  }

  void adiar_deinit()
  {
    tpie::tpie_finish(TPIE_SUBSYSTEMS);
  }
}
