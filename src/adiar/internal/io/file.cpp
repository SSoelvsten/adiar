#include "file.h"

#include <adiar/internal/assert.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  template<bool reverse>
  node::label_t extract_label(const node_file &file)
  {
    adiar_debug(file->size() > 0, "Invalid node_file: empty");

    node_stream<reverse> ns(file);
    node n = ns.pull();

    adiar_debug(!n.is_terminal(), "Cannot extract label from terminal-only file");

    return n.label();
  }

  node::label_t min_label(const node_file &file)
  {
    return extract_label<false>(file);
  }

  node::label_t max_label(const node_file &file)
  {
    return extract_label<true>(file);
  }
}
