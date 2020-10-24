#ifndef COOM_BUILD_H
#define COOM_BUILD_H

#include <tpie/file_stream.h>

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD of only a single sink.
  ///
  /// \param value     The value for the sink
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_sink(bool value);

  node_file bdd_false();
  node_file bdd_true();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_ithvar(label_t label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_nithvar(label_t label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true only when all given labels are set to
  ///        to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_and(const label_file &in_labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true only when at least one of the given
  ///        labels are set to to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  //////////////////////////////////////////////////////////////////////////////
  node_file bdd_or(const label_file &in_labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an OBDD that is true when the number of variables set to
  ///        true satisfy a given threshold.
  ///
  /// \param min_label The minimum label (inclusive) to start counting from
  /// \param max_label The maximum label (inclusive) to end counting at
  /// \param threshold The threshold number of variables set to true
  ///
  /// \param comparator The comparator whether a counting above, below, or equal
  ///                   to the threshold should be accepted or not.
  ///
  /// TODO: Currently the comparator is disabled, since the form of the a reduced
  ///       OBDD changes with the comparator. So, currently it only supports
  ///       equality with the threshold.
  //////////////////////////////////////////////////////////////////////////////
  //template <typename comparator = std::equal_to<uint64_t>()>
  node_file bdd_counter(label_t min_label, label_t max_label, uint64_t threshold);
}

#endif // COOM_BUILD_H
