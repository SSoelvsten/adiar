#include "file.h"

#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  bool is_sink(const node_file &file)
  {
    adiar_debug(!file.empty(), "Invalid node_file: empty");

    // A node_file only contains a sink iff the number of arcs to a sink value
    // in its meta information is exactly one.
    return (file._file_ptr -> number_of_sinks[false] +
            file._file_ptr -> number_of_sinks[true]) == 1;
  }

  bool value_of(const node_file &file)
  {
    adiar_debug(is_sink(file), "Must be a sink to extract its value");

    // Since the sum of the number of sinks is exactly one, then we can use the
    // value of the number of true sinks to indirectly derive the value of the
    // sink.
    return file._file_ptr -> number_of_sinks[true];
  }

  template<bool reverse>
  label_t extract_label(const node_file &file)
  {
    adiar_debug(file.size() > 0, "Invalid node_file: empty");

    node_stream<reverse> ns(file);
    node_t n = ns.pull();

    adiar_debug(!is_sink(n), "Cannot extract label from sink-only file");

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
    if (is_sink(nodes)) {
      return 0u;
    }
    return nodes.size();
  }

  uint64_t nodecount(const arc_file &arcs)
  {
    // Every node is represented by two arcs
    return arcs.size() / 2;
  }

  uint64_t varcount(const node_file &nodes)
  {
    return nodes.meta_size();
  }
}
