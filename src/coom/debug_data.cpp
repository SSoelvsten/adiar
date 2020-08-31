#ifndef COOM_DEBUG_DATA_CPP
#define COOM_DEBUG_DATA_CPP

#include <stdint.h>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/tpie_log.h>

#include "data.h"

#include "debug_data.h"

namespace coom { namespace debug {
    void print_nil_ptr()
    {
#if COOM_DEBUG
      tpie::log_info() << "[* NIL *]";
#endif
    }

    void print_node_ptr([[maybe_unused]] ptr_t n)
    {
#if COOM_DEBUG
      tpie::log_info() << "[* " << label_of(n) << " : " << id_of(n) << " *]";
#endif
    }

    void print_sink_ptr([[maybe_unused]] ptr_t n)
    {
#if COOM_DEBUG
      tpie::log_info() << "[* " << value_of(n) << " *]";
#endif
    }

    void print_child([[maybe_unused]] ptr_t n)
    {
#if COOM_DEBUG
      if (is_nil(n))
        {
          print_nil_ptr();
        }
      else if (is_sink_ptr(n))
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
#endif
    }

    void print_node([[maybe_unused]] const node& n)
    {
#if COOM_DEBUG
      tpie::log_info() << "([ ";
      if (is_sink(n))
        {
          bool value = value_of(n);
          tpie::log_info() << value;
        }
      else
        {
          label_t label = label_of(n);
          id_t id = id_of(n);
          tpie::log_info() << label << " , " << id;
        }
      tpie::log_info() << " ], ";
      print_child(n.low);
      tpie::log_info() << ", ";
      print_child(n.high);
      tpie::log_info() << " )";
#endif
    }

    void println_node([[maybe_unused]] const node& n)
    {
#if COOM_DEBUG
      print_node(n);
      tpie::log_info() << std::endl;
#endif
    }

    void print_arc([[maybe_unused]] const arc& a)
    {
#if COOM_DEBUG
      tpie::log_info() << "( ";
      print_child(a.source);
      tpie::log_info() << " , ";
      if (is_flagged(a.source))
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
#endif
    }

    void println_arc([[maybe_unused]] const arc& a)
    {
#if COOM_DEBUG
      print_arc(a);
      tpie::log_info() << std::endl;
#endif
    }

    void print_file_stream([[maybe_unused]] tpie::file_stream<node_t>& nodes,
                           [[maybe_unused]] std::string name)
    {
#if COOM_DEBUG
      auto original_pos = nodes.get_position();
      nodes.seek(0);

      tpie::log_info() << name << " {" << std::endl;
      while (nodes.can_read())
        {
          tpie::log_info() << "\t";
          print_node(nodes.read());
          tpie::log_info() << "," << std::endl;
        }
      tpie::log_info() << "}";

      nodes.set_position(original_pos);
#endif
    }

    void print_file_stream([[maybe_unused]] tpie::file_stream<node_t>& nodes)
    {
      print_file_stream(nodes, "file_stream<node_t>");
    }

    void println_file_stream([[maybe_unused]] tpie::file_stream<node_t>& nodes,
                             [[maybe_unused]] std::string name)
    {
#if COOM_DEBUG
      print_file_stream(nodes, name);
      tpie::log_info() << std::endl;
#endif
    }

    void println_file_stream([[maybe_unused]] tpie::file_stream<node_t>& nodes)
    {
      println_file_stream(nodes, "file_stream<node_t>");
    }

    void print_file_stream([[maybe_unused]] tpie::file_stream<arc_t>& arcs,
                           [[maybe_unused]] std::string name)
    {
#if COOM_DEBUG
      auto original_pos = arcs.get_position();
      arcs.seek(0);

      tpie::log_info() << name <<" {" << std::endl;
      while (arcs.can_read())
        {
          tpie::log_info() << "\t";
          print_arc(arcs.read());
          tpie::log_info() << "," << std::endl;
        }
      tpie::log_info() << "}";

      arcs.set_position(original_pos);
#endif
    }

    void print_file_stream([[maybe_unused]] tpie::file_stream<arc_t>& arcs)
    {
      print_file_stream(arcs, "file_stream<arc_t>");
    }

    void println_file_stream([[maybe_unused]] tpie::file_stream<arc_t>& arcs,
                             [[maybe_unused]] std::string name)
    {
#if COOM_DEBUG
      print_file_stream(arcs, name);
      tpie::log_info() << std::endl;
#endif
    }

    void println_file_stream([[maybe_unused]] tpie::file_stream<arc_t>& arcs)
    {
      println_file_stream(arcs, "file_stream<arc_t>");
    }
  }
}

#endif // COOM_DEBUG_DATA_CPP
