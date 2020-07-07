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
#include "data.cpp"

#include "dot.h"

namespace coom {
  void output_dot_nodes(std::ofstream& out, tpie::file_stream<node>& nodes)
  {
    nodes.seek(0, tpie::file_stream_base::end);
    out << "\tnode [shape=box];" << std::endl;

    while (nodes.can_read_back()) {
      auto node = nodes.read_back();

      out << "\tn"
          << node.node_ptr
          << " [label=<x<SUB>"
          << label_of(node)
          << "</SUB>, id<SUB>"
          << (id_of(node)) << "</SUB>>, style=rounded];"
          << std::endl;
    }
  }

  void output_dot_arcs(std::ofstream& out, tpie::file_stream<node>& nodes)
  {
    nodes.seek(0, tpie::file_stream_base::end);
    while (nodes.can_read_back()) {
      auto node = nodes.read_back();

      out << "\tn" << node.node_ptr << " -> " << "n" << node.low << " [style=solid];" << std::endl;
      out << "\tn" << node.node_ptr << " -> " << "n" << node.high << "[style=dashed];"  << std::endl;
    }
  }

  void output_dot(std::string filename, tpie::file_stream<node>& nodes)
  {
    std::ofstream out;
    out.open(filename);

    out << "digraph OBDD {" << std::endl;

    nodes.seek(0);

    if (nodes.size() == 1 && is_sink_node(nodes.peek())) {
      out << "\t" << value_of(nodes.read()) << " [shape=box];" << std::endl;
    } else if (nodes.size() > 0) {
      out << "\t// Nodes" << std::endl;
      output_dot_nodes(out, nodes);

      out << "\tn" << create_sink(false) << " [label=\"0\"];" << std::endl;
      out << "\tn" << create_sink(true) << " [label=\"1\"];" << std::endl;

      out <<  std::endl << "\t// Arcs" << std::endl;
      output_dot_arcs(out, nodes);
    }
    out << "}" << std::endl;
  }
}

int main(const int argc, const char* argv[]) {
  // ===== TPIE =====
  // Initialize
  tpie::tpie_init();

  size_t available_memory_mb = 128;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // ===== FAIL FAST ON MISSING ARGUMENT =====
  if (argc == 1) {
    tpie::log_info() << "// Please provide a filename(s) to print" << std::endl;

    tpie::file_stream<coom::node> file_stream;
    file_stream.open();

    file_stream.write(coom::create_node(2, coom::MAX_ID,
                                        coom::create_sink(false),
                                        coom::create_sink(true)));

    file_stream.write(coom::create_node(1, coom::MAX_ID,
                                        coom::create_node_ptr(2,coom::MAX_ID),
                                        coom::create_sink(true)));

    file_stream.write(coom::create_node(0, coom::MAX_ID,
                                        coom::create_node_ptr(2,coom::MAX_ID),
                                        coom::create_node_ptr(1,coom::MAX_ID)));

    std::string out_filename("test.tpie.dot");
    coom::output_dot("test.tpie", file_stream);

    tpie::tpie_finish();
    exit(0);
  }

  // ===== OUTPUT DOT FILE =====
  int arg = 0;
  while (arg < argc) {
    const char* filename = argv[arg+1];

    tpie::file_stream<coom::node> file_stream;
    file_stream.open(filename, tpie::open::read_only | tpie::open::compression_normal);

    std::string filename_str(filename);
    coom::output_dot(filename_str + ".dot", file_stream);
  }

  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(0);
}

#endif // COOM_DOT_CPP
