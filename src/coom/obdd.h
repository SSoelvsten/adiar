#ifndef COOM_OBDD_H
#define COOM_OBDD_H

#include <stdint.h>
#include <tpie/tpie.h>

#include "data.h"

namespace coom {
  class OBDD_sink;
  class OBDD_node;

  class OBDD
  {
  public:
    virtual bool is_sink();
    virtual bool has_nodes();

    virtual uint64_t as_sink();
    virtual OBDD_node as_node();

    static tpie::file_stream<node> open_filestream(const char * filename);
  };

  class OBDD_sink : public OBDD
  {
  private:
    uint64_t data;

  public:
    OBDD_sink(bool value);

    bool is_sink();
    uint64_t as_sink();
  };

  class OBDD_node : public OBDD
  {
  private:
    tpie::temp_file temp_file;

  public:
    bool negated;

    OBDD_node(const std::string & path, bool persist);

    bool has_nodes();
    OBDD_node as_node();

    tpie::file_stream<node> open_filestream();
  };
}

#endif // COOM_OBDD_H
