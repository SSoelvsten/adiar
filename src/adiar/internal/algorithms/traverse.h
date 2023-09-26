#ifndef ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
#define ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H

#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  template<typename dd_t, typename traverse_visitor>
  void traverse(const dd_t &dd, traverse_visitor &visitor)
  {
    node_stream<> in_nodes(dd);

    typename dd_t::node_t n   = in_nodes.pull();
    typename dd_t::ptr_t  tgt = n.uid();

    while (!tgt.is_terminal() && !tgt.is_nil()) {
      while (n.uid() < tgt) { n = in_nodes.pull(); }

      adiar_assert(n.uid() == tgt,
                   "Invalid uid chasing; fell out of Decision Diagram");

      tgt = visitor.visit(n);

      adiar_assert((tgt == n.low()) || (tgt == n.high()) || (tgt.is_nil()),
                   "Visitor pointer should be within the diagram or nil");
    }
    if (!tgt.is_nil()) {
      visitor.visit(tgt.value());
    }
  }

  class traverse_satmin_visitor
  {
  public:
    static constexpr bool default_direction = false;

    inline ptr_uint64 visit(const node &n)
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
    static constexpr bool default_direction = true;

    inline ptr_uint64 visit(const node &n)
    {
      // Pick high as long it is not the false terminal
      return n.high().is_node() || n.high().value() ? n.high() : n.low();
    }

    inline void visit(const bool /*s*/)
    { }
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
