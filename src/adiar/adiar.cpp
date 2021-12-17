/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "adiar.h"

#include <tpie/tpie.h>

#include <adiar/internal/memory.h>
#include <adiar/assert.h>

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

    tpie::tpie_init();

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
    if (_adiar_initialized) tpie::tpie_finish();
    _adiar_initialized = false;
  }
}
