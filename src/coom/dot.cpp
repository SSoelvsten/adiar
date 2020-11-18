#ifndef COOM_DOT_CPP
#define COOM_DOT_CPP

#include "dot.h"

namespace coom {
  void output_dot(const arc_file& arcs, const std::string &filename)
  {
    std::ofstream out;
    out.open(filename + ".dot");

    out << "digraph OBDD {" << std::endl;

    out << "\t// Node Arcs" << std::endl;

    node_arc_stream<> nas(arcs);
    while (nas.can_pull()) {
      arc_t a = nas.pull();
      out << "\t"
          << "n" << label_of(a.target) << "_" << id_of(a.target)
          << " -> "
          << "n" << label_of(a.source) << "_" << id_of(a.source)
          << " [style=" << (is_flagged(a.source) ? "solid" : "dashed") << ", color=blue];"
          << std::endl;
    }

    out << std::endl << "\t// Sink Arcs" << std::endl;

    out << "\ts0 [shape=box, label=\"0\"];" << std::endl;
    out << "\ts1 [shape=box, label=\"1\"];" << std::endl;

    sink_arc_stream<> sas(arcs);
    while (sas.can_pull()) {
      arc_t a = sas.pull();
      out << "\t"
          << "n" << label_of(a.source) << "_" << id_of(a.source)
          << " -> "
          << "s" << value_of(a.target)
          << " [style=" << (is_flagged(a.source) ? "solid" : "dashed") << ", color=red];"
          << std::endl;
    }

    out << "\t{ rank=min; s0 s1 }" << std::endl << "}" << std::endl;
    out.close();
  }
}

#endif // COOM_DOT_CPP
