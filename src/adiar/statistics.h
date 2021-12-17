////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#ifndef ADIAR_STATISTICS_H
#define ADIAR_STATISTICS_H

#include <iostream>
#include <cstddef>

namespace adiar
{
  struct stats_t
  {
    // Equality Checking statistics  (ADIAR_STATS)
    struct equality_t
    {
      // Early termination cases
      size_t exit_on_same_file = 0;
      size_t exit_on_nodecount = 0;
      size_t exit_on_varcount = 0;
      size_t exit_on_levels_mismatch = 0;

      // Statistics on non-trivial cases
      struct slow_t
      {
        size_t runs = 0;
        size_t exit_on_root = 0;
        size_t exit_on_processed_on_level = 0;
        size_t exit_on_children = 0;
      } slow_check;

      struct fast_t
      {
        size_t runs = 0;
        size_t exit_on_mismatch = 0;
      } fast_check;
    } equality;

    // Levelized Priority Queue (ADIAR_STATS_EXTRA)
    struct priority_queue_t
    {
      size_t push_bucket;
      size_t push_overflow;
    } priority_queue;

    // Reduce
    struct reduce_t
    {
      // (ADIAR_STATS)
      size_t sum_node_arcs;
      size_t sum_sink_arcs;

      // (ADIAR_STATS_EXTRA)
      size_t removed_by_rule_1;
      size_t removed_by_rule_2;
    } reduce;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain statistics
  //////////////////////////////////////////////////////////////////////////////
  stats_t adiar_stats();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Print statistics to an output stream (default std::cout)
  //////////////////////////////////////////////////////////////////////////////
  void adiar_printstat(std::ostream &o = std::cout);
}

#endif // ADIAR_STATISTICS_H
