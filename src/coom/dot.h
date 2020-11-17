#ifndef COOM_DOT_H
#define COOM_DOT_H

#include <string.h>
#include <coom/file.h>

namespace coom {
  void output_dot(const node_file& nodes, const std::string &filename);
  void output_dot(const arc_file& arcs, const std::string &filename);
  void output_dot(const node_or_arc_file& arcs, const std::string &filename);
}

#endif // COOM_DOT_H
