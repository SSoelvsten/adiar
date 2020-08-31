#ifndef COOM_DOT_CPP
#define COOM_DOT_CPP

#include <stdint.h>
#include <string.h>

#include <fstream>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/tpie_log.h>

// COOM Imports
#include "data.h"

#include "dot.h"

namespace coom {
  void output_dot(tpie::file_stream<node_t>& nodes, std::string filename)
  {
    std::ofstream out;
    out.open(filename + ".dot");

    out << "digraph OBDD {" << std::endl;

    nodes.seek(0);

    if (nodes.size() == 1 && is_sink(nodes.peek())) {
      out << "\t" << value_of(nodes.read()) << " [shape=box];" << std::endl;
    } else if (nodes.size() > 0) {
      out << "\t// Nodes" << std::endl;

      nodes.seek(0, tpie::file_stream_base::end);
      out << "\tnode [shape=box];" << std::endl;

      while (nodes.can_read_back()) {
        auto node = nodes.read_back();

        out << "\tn"
            << node.uid
            << " [label=<x<SUB>"
            << label_of(node)
            << "</SUB>, id<SUB>"
            << (id_of(node)) << "</SUB>>, style=rounded];"
            << std::endl;
      }

      out << "\tn" << create_sink_ptr(false) << " [label=\"0\"];" << std::endl;
      out << "\tn" << create_sink_ptr(true) << " [label=\"1\"];" << std::endl;

      out <<  std::endl << "\t// Arcs" << std::endl;

      nodes.seek(0, tpie::file_stream_base::end);
      while (nodes.can_read_back()) {
        auto node = nodes.read_back();

        out << "\tn" << node.uid
            << " -> "
            << "n" << node.low
            << " [style=dashed];" << std::endl;
        out << "\tn" << node.uid
            << " -> "
            << "n" << node.high
            << "[style=solid];"  << std::endl;
      }

      out <<  std::endl << "\t// Ranks" << std::endl;
      nodes.seek(0, tpie::file_stream_base::end);

      node_t prev_node = nodes.read_back();
      out << "\t{ rank=same; " << "n" << prev_node.uid << " }" << std::endl;

      bool has_next_node = nodes.can_read_back();
      node_t next_node;
      if (has_next_node) {
        next_node = nodes.read_back();
      }

      while (has_next_node) {
        out << "\t{ rank=same; " << "n" << next_node.uid << " ";
        prev_node = next_node;

        if (nodes.can_read_back()) {
          next_node = nodes.read_back();
        } else {
          has_next_node = false;
        }

        while(has_next_node && label_of(next_node) == label_of(prev_node)) {
          out << "n" << next_node.uid << " ";

          if (nodes.can_read_back()) {
            next_node = nodes.read_back();
          } else {
            has_next_node = false;
          }
        }
        out << "}" << std::endl;
      } while (has_next_node);
    }
    out << "}" << std::endl;
    out.close();

    system(("dot -Tpng " + filename + ".dot" + " -o " + filename + ".png").c_str());
  }

  void output_dot(tpie::file_stream<arc_t>& node_arcs,
                  tpie::file_stream<arc_t>& sink_arcs,
                  std::string filename)
  {
    std::ofstream out;
    out.open(filename + ".dot");

    out << "digraph OBDD {" << std::endl;

    node_arcs.seek(0);
    out << "\t// Node Arcs" << std::endl;

    while (node_arcs.can_read())
      {
        arc_t a = node_arcs.read();
        out << "\t"
            << "n" << label_of(a.target) << "_" << id_of(a.target)
            << " -> "
            << "n" << label_of(a.source) << "_" << id_of(a.source)
            << " [style=" << (is_flagged(a.source) ? "solid" : "dashed") << ", color=blue];"
            << std::endl;
      }

    sink_arcs.seek(0);
    out << std::endl << "\t// Sink Arcs" << std::endl;

    out << "\ts0 [shape=box, label=\"0\"];" << std::endl;
    out << "\ts1 [shape=box, label=\"1\"];" << std::endl;

    while (sink_arcs.can_read())
      {
        arc_t a = sink_arcs.read();
        out << "\t"
            << "n" << label_of(a.source) << "_" << id_of(a.source)
            << " -> "
            << "s" << value_of(a.target)
            << " [style=" << (is_flagged(a.source) ? "solid" : "dashed") << ", color=red];"
            << std::endl;
          }

    out << "\t{ rank=min; s0 s1 }" << std::endl << "}" << std::endl;
    out.close();

    system(("dot -Tpng " + filename + ".dot" + " -o " + filename + ".png").c_str());
  }
}

#endif // COOM_DOT_CPP
