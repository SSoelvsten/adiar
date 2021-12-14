/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "memory.h"

#include <tpie/tpie.h>

#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/assert.h>

namespace adiar
{
  namespace memory
  {
    const tpie::memory_size_type BLOCK_SIZE = 1 << 21; // 2 MiB (TPIE default)

    void init(const std::string &temp_dir)
    {
      // Naming scheme and path for temporary files
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      if (temp_dir != "") {
        tpie::tempname::set_default_path(temp_dir);
      }

      // Set default memory
      set_limit();

      // Set block size
      set_block_size(BLOCK_SIZE);
    }

    void set_limit(size_t memory_limit_bytes)
    {
      adiar_assert(memory_limit_bytes >= MINIMUM_MEMORY_LIMIT,
                   "ADIAR requires at least 128 MiB of memory");
      tpie::get_memory_manager().set_limit(memory_limit_bytes);
    }

    void set_block_size(tpie::memory_size_type block_size)
    {
      tpie::set_block_size(block_size);
    }
  }
}
