#ifndef ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
#define ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  Traverse Algorithm
  // ====================
  //
  // Traversal of a Decision Diagram together with callbacks for a Visitor to
  // follow along and pick whereto go.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  template<typename Dd, typename Visitor>
  void traverse(const Dd &dd, Visitor &visitor)
  {
    node_stream<> in_nodes(dd);

    typename Dd::node_type n      = in_nodes.pull();
    typename Dd::pointer_type tgt = n.uid();

    while (!tgt.is_terminal() && !tgt.is_nil()) {
      while (n.uid() < tgt) { n = in_nodes.pull(); }

      adiar_assert(n.uid() == tgt,
                   "Invalid uid chasing; fell out of Decision Diagram");

      tgt = visitor.visit(n);

      adiar_assert((tgt == n.low()) || (tgt == n.high()) || (tgt.is_nil()),
                   "Visitor pointer should be a child or nil");
    }
    if (!tgt.is_nil()) {
      visitor.visit(tgt.value());
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Implementation of Visitor logic, traversing the lexicographically
  ///        smallest assignment.
  ///
  /// \details Picks the high arc as long as it does not goto `false`.
  //////////////////////////////////////////////////////////////////////////////
  class traverse_satmin_visitor
  {
  public:
    static constexpr bool default_direction = false;

    inline node::pointer_type visit(const node &n)
    {
      return n.low().is_false() ? n.high() : n.low();
    }

    inline void visit(const bool/*t*/)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Implementation of Visitor logic, traversing the lexicographically
  ///        largest assignment.
  ///
  /// \details Picks the low arc as long as it does not goto the `false`.
  //////////////////////////////////////////////////////////////////////////////
  class traverse_satmax_visitor
  {
  public:
    static constexpr bool default_direction = true;

    inline node::pointer_type visit(const node &n)
    {
      return n.high().is_false() ? n.low() : n.high();
    }

    inline void visit(const bool/*t*/)
    { }
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_TRAVERSE_H
