#ifndef COOM_DEBUG_DATA_H
#define COOM_DEBUG_DATA_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <stdint.h>

#include "data.h"

namespace coom { namespace debug {
    void print_nil_ptr();
    void print_node_ptr(ptr_t n);
    void print_sink_ptr(ptr_t n);

    void print_child(ptr_t n);

    void print_node(const node_t &n);
    void println_node(const node_t &n);

    void print_arc(const arc_t &a);
    void println_arc(const arc_t &a);

    void print_file_stream(tpie::file_stream<node_t> &nodes);
    void print_file_stream(tpie::file_stream<node_t> &nodes, std::string name);
    void println_file_stream(tpie::file_stream<node_t> &nodes);
    void println_file_stream(tpie::file_stream<node_t> &nodes, std::string name);
    void print_file_stream(tpie::file_stream<arc_t> &arcs);
    void print_file_stream(tpie::file_stream<arc_t> &arcs, std::string name);
    void println_file_stream(tpie::file_stream<arc_t> &arcs);
    void println_file_stream(tpie::file_stream<arc_t> &arcs, std::string name);
  }
}

#endif // COOM_DEBUG_DATA_H
