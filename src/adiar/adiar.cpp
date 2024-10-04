#include "adiar.h"

#include <exception>

#include <tpie/memory.h>
#include <tpie/tempname.h>
#include <tpie/tpie.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/memory.h>

namespace adiar
{
  /// \brief Whether Adiar is initialized.
  bool _adiar_initialized = false;

  /// \brief Whether TPIE is initialized.
  bool _tpie_initialized = false;

  /// \brief Subsystems of TPIE to be enabled
  const tpie::flags<tpie::subsystem> _tpie_subsystems =
    // Enable subsystems we use directly from Adiar
    tpie::MEMORY_MANAGER | tpie::STREAMS | tpie::TEMPFILE
    | tpie::FILE_MANAGER
    // Enable subsystems hiding inside 'tpie::sort' and 'tpie::merge_sorter'
    | tpie::PROGRESS
    | tpie::JOB_MANAGER
#ifndef NDEBUG
    // Enable default logging to 'tmp/*/TPIE_log*.txt' while in Debug
    | tpie::DEFAULT_LOGGING
#endif
    ;

#ifdef NDEBUG
  /// \brief Empty implementation of TPIE's logging system while in Production.
  struct dev_null : tpie::log_target
  {
    void
    log(tpie::log_level, const char*, size_t) override
    {}

    void
    begin_group(const std::string&) override
    {}

    void
    end_group() override
    {}
  };

  dev_null _devnull;
#endif

  void
  adiar_init(size_t memory_limit_bytes, std::string temp_dir)
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
                             + std::to_string(minimum_memory / 1024 / 1024) + " MiB of memory");
    }

    try {
      // Set the temporary directory for TPIE before calling
      // `tpie::tpie_init()`. This avoids a directory named `TPIE_<date>_<...>`
      // at the default tmp path.

      // - file names
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      // - tmp directory
      if (temp_dir != "") { tpie::tempname::set_default_path(temp_dir); }

      // Initialise TPIE
      tpie::tpie_init(_tpie_subsystems);

#ifdef NDEBUG
      // - add 'dev/null' output for TPIE logging in Production. Otherwise, TPIE will print
      //   everything to 'std::cerr'.
      tpie::add_log_target(&_devnull);
#endif

      // - memory limit and block size
      tpie::get_memory_manager().set_limit(memory_limit_bytes);
      internal::set_block_size(internal::recommended_block_size(memory_limit_bytes));

      _tpie_initialized = true;

      // Initialise Adiar state
      // - reset statistics
      statistics_reset();

      // - all done, mark initialized
      _adiar_initialized = true;
    } catch (const std::exception& e) {
      // LCOV_EXCL_START
      // Mark as not initialized.
      _adiar_initialized = false;
      _tpie_initialized  = true;

      // Try to gracefully close down TPIE.
      tpie::tpie_finish(_tpie_subsystems);

      // Tell the user what happened.
      throw e;
      // LCOV_EXCL_STOP
    }
  }

  bool
  adiar_initialized() noexcept
  {
    return _adiar_initialized;
  }

  void
  adiar_deinit()
  {
    if (!_adiar_initialized) return;

    domain_unset();

    tpie::tpie_finish(_tpie_subsystems);
    _adiar_initialized = false;

    // TPIE does seem to work after having called 'tpie::tpie_finish' the first
    // time. Hence, we will not update the _tpie_initialized flag.
    //
    // See: 'https://github.com/thomasmoelhave/tpie/issues/265'
  }
}
