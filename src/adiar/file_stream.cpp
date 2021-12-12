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
