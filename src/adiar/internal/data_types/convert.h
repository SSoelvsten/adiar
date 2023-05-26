#ifndef ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
#define ADIAR_INTERNAL_DATA_TYPES_CONVERT_H

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *low* arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc low_arc_of(const node &n)
  {
    return { n.uid()/*.with(false)*/, n.low() };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *high* arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc high_arc_of(const node &n)
  {
    return { n.uid().with(true), n.high() };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Combine two arcs with the same source into a node.
  ///
  /// \param low  The low arc
  /// \param high The high arc
  //////////////////////////////////////////////////////////////////////////////
  inline node node_of(const arc &low, const arc &high)
  {
    adiar_debug(essential(low.source()) == essential(high.source()),
                "Source are the same origin");

    adiar_debug(low.out_idx()  == 0u, "Out-index is correct on low arc");
    adiar_debug(high.out_idx() == 1u,  "Out-index is correct on high arc");

    adiar_debug(!low.target().is_node()  || low.target().out_idx()  == 0u, "Out-index is empty in low target");
    adiar_debug(!high.target().is_node() || high.target().out_idx() == 0u, "Out-index is empty in high target");

    adiar_debug(low.source().is_flagged()  == false, "Source is not flagged on low arc");
    adiar_debug(high.source().is_flagged() == false, "Source is not flagged on high arc");

    return node(essential(low.source()), low.target(), high.target());
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
