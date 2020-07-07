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
  void output_dot_nodes(tpie::file_stream<node>& nodes, std::ofstream& out)
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

  void output_dot_arcs(tpie::file_stream<node>& nodes, std::ofstream& out)
  {
    nodes.seek(0, tpie::file_stream_base::end);
    while (nodes.can_read_back()) {
      auto node = nodes.read_back();

      out << "\tn" << node.node_ptr << " -> " << "n" << node.low << " [style=solid];" << std::endl;
      out << "\tn" << node.node_ptr << " -> " << "n" << node.high << "[style=dashed];"  << std::endl;
    }
  }

  void output_dot(tpie::file_stream<node>& nodes, std::string filename)
  {
    std::ofstream out;
    out.open(filename + ".dot");

    out << "digraph OBDD {" << std::endl;

    nodes.seek(0);

    if (nodes.size() == 1 && is_sink_node(nodes.peek())) {
      out << "\t" << value_of(nodes.read()) << " [shape=box];" << std::endl;
    } else if (nodes.size() > 0) {
      out << "\t// Nodes" << std::endl;
      output_dot_nodes(nodes, out);

      out << "\tn" << create_sink(false) << " [label=\"0\"];" << std::endl;
      out << "\tn" << create_sink(true) << " [label=\"1\"];" << std::endl;

      out <<  std::endl << "\t// Arcs" << std::endl;
      output_dot_arcs(nodes, out);
    }
    out << "}" << std::endl;
    out.close();

    system(("dot -Tpng " + filename + ".dot" + " -o " + filename + ".png").c_str());
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
    tpie::log_info() << "Please provide a filename(s) to print" << std::endl;

    // TODO: Remove from here...
    tpie::log_info() << "(Will create dot_test.tpie for testing)" << std::endl;

    tpie::file_stream<coom::node> file_stream;
    file_stream.open("dot_test.tpie", tpie::open::write_only | tpie::open::compression_normal);

    file_stream.write(coom::create_node(2, coom::MAX_ID,
                                        coom::create_sink(false),
                                        coom::create_sink(true)));

    file_stream.write(coom::create_node(1, coom::MAX_ID,
                                        coom::create_node_ptr(2,coom::MAX_ID),
                                        coom::create_sink(true)));

    file_stream.write(coom::create_node(0, coom::MAX_ID,
                                        coom::create_node_ptr(2,coom::MAX_ID),
                                        coom::create_node_ptr(1,coom::MAX_ID)));
    file_stream.close();

    // TODO: Remove until here...

    tpie::tpie_finish();
    exit(0); // TODO: Make into non-zero
  }

  // ===== OUTPUT DOT FILE =====
  int arg = 1; // argument 0 is the executable

  while (arg < argc) {
    const char* filename = argv[arg];

    tpie::file_stream<coom::node> file_stream;
    file_stream.open(filename, tpie::open::read_only | tpie::open::compression_normal);

    std::string filename_str(filename);
    coom::output_dot(file_stream, filename_str);
    file_stream.close();

    arg += 1;
  }

  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(0);
}

#endif // COOM_DOT_CPP
