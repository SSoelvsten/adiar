#ifndef ADIAR_REDUCE_H
#define ADIAR_REDUCE_H

#include <adiar/file.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Applies Reduction Rule 1 to a node (should the node be omitted or
  ///        not?)
  ///
  /// \return Returns the uid of the node to use. This is either the given node,
  ///         one of its children, or a sink.
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<ptr_t(const node_t&)> reduction_rule_t;

  extern const reduction_rule_t reduction_rule_bdd;
  extern const reduction_rule_t reduction_rule_zdd;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based BDD.
  ///
  /// \param arc_file The unreduced bdd in its arc-based representation
  /// \param reduction_rule The reduction rule to apply on nodes
  ///
  /// \return The reduced decision diagram in a node-based representation
  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const arc_file &arc_file,
                   const reduction_rule_t& reduction_rule);
}

#endif // ADIAR_REDUCE_H
