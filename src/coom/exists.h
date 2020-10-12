#ifndef COOM_EXISTS_H
#define COOM_EXISTS_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <stdint.h>

#include "apply.h"
#include "data.h"
#include "quantify.cpp"

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  ///
  /// Creates the OBDD representing whether there exists an assignment to the
  /// variable with the given label, such that the given OBDD evaluates to true.
  ///
  /// \param label     The variable to existentially quantify.
  ///
  /// \param in_nodes  Nodes in reverse topological order of the first OBDD.
  /// \param in_meta   The meta stream related to in_nodes
  ///
  /// \param out_nodes The output stream to send the nodes in reverse
  ///                  topological order.
  /// \param out_nodes The output stream to the related meta information
  //////////////////////////////////////////////////////////////////////////////
  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    return quantify(label, in_nodes, in_meta, or_op, out_nodes, out_meta);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  ///
  /// Does the same as Exists, but allows one to control the state of the
  /// streams at the time of Reduction. For further use, one needs to use
  /// coom::reduce to obtain the resulting OBDD.
  ///
  /// Assumes that the given OBDD in_nodes contains internal nodes and that the
  /// label to be quantified is not at the root with a trivial sink-only result.
  //////////////////////////////////////////////////////////////////////////////
  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<arc_t> &reduce_node_arcs,
              tpie::file_stream<arc_t> &reduce_sink_arcs,
              tpie::file_stream<meta_t> &reduce_meta)
  {
    return quantify(label, in_nodes, in_meta, or_op,
                    reduce_node_arcs, reduce_sink_arcs, reduce_meta);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for variables with given labels.
  ///
  /// Creates the OBDD representing whether there exists an assignment to the
  /// variable with the given label, such that the given OBDD evaluates to true.
  ///
  /// \param labels    The variables to existentially quantify. The variables
  ///                  will be quantified in the order they are given.
  ///
  /// \param in_nodes  Nodes in reverse topological order of the first OBDD.
  /// \param in_meta   The meta stream related to in_nodes
  ///
  /// \param out_nodes The output stream to send the nodes in reverse
  ///                  topological order.
  /// \param out_nodes The output stream to the related meta information
  //////////////////////////////////////////////////////////////////////////////
  void exists(tpie::file_stream<label_t> &labels,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta)
  {
    return quantify(labels, in_nodes, in_meta, or_op, out_nodes, out_meta);
  }
}

#endif // COOM_EXISTS_H
