#include "elem.h"

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/traverse.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  template<typename visitor>
  class zdd_sat_label_writer_visitor
  {
    visitor __visitor;

    bool has_elem = false;

    label_file lf;
    label_writer lw;

  public:
    zdd_sat_label_writer_visitor() : lw(lf) { }

    bool visit(const node_t &n)
    {
      const bool go_high = __visitor.visit(n);

      if (go_high) {
        lw << label_of(n);
      }

      return go_high;
    }

    void visit(const bool s)
    {
      __visitor.visit(s);
      has_elem = s;
    }

    const std::optional<label_file> get_result() const
    {
      if (has_elem) { return lf; } else { return std::nullopt; }
    }
  };

  std::optional<label_file> zdd_minelem(const zdd &A)
  {
    zdd_sat_label_writer_visitor<traverse_satmin_visitor> v;
    traverse(A, v);
    return v.get_result();
  }

  class zdd_satmax_visitor
  {
  public:
    inline bool visit(const node_t &n) {
      adiar_debug(!is_sink(n.high) || value_of(n.high), "high sinks are never false");
      return true;
    }

    inline void visit(const bool /*s*/)
    { }
  };

  std::optional<label_file> zdd_maxelem(const zdd &A)
  {
    zdd_sat_label_writer_visitor<zdd_satmax_visitor> v;
    traverse(A, v);
    return v.get_result();
  }
}
