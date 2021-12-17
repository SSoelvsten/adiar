////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

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
