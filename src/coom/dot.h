#ifndef COOM_DOT_H
#define COOM_DOT_H

#include <tpie/file_stream.h>

#include <string.h>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/file_stream.h>

// COOM Imports
#include <coom/data.h>


namespace coom {
  void output_dot(tpie::file_stream<node_t>& nodes,
                  std::string filename);

  void output_dot(tpie::file_stream<arc_t>& node_arcs,
                  tpie::file_stream<arc_t>& sink_arcs,
                  std::string filename);
}

#endif // COOM_DOT_H
