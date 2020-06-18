#ifndef COOM_OBDD_CPP
#define COOM_OBDD_CPP

#include <stdint.h>
#include <stdio.h>
#include <tpie/tpie.h>

#include "data.h"
#include "obdd.h"

namespace coom {
  // Member functions in OBDD
  bool OBDD::is_sink()
  {
    return false;
  }

  bool OBDD::has_nodes()
  {
    return false;
  }

  uint64_t OBDD::as_sink()
  {
    throw "Cannot cast to sink-only OBDD";
  }

  OBDD_node OBDD::as_node()
  {
    throw "Cannot cast to node-based OBDD";
  }

  // Member functions in OBDD_sink
  bool OBDD_sink::is_sink()
  {
    return true;
  }

  uint64_t OBDD_sink::as_sink()
  {
    return this->data;
  }


  // Member functions in OBDD_node
  bool OBDD_node::has_nodes()
  {
    return true;
  }

  OBDD_node OBDD_node::as_node()
  {
    return *this;
  }

  tpie::file_stream<node> OBDD_node::open_filestream()
  {
    tpie::file_stream<node> xs;
    xs.open(this->temp_file, tpie::open::compression_normal);
    return xs;
  }

  // Constructors
  OBDD_sink::OBDD_sink(bool value)
  {
    data = create_sink(value);
  }

  OBDD_node::OBDD_node(const std::string & path, bool persist = false)
  {
    negated = false;
    tpie::temp_file tf(path, persist);
    temp_file = tf;
  }
}

#endif // COOM_OBDD_CPP
