#ifndef COOM_RESTRICT_H
#define COOM_RESTRICT_H

#include <tpie/file_stream.h>

#include <coom/data.h>
#include <coom/assignment.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a given node-based OBDD based on a partial assignment.
  ///
  /// \param nodes_in   input stream of OBDD; sorted in reverse topological
  ///                   order.
  ///
  /// \param assignment Assignment sorted in order of label (and expected
  ///                   unique)
  ///
  /// \param nodes_out  (empty) output stream for result OBDD
  //////////////////////////////////////////////////////////////////////////////
  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                tpie::file_stream<assignment_t> &in_assignment,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the unreduced intermediate result for restricting an OBDD by
  ///        an assignment.
  ///
  /// Does the same as Restrict above given a single coom::node_t output stream.
  /// If the intermediate result isn't a mere sink, then the output is solely
  /// placed in reduce_node_arcs and reduce_sink_arcs, which will have to go
  /// through coom::reduce, before one can use them further.
  ///
  /// The reason one would care to use this function is only to be able to
  /// either record statistics about the intermediate result or to be able to
  /// close one or more of the two input streams. That is, with the version
  /// above the C++ compiler isn't able to infer that the input streams may be
  /// dead variables (i.e. they can be closed) before one starts the next
  /// reduction. So, one can use this function to explicitly close the input
  /// streams and with that limit the concurrent memory usage.
  ///
  /// \param nodes_in   input stream of OBDD; sorted in reverse topological
  ///                   order.
  ///
  /// \param assignment Assignment sorted in order of label (and expected
  ///                   unique)
  ///
  /// \param out_nodes The output stream to send the nodes in reverse topological
  ///                  order.
  ///
  /// \param reduce_node_arcs The output stream to send the transposed node-arcs
  ///                         for the later reduce.
  /// \param reduce_sink_arcs The output stream to send the sink-arcs for the
  ///                         later reduce.
  //////////////////////////////////////////////////////////////////////////////
  void restrict(tpie::file_stream<node_t> &in_nodes,
                tpie::file_stream<meta_t> &in_meta,
                tpie::file_stream<assignment_t> &in_assignment,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<arc_t> &reduce_node_arcs,
                tpie::file_stream<arc_t> &reduce_sink_arcs,
                tpie::file_stream<meta_t> &reduce_meta);
}

#endif // COOM_RESTRICT_H
