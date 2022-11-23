#ifndef ADIAR_INTERNAL_DOT_H
#define ADIAR_INTERNAL_DOT_H

#include <string.h>

#include <fstream>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>

namespace adiar::internal
{
  template <typename T>
  void output_dot(const T& nodes, std::ostream &out)
  {
    out << "digraph BDD {" << std::endl;

    node_stream<> ns(nodes);

    if (is_terminal(nodes)) {
      out << "\t"
          << ns.pull().value()
          << " [shape=box];" << std::endl;
    } else {
      out << "\t// Nodes" << std::endl;
      out << "\tnode [shape=box];" << std::endl;

      while (ns.can_pull()) {
        node node = ns.pull();

        out << "\tn"
            << node.uid()._raw
            << " [label=<x<SUB>"
            << node.label()
            << "</SUB>, id<SUB>"
            << node.id() << "</SUB>>, style=rounded];"
            << std::endl;
      }

      out << "\tn" << ptr_uint64(false)._raw << " [label=\"" << T::false_print << "\"];" << std::endl;
      out << "\tn" << ptr_uint64(true)._raw << " [label=\"" << T::true_print << "\"];" << std::endl;

      out <<  std::endl << "\t// Arcs" << std::endl;

      ns.reset();
      while (ns.can_pull()) {
        node node = ns.pull();

        out << "\tn" << node.uid()._raw
            << " -> "
            << "n" << node.low()._raw
            << " [style=dashed];" << std::endl;
        out << "\tn" << node.uid()._raw
            << " -> "
            << "n" << node.high()._raw
            << "[style=solid];"  << std::endl;
      }

      out <<  std::endl << "\t// Ranks" << std::endl;

      ns.reset();
      out << "\t{ rank=same; " << "n" << ns.pull().uid()._raw << " }" << std::endl;

      while (ns.can_pull()) {
        node current_node = ns.pull();

        out << "\t{ rank=same; " << "n" << current_node.uid()._raw << " ";

        while(ns.can_pull() && current_node.label() == ns.peek().label()) {
          out << "n" << ns.pull().uid()._raw << " ";
        }
        out << "}" << std::endl;
      }
    }
    out << "}" << std::endl;
  }

  template <typename T>
  void output_dot(const T& nodes, const std::string &filename)
  {
    std::ofstream out;
    out.open(filename);

    output_dot(nodes, out);
    out.close();
  }

  inline void output_dot(const arc_file& arcs, const std::string &filename)
  {
    std::ofstream out;
    out.open(filename);

    out << "digraph BDD {" << std::endl;

    out << "\t// Node Arcs" << std::endl;

    node_arc_stream<> nas(arcs);
    while (nas.can_pull()) {
      arc a = nas.pull();
      out << "\t"
          << "n" << a.target().label() << "_" << a.target().id()
          << " -> "
          << "n" << a.source().label() << "_" << a.source().id()
          << " [style=" << (a.is_high() ? "solid" : "dashed") << ", color=blue];"
          << std::endl;
    }

    out << std::endl << "\t// Sink Arcs" << std::endl;

    out << "\ts0 [shape=box, label=\"0\"];" << std::endl;
    out << "\ts1 [shape=box, label=\"1\"];" << std::endl;

    terminal_arc_stream<> sas(arcs);
    while (sas.can_pull()) {
      arc a = sas.pull();
      out << "\t"
          << "n" << a.source().label() << "_" << a.source().id()
          << " -> "
          << "s" << a.target().value()
          << " [style=" << (a.is_high() ? "solid" : "dashed") << ", color=red];"
          << std::endl;
    }

    out << "\t{ rank=min; s0 s1 }" << std::endl << "}" << std::endl;
    out.close();
  }
}

#endif // ADIAR_INTERNAL_DOT_H
