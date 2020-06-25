#ifndef COOM_DATA_PTY_CPP
#define COOM_DATA_PTY_CPP

#include <stdint.h>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/tpie_log.h>

#include "data.h"

#include "data_pty.h"

namespace coom {
  void print_nil_ptr()
  {
    tpie::log_info() << "[* NIL *]";
  }

  void print_node_ptr(uint64_t n)
  {
    auto label = label_of(n);
    auto index = id_of(n);
    tpie::log_info() << "[* " << label << " : " << index << " *]";
  }

  void print_sink_ptr(uint64_t n)
  {
    bool value = value_of(n);
    tpie::log_info() << "[* " << value << " *]";
  }

  void print_child(uint64_t n)
  {
    if (is_nil(n))
      {
        print_nil_ptr();
      }
    else if (is_sink(n))
      {
        print_sink_ptr(n);
      }
    else if (is_node_ptr(n))
      {
        print_node_ptr(n);
      }
    else
      {
        tpie::log_info() << "[* ? *]";
      }
  }

  void print_node(const node& n)
  {
    tpie::log_info() << "([ ";
    if (is_sink_node(n))
      {
        bool value = value_of(n);
        tpie::log_info() << value;
      }
    else
      {
        auto label = label_of(n);
        auto index = id_of(n);
        tpie::log_info() << label << " , " << index;
      }
    tpie::log_info() << " ], ";
    print_child(n.low);
    tpie::log_info() << ", ";
    print_child(n.high);
    tpie::log_info() << " )";
  }

  void println_node(const node& n)
  {
    print_node(n);
    tpie::log_info() << std::endl;
  }

  void print_arc(const arc& a)
  {
    tpie::log_info() << "( ";
    print_child(a.source);
    tpie::log_info() << " , ";
    if (a.is_high)
      {
        tpie::log_info() << " T ";
      }
    else
      {
        tpie::log_info() << " F ";
      }
    tpie::log_info() << " , ";
    print_child(a.target);
    tpie::log_info() << " )";
  }

  void println_arc(const arc& a)
  {
    print_arc(a);
    tpie::log_info() << std::endl;
  }

  void print_file_stream(tpie::file_stream<node>& nodes)
  {
    auto original_pos = nodes.get_position();
    nodes.seek(0);

    tpie::log_info() << "file_stream<node> {" << std::endl;
    while (nodes.can_read())
      {
        tpie::log_info() << "\t";
        print_node(nodes.read());
        tpie::log_info() << "," << std::endl;
      }
    tpie::log_info() << "}";

    nodes.set_position(original_pos);
  }

  void println_file_stream(tpie::file_stream<node>& nodes)
  {
    print_file_stream(nodes);
    tpie::log_info() << std::endl;
  }

  void print_file_stream(tpie::file_stream<arc>& arcs)
  {
    auto original_pos = arcs.get_position();
    arcs.seek(0);

    tpie::log_info() << "file_stream<arc> {" << std::endl;
    while (arcs.can_read())
      {
        tpie::log_info() << "\t";
        print_arc(arcs.read());
        tpie::log_info() << "," << std::endl;
      }
    tpie::log_info() << "}";

    arcs.set_position(original_pos);
  }

  void println_file_stream(tpie::file_stream<arc>& arcs)
  {
    print_file_stream(arcs);
    tpie::log_info() << std::endl;
  }
}

#endif // COOM_DATA_PTY_CPP
