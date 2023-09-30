#include "adiar.h"

#include <exception>

#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/memory.h>

namespace adiar
{
  bool _adiar_initialized = false;
  bool _tpie_initialized  = false;

  void adiar_init(size_t memory_limit_bytes, std::string temp_dir)
  {
    if (_adiar_initialized) {
#ifndef NDEBUG
      std::cerr << "Adiar has already been initialized! Skipping 'adiar_init()'" << std::endl;
#endif
      return;
    }
    if (!_adiar_initialized && _tpie_initialized) {
      throw runtime_error("Adiar cannot be initialized after call to 'adiar_deinit()'");
    }
    if (memory_limit_bytes < minimum_memory) {
      throw invalid_argument("Adiar requires at least "
                             + std::to_string(minimum_memory / 1024 / 1024)
                             + " MiB of memory");
    }

    try {
      // Initialise TPIE
      tpie::tpie_init(tpie::ALL);

      // - file names
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      // - tmp directory
      if (temp_dir != "") {
        tpie::tempname::set_default_path(temp_dir);
      }

      // - memory limit and block size
      tpie::get_memory_manager().set_limit(memory_limit_bytes);
      internal::set_block_size(internal::recommended_block_size(memory_limit_bytes));

      _tpie_initialized = true;

      // Initialise Adiar state
      // - reset statistics
      statistics_reset();

      // - reset enum settings
      access_mode   = access_mode_t::Auto;
      memory_mode   = memory_mode_t::Auto;

      // - all done, mark initialized
      _adiar_initialized = true;
    } catch (const std::exception &e) {
      // LCOV_EXCL_START
      // Mark as not initialized.
      _adiar_initialized = false;
      _tpie_initialized  = true;

      // Try to gracefully close down TPIE.
      tpie::tpie_finish(tpie::ALL);

      // Tell the user what happened.
      throw e;
      // LCOV_EXCL_STOP
    }
  }

  bool adiar_initialized() noexcept
  {
    return _adiar_initialized;
  }

  void adiar_deinit()
  {
    if (!_adiar_initialized) return;

    domain_unset();

    tpie::tpie_finish(tpie::ALL);
    _adiar_initialized = false;

    // TPIE does seem to work after having called 'tpie::tpie_finish' the first
    // time. Hence, we will not update the _tpie_initialized flag.
    //
    // See: 'https://github.com/thomasmoelhave/tpie/issues/265'
  }
}
