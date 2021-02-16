#ifndef ADIAR_BUILD_H
#define ADIAR_BUILD_H

#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD of only a single sink.
  ///
  /// \param value     The value for the sink
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_sink(bool value);

  bdd bdd_false();
  bdd bdd_true();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_ithvar(label_t label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD for the variable with the given label.
  ///
  /// \param label     The label of the variable
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_nithvar(label_t label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD that is true only when all given labels are set to
  ///        to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_and(const label_file &in_labels);

  // TODO: std::vector variant?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD that is true only when at least one of the given
  ///        labels are set to to true.
  ///
  /// \param in_labels The labels sorted from smallest to highest.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_or(const label_file &in_labels);

  // TODO: std::vector variant?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an BDD that is true when the number of variables set to
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
  ///       BDD changes with the comparator. So, currently it only supports
  ///       equality with the threshold.
  //////////////////////////////////////////////////////////////////////////////
  //template <typename comparator = std::equal_to<uint64_t>()>
  bdd bdd_counter(label_t min_label, label_t max_label, label_t threshold);
}

#endif // ADIAR_BUILD_H
