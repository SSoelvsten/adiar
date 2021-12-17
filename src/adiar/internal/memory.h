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

#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>

#include <tpie/tpie.h>

namespace adiar {
  namespace memory
  {
    constexpr size_t MINIMUM_MEMORY_LIMIT = 128 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Initiates the memory management of TPIE.
    //////////////////////////////////////////////////////////////////////////////
    void init(const std::string &temp_dir = "");

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the memory limit for TPIE.
    //////////////////////////////////////////////////////////////////////////////
    void set_limit(size_t memory_limit_bytes = MINIMUM_MEMORY_LIMIT);

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the block size for TPIE.
    //////////////////////////////////////////////////////////////////////////////
    void set_block_size(tpie::memory_size_type memory_limit_bytes);
  }
}

#endif // ADIAR_INTERNAL_MEMORY_H
