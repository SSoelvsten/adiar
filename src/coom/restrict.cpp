#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include "data.h"
#include "data_pty.h"
#include "reduce.h"

#include "restrict.h"

namespace coom
{
  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes,
                tpie::file_stream<arc> &reduce_node_arcs,
                tpie::file_stream<arc> &reduce_sink_arcs)
  {
    in_nodes.seek(0);

    out_nodes.write(create_sink_node(false));
  }

  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes)
  {
#if COOM_ASSERT
    assert (out_nodes.size() == 0);
#endif
#if COOM_DEBUG
    tpie::log_info() << "//===\\\\ RESTRICT //===\\\\\n";
    tpie::log_info() << "in_nodes: ";
    coom::println_file_stream(in_nodes);
#endif

    tpie::file_stream<arc> reduce_node_arcs;
    reduce_node_arcs.open();

    tpie::file_stream<arc> reduce_sink_arcs;
    reduce_sink_arcs.open();

    restrict(in_nodes, in_assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

    if (reduce_node_arcs.size() > 0 || reduce_sink_arcs.size() > 0) {
#if COOM_ASSERT
      assert (out_nodes.size() == 0);
#endif
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    } else {
#if COOM_ASSERT
      assert (out_nodes.size() == 1);
#endif

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      coom::println_file_stream(out_nodes);
#endif
    }

#if COOM_DEBUG
    tpie::log_info() << "\\\\===// RESTRICT \\\\===//\n";
#endif
  }
}

#endif // COOM_RESTRICT_CPP
