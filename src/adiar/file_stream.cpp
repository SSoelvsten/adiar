#include "file_stream.h"

namespace adiar
{
  template<>
  meta_stream<node_t, 1, false>::meta_stream(const __bdd& bdd) : meta_stream(bdd.get<node_file>()) { }

  template<>
  meta_stream<node_t, 1, true>::meta_stream(const __bdd& bdd) : meta_stream(bdd.get<node_file>()) { }

  template<>
  meta_stream<arc_t, 2, false>::meta_stream(const __bdd& bdd) : meta_stream(bdd.get<arc_file>()) { }

  template<>
  meta_stream<arc_t, 2, true>::meta_stream(const __bdd& bdd) : meta_stream(bdd.get<arc_file>()) { }
}
