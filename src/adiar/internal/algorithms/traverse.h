#ifndef ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
#define ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H

#include <adiar/label.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>

namespace adiar
{
  template<typename dd_t, typename traverse_visitor>
  void traverse(const dd_t &dd, traverse_visitor &visitor)
  {
    node_stream<> in_nodes(dd);
    node_t n = in_nodes.pull();
    ptr_uint64 tgt = n.uid();

    while (!tgt.is_terminal() && !tgt.is_nil()) {
      while (n.uid() < tgt) { n = in_nodes.pull(); }

      adiar_debug(n.uid() == tgt,
                  "Invalid uid chasing; fell out of Decision Diagram");

      tgt = visitor.visit(n);

      adiar_debug((tgt == n.low()) || (tgt == n.high()) || (tgt.is_nil()),
                  "Visitor pointer should be within the diagram or NIL");
    }
    if (!tgt.is_nil()) {
      visitor.visit(tgt.value());
    }
  }

  class traverse_satmin_visitor
  {
  public:
    inline ptr_uint64 visit(const node_t &n)
    {
      // Only pick high, if low is the false terminal
      return n.low().is_false() ? n.high() : n.low();
    }

    inline void visit(const bool /*s*/)
    { }
  };

  class traverse_satmax_visitor
  {
  public:
    inline ptr_uint64 visit(const node_t &n)
    {
      // Pick high as long it is not the false terminal
      return n.high().is_node() || n.high().value() ? n.high() : n.low();
    }

    inline void visit(const bool /*s*/)
    { }
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
