#ifndef COOM_FILE_CPP
#define COOM_FILE_CPP

#include "file.h"

#include <coom/assert.h>
#include <coom/reduce.h>

#include <coom/file_stream.h>

namespace coom
{
  // TODO: Remove?

  //////////////////////////////////////////////////////////////////////////////
  /// node_file
  bool is_sink(const node_file &file, const sink_pred &pred)
  {
    coom_assert(!file.empty(), "Invalid node_file: empty");

    if (file.size() != 1) {
      return false;
    }
    node_stream<> ns(file);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  template<bool reverse>
  label_t extract_label(const node_file &file)
  {
    coom_assert(file.size() > 0, "Invalid node_file: empty");

    node_stream<reverse> ns(file);
    node_t n = ns.pull();

    coom_assert(!is_sink(n), "Cannot extract label from sink-only file");

    return label_of(n);
  }

  label_t min_label(const node_file &file)
  {
    return extract_label<false>(file);
  }

  label_t max_label(const node_file &file)
  {
    return extract_label<true>(file);
  }

  uint64_t nodecount(const node_file &nodes)
  {
    return nodes.size();
  }

  uint64_t nodecount(const arc_file &arcs)
  {
    // Every node is represented by two arcs
    return arcs.size() / 2;
  }
}

#endif // COOM_FILE_CPP
