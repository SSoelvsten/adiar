#ifndef ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
#define ADIAR_INTERNAL_DATA_TYPES_CONVERT_H

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *low* arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t low_arc_of(const node_t &n)
  {
    return { n.uid(), n.low() };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *high* arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t high_arc_of(const node_t &n)
  {
    return { flag(n.uid()), n.high() };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Combine two arcs with the same source into a node.
  ///
  /// \param low  The low arc
  /// \param high The high arc
  //////////////////////////////////////////////////////////////////////////////
  inline node_t node_of(const arc_t &low, const arc_t &high)
  {
    adiar_debug(unflag(low.source()) == unflag(high.source()), "Arcs are not of the same node");
    adiar_debug(!low.is_high(), "High flag is not set on low child");
    adiar_debug(high.is_high(), "High flag is set on high child");

    return node(low.source(), low.target(), high.target());
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
