#ifndef COOM_QUANTIFY_H
#define COOM_QUANTIFY_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <stdint.h>

#include <coom/data.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Quantify an OBDD for a variable with a given label and operator.
  ///
  /// Creates the OBDD representing, in which the variable is removed and the
  /// sink-value is determined by the operator applied on both possibilities.
  ///
  /// Assumes that the given operator is commutative.
  ///
  /// \param label     The variable to existentially quantify.
  ///
  /// \param in_nodes  Nodes in reverse topological order of the first OBDD.
  /// \param in_meta   The meta stream related to in_nodes
  ///
  /// \param op        The operator to quantify with
  ///
  /// \param out_nodes The output stream to send the nodes in reverse
  ///                  topological order.
  /// \param out_nodes The output stream to the related meta information
  //////////////////////////////////////////////////////////////////////////////
  void quantify(label_t label,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Quantify an OBDD for a variable with a given label and operator.
  ///
  /// Does the same as Quantify, but allows one to control the state of the
  /// streams at the time before the following Reduce. For further use, one
  /// needs to use coom::reduce to obtain the resulting OBDD.
  ///
  /// Assumes that the given OBDD in_nodes contains internal nodes and that the
  /// label to be quantified is not at the root with a trivial sink-only result.
  //////////////////////////////////////////////////////////////////////////////
  void quantify(label_t label,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<arc_t> &reduce_node_arcs,
                tpie::file_stream<arc_t> &reduce_sink_arcs,
                tpie::file_stream<meta_t> &reduce_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Quantify an OBDD for variables with given labels and an operator.
  ///
  /// Does the same as Quantify above, but applies the quantification (in-order
  /// as given) of the variables.
  ///
  /// \param labels    The variables to existentially quantify. The variables
  ///                  will be quantified in the order they are given.
  ///
  /// \param in_nodes  Nodes in reverse topological order of the first OBDD.
  /// \param in_meta   The meta stream related to in_nodes
  ///
  /// \param op        The operator to quantify with
  ///
  /// \param out_nodes The output stream to send the nodes in reverse
  ///                  topological order.
  /// \param out_nodes The output stream to the related meta information
  //////////////////////////////////////////////////////////////////////////////
  void quantify(tpie::file_stream<label_t> &labels,
                tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                const bool_op &op,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'or' operator.
  //////////////////////////////////////////////////////////////////////////////
  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'or' operator.
  //////////////////////////////////////////////////////////////////////////////
  void exists(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<arc_t> &reduce_node_arcs,
              tpie::file_stream<arc_t> &reduce_sink_arcs,
              tpie::file_stream<meta_t> &reduce_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'or' operator.
  //////////////////////////////////////////////////////////////////////////////
  void exists(tpie::file_stream<label_t> &labels,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'and' operator.
  //////////////////////////////////////////////////////////////////////////////
  void forall(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'and' operator.
  //////////////////////////////////////////////////////////////////////////////
  void forall(label_t label,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<arc_t> &reduce_node_arcs,
              tpie::file_stream<arc_t> &reduce_sink_arcs,
              tpie::file_stream<meta_t> &reduce_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an OBDD for a variable with a given label.
  ///
  /// Alias for quantify with the 'and' operator.
  //////////////////////////////////////////////////////////////////////////////
  void forall(tpie::file_stream<label_t> &labels,
              tpie::file_stream<node_t> &in_nodes,
              tpie::file_stream<meta_t> &in_meta,
              tpie::file_stream<node_t> &out_nodes,
              tpie::file_stream<meta_t> &out_meta);
}

#endif // COOM_QUANTIFY_H
