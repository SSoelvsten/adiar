#ifndef ADIAR_INTERNAL_DOT_H
#define ADIAR_INTERNAL_DOT_H

#include <string.h>

#include <fstream>

#include <adiar/internal/dd.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_stream.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  template <typename file_t>
  void output_dot(const file_t& nodes, std::ostream &out)
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
        const node node = ns.pull();

        out << "\tn"
            << node.uid()._raw
            << " [label=<x<SUB>"
            << node.label()
            << "</SUB>, id<SUB>"
            << node.id() << "</SUB>>, style=rounded];"
            << std::endl;
      }

      out << "\tn" << ptr_uint64(false)._raw
          << " [label=\"" << file_t::false_print << "\"];" << std::endl;
      out << "\tn" << ptr_uint64(true)._raw
          << " [label=\"" << file_t::true_print << "\"];" << std::endl;

      out <<  std::endl << "\t// Arcs" << std::endl;

      ns.reset();
      while (ns.can_pull()) {
        const node node = ns.pull();

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
        const node current_node = ns.pull();

        out << "\t{ rank=same; " << "n" << current_node.uid()._raw << " ";

        while(ns.can_pull() && current_node.label() == ns.peek().label()) {
          out << "n" << ns.pull().uid()._raw << " ";
        }
        out << "}" << std::endl;
      }
    }
    out << "}" << std::endl;
  }

  template <typename file_t>
  void output_dot(const file_t& nodes, const std::string &filename)
  {
    std::ofstream out;
    out.open(filename);

    output_dot(nodes, out);
    out.close();
  }

  inline void output_dot(const shared_levelized_file<arc>& arcs,
                         const std::string &filename)
  {
    std::ofstream out;
    out.open(filename);

    out << "digraph BDD {" << std::endl;

    out << "\t// Node Arcs" << std::endl;

    arc_stream<> as(arcs);
    while (as.can_pull_internal()) {
      const arc a = as.pull_internal();
      out << "\t"
          << "n" << a.target().label() << "_" << a.target().id()
          << " -> "
          << "n" << a.source().label() << "_" << a.source().id()
          << " [style=" << (a.out_idx() ? "solid" : "dashed") << ", color=blue];"
          << std::endl;
    }

    out << std::endl << "\t// Sink Arcs" << std::endl;

    out << "\ts0 [shape=box, label=\"0\"];" << std::endl;
    out << "\ts1 [shape=box, label=\"1\"];" << std::endl;

    while (as.can_pull_terminal()) {
      const arc a = as.pull_terminal();
      out << "\t"
          << "n" << a.source().label() << "_" << a.source().id()
          << " -> "
          << "s" << a.target().value()
          << " [style=" << (a.out_idx() ? "solid" : "dashed") << ", color=red];"
          << std::endl;
    }

    out << "\t{ rank=min; s0 s1 }" << std::endl << "}" << std::endl;
    out.close();
  }
}

#endif // ADIAR_INTERNAL_DOT_H
