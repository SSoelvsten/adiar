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
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a container of nodes to a given output stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename nodes_t>
  inline void __print_dot(const nodes_t& nodes, std::ostream &out)
  {
    out << "digraph DD {" << std::endl;

    node_stream<> ns(nodes);

    if (nodes->is_terminal()) {
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
          << " [label=\"" << nodes_t::false_print << "\"];" << std::endl;
      out << "\tn" << ptr_uint64(true)._raw
          << " [label=\"" << nodes_t::true_print << "\"];" << std::endl;

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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a container of nodes to a given file name
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  inline void __print_dot(const dd_t& dd, const std::string& filename)
  {
    // Create output stream for file
    std::ofstream out;
    out.open(filename);

    // Print to output
    __print_dot(dd, out);

    // Close
    out.close();
  }

  // TODO: print_dot(const shared_levelized_file<node>& nodes, ...)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a decision diagram to a given output stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void print_dot(const dd_t& dd, std::ostream &out)
  { __print_dot(dd, out); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a decision diagram to a given file
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void print_dot(const dd_t& dd, const std::string &filename)
  { __print_dot(dd, filename); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Print dot file of an intermediate output of arcs.
  //////////////////////////////////////////////////////////////////////////////
  inline void print_dot(const shared_levelized_file<arc>& arcs,
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

  // TODO: generalize for non-sharing arc files and std::ostream
}

#endif // ADIAR_INTERNAL_DOT_H
