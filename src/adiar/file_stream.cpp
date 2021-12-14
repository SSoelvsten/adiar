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

#include "file_stream.h"

namespace adiar
{
  template<>
  level_info_stream<node_t, NODE_FILE_COUNT, false>::level_info_stream(const __decision_diagram& dd) : level_info_stream(dd.get<node_file>()) { }

  template<>
  level_info_stream<node_t, NODE_FILE_COUNT, true>::level_info_stream(const __decision_diagram& dd) : level_info_stream(dd.get<node_file>()) { }

  template<>
  level_info_stream<arc_t, ARC_FILE_COUNT, false>::level_info_stream(const __decision_diagram& dd) : level_info_stream(dd.get<arc_file>()) { }

  template<>
  level_info_stream<arc_t, ARC_FILE_COUNT, true>::level_info_stream(const __decision_diagram& dd) : level_info_stream(dd.get<arc_file>()) { }
}
