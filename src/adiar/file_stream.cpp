#include "file_stream.h"

namespace adiar
{
  template<>
  meta_stream<node_t, NODE_FILE_COUNT, false>::meta_stream(const __decision_diagram& dd) : meta_stream(dd.get<node_file>()) { }

  template<>
  meta_stream<node_t, NODE_FILE_COUNT, true>::meta_stream(const __decision_diagram& dd) : meta_stream(dd.get<node_file>()) { }

  template<>
  meta_stream<arc_t, ARC_FILE_COUNT, false>::meta_stream(const __decision_diagram& dd) : meta_stream(dd.get<arc_file>()) { }

  template<>
  meta_stream<arc_t, ARC_FILE_COUNT, true>::meta_stream(const __decision_diagram& dd) : meta_stream(dd.get<arc_file>()) { }
}
