#ifndef COOM_BUILD_H
#define COOM_BUILD_H

#include <tpie/file_stream.h>

#include <coom/data.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD of only a single sink.
  ///
  /// \param value     The value for the sink
  /// \param out_nodes A (yet empty) file_stream to output to
  //////////////////////////////////////////////////////////////////////////////
  void build_sink(bool value,
                  tpie::file_stream<node_t> &out_nodes,
                  tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  /// \param out_nodes A (yet empty) file_stream to output to
  //////////////////////////////////////////////////////////////////////////////
  void build_x(label_t label,
               tpie::file_stream<node_t> &out_nodes,
               tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  /// \param out_nodes A (yet empty) file_stream to output to
  //////////////////////////////////////////////////////////////////////////////
  void build_not_x(label_t label,
                   tpie::file_stream<node_t> &out_nodes,
                   tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true only when all given labels are set to
  ///        to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  /// \param out_nodes A (yet empty) file_stream to output to
  //////////////////////////////////////////////////////////////////////////////
  void build_and(tpie::file_stream<label_t> &in_labels,
                 tpie::file_stream<node_t> &out_nodes,
                 tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true only when at least one of the given
  ///        labels are set to to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  /// \param out_nodes A (yet empty) file_stream to output to
  //////////////////////////////////////////////////////////////////////////////
  void build_or(tpie::file_stream<label_t> &in_labels,
                tpie::file_stream<node_t> &out_nodes,
                tpie::file_stream<meta_t> &out_meta);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true when the number of variables set to
  ///        true satisfy a given threshold.
  ///
  /// \param min_label The minimum label (inclusive) to start counting from
  /// \param max_label The maximum label (inclusive) to end counting at
  /// \param threshold The threshold number of variables set to true
  /// \param out_nodes A (yet empty) file_stream to output to
  ///
  /// \param comparator The comparator whether a counting above, below, or equal
  ///                   to the threshold should be accepted or not.
  ///
  /// TODO: Currently the comparator is disabled, since the form of the a reduced
  ///       OBDD changes with the comparator. So, currently it only supports
  ///       equality with the threshold.
  //////////////////////////////////////////////////////////////////////////////
  //template <typename comparator = std::equal_to<uint64_t>()>
  void build_counter(label_t min_label, label_t max_label,
                     uint64_t threshold,
                     tpie::file_stream<node_t> &out_nodes,
                     tpie::file_stream<meta_t> &out_meta);
}

#endif // COOM_BUILD_H
