#include "memory.h"

#include <tpie/tpie.h>

#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  namespace memory
  {
    void set_path(const std::string &temp_dir)
    {
      // Naming scheme and path for temporary files
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      if (temp_dir != "") {
        tpie::tempname::set_default_path(temp_dir);
      }
    }

    void set_limit(size_t memory_limit_bytes)
    {
      adiar_assert(memory_limit_bytes >= MINIMUM_BYTES,
                   "Adiar requires at least 128 MiB of memory");

      tpie::get_memory_manager().set_limit(memory_limit_bytes);
    }

    size_t recommended_block_size(size_t /* memory_limit_bytes */)
    {
      const size_t TWO_MiB = 1 << 21; // TPIE default
      return TWO_MiB;
    }

    void set_block_size(size_t block_size_bytes)
    {
      tpie::set_block_size(block_size_bytes);
    }

    size_t get_block_size()
    {
      return tpie::get_block_size();
    }
  }
}
