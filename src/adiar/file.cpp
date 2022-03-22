#include "file.h"

#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>

namespace adiar
{
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
}
