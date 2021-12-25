#ifndef ADIAR_INTERNAL_TRAVERSE_H
#define ADIAR_INTERNAL_TRAVERSE_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

namespace adiar
{
  typedef std::function<void(label_t, bool)> sat_trav_callback_t;

  template<typename dd_t, typename traverse_visitor>
  void traverse(const dd_t &dd,
                traverse_visitor &visitor)
  {
    node_stream<> in_nodes(dd);
    node_t n_curr = in_nodes.pull();
    ptr_t  n_next = n_curr.uid;

    while (!is_sink(n_next)) {
      // forward to n_next
      while (n_curr.uid < n_next) { n_curr = in_nodes.pull(); }

      adiar_debug(n_curr.uid == n_next,
                  "Invalid uid chasing; fell out of Decision Diagram");

      const bool go_high = visitor.visit(n_curr);

      n_next = go_high ? n_curr.high : n_curr.low;
    }
    visitor.visit(value_of(n_next));
  }

  class traverse_satmin_visitor
  {
  public:
    inline bool visit(const node_t &n)
    {
      // Only pick high, if low is the false sink
      return is_sink(n.low) && !value_of(n.low);
    }

    inline void visit (const bool /*s*/)
    { }
  };

  class traverse_satmax_visitor
  {
  public:
    inline bool visit(const node_t &n)
    {
      // Pick high as long it is not the false sink
      return is_node(n.high) || value_of(n.high);
    }

    inline void visit(const bool /*s*/)
    { }
  };
}

#endif // ADIAR_INTERNAL_TRAVERSE_H
