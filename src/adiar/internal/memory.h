#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>

#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  namespace memory
  {
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Choices for whether to use only internal or external memory, or
    ///        let Adiar choose based on size of input.
    //////////////////////////////////////////////////////////////////////////////
    enum memory_mode { AUTO, INTERNAL, EXTERNAL };

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The current memory mode (set to AUTO as default).
    //////////////////////////////////////////////////////////////////////////////
    extern memory_mode mode;

    constexpr size_t MINIMUM_BYTES = 128 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the path for temporary TPIE files.
    //////////////////////////////////////////////////////////////////////////////
    inline void set_path(const std::string &temp_dir = "")
    {
      // Naming scheme and path for temporary files
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      if (temp_dir != "") {
        tpie::tempname::set_default_path(temp_dir);
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the memory limit for TPIE.
    //////////////////////////////////////////////////////////////////////////////
    inline void set_limit(size_t memory_limit_bytes = MINIMUM_BYTES)
    {
      adiar_assert(memory_limit_bytes >= MINIMUM_BYTES,
                   "Adiar requires at least 128 MiB of memory");

      tpie::get_memory_manager().set_limit(memory_limit_bytes);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Minimum block size of 2 MiB.
    //////////////////////////////////////////////////////////////////////////////
    constexpr size_t MINIMUM_BLOCK_SIZE = 2 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the block size for TPIE.
    //////////////////////////////////////////////////////////////////////////////
    inline void set_block_size(size_t block_size_bytes)
    {
      tpie::set_block_size(block_size_bytes);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Get the block size from TPIE.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t get_block_size()
    {
      return tpie::get_block_size();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Computes a recommended block size to be used with a specific
    ///        amount of available internal memory.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t recommended_block_size(size_t /*memory_limit_bytes*/)
    {
      return MINIMUM_BLOCK_SIZE;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain from TPIE the amount of available memory.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t available()
    {
      return tpie::get_memory_manager().available();
    }
  }
}

#endif // ADIAR_INTERNAL_MEMORY_H
