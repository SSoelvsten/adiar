#ifndef ADIAR_DOT_H
#define ADIAR_DOT_H

#include <string.h>
#include <adiar/file.h>

#include <fstream>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

namespace adiar {
  template <typename T>
  void output_dot(const T& nodes, const std::string &filename)
  {
    std::ofstream out;
    out.open(filename);

    out << "digraph OBDD {" << std::endl;

    node_stream<> ns(nodes);

    if (is_sink(nodes)) {
      out << "\t"
          << value_of(ns.pull())
          << " [shape=box];" << std::endl;
    } else {
      out << "\t// Nodes" << std::endl;
      out << "\tnode [shape=box];" << std::endl;

      while (ns.can_pull()) {
        node_t node = ns.pull();

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

      ns.reset();
      while (ns.can_pull()) {
        node_t node = ns.pull();

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

      ns.reset();
      out << "\t{ rank=same; " << "n" << ns.pull().uid << " }" << std::endl;

      while (ns.can_pull()) {
        node_t current_node = ns.pull();

        out << "\t{ rank=same; " << "n" << current_node.uid << " ";

        while(ns.can_pull() && label_of(current_node) == label_of(ns.peek())) {
          out << "n" << ns.pull().uid << " ";
        }
        out << "}" << std::endl;
      }
    }
    out << "}" << std::endl;
    out.close();
  }

  void output_dot(const arc_file& arcs, const std::string &filename);
}

#endif // ADIAR_DOT_H
