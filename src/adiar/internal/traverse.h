#ifndef ADIAR_INTERNAL_TRAVERSE_H
#define ADIAR_INTERNAL_TRAVERSE_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

namespace adiar
{
  template<typename dd_t, typename traverse_visitor>
  void traverse(const dd_t &dd, traverse_visitor &visitor)
  {
    node_stream<> in_nodes(dd);
    node_t n = in_nodes.pull();
    ptr_t tgt = n.uid;

    while (!is_sink(tgt) && !is_nil(tgt)) {
      while (n.uid < tgt) { n = in_nodes.pull(); }

      adiar_debug(n.uid == tgt,
                  "Invalid uid chasing; fell out of Decision Diagram");

      tgt = visitor.visit(n);

      adiar_debug((tgt == n.low) || (tgt == n.high) || (is_nil(tgt)),
                  "Visitor pointer should be within the diagram or NIL");
    }
    if (!is_nil(tgt)) {
      visitor.visit(value_of(tgt));
    }
  }

  class traverse_satmin_visitor
  {
  public:
    inline ptr_t visit(const node_t &n)
    {
      // Only pick high, if low is the false sink
      return is_false(n.low) ? n.high : n.low;
    }

    inline void visit(const bool /*s*/)
    { }
  };

  class traverse_satmax_visitor
  {
  public:
    inline ptr_t visit(const node_t &n)
    {
      // Pick high as long it is not the false sink
      return is_node(n.high) || value_of(n.high) ? n.high : n.low;
    }

    inline void visit(const bool /*s*/)
    { }
  };
}

#endif // ADIAR_INTERNAL_TRAVERSE_H
