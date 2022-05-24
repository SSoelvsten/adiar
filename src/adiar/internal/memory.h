#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>
#include <cmath>

#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  namespace memory
  {
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
    inline size_t recommended_block_size(size_t memory_limit_bytes)
    {
      constexpr size_t MINIMUM_BLOCK_SIZE_MiB = 2;
      constexpr size_t MAXIMUM_BLOCK_SIZE_MiB = 32;

      constexpr size_t MAX_SPLITS_OF_MEMORY = 12;

      constexpr size_t MiB_VS_BYTES = 1024 * 1024;

      const size_t memory_limit_MiB = memory_limit_bytes / MiB_VS_BYTES;

      const size_t block_size_ub = MiB_VS_BYTES *
        std::min(static_cast<size_t>(sqrt(memory_limit_MiB / MAX_SPLITS_OF_MEMORY)), MAXIMUM_BLOCK_SIZE_MiB);

      size_t block_size = MINIMUM_BLOCK_SIZE_MiB * MiB_VS_BYTES;
      while ((block_size * 2) <= block_size_ub) {
        block_size *= 2;
      }
      return block_size;
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
