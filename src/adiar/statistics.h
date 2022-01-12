#ifndef ADIAR_STATISTICS_H
#define ADIAR_STATISTICS_H

#include <iostream>
#include <cstddef>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// These numbers are always available, but they are only populated with
  /// actual statistics if Adiar is compiled with certain CMake variables set.
  /// Statistics can be gathered on two levels of detail:
  ///
  /// - If <tt>ADIAR_STATS=ON</tt> then only the low-overhead O(1) statistics
  ///   are gathered.
  ///
  /// - If <tt>ADIAR_STATS_RXTRA=ON</tt> then also the more detailed statistics
  ///   requiring a linear time overhead is gathered.
  //////////////////////////////////////////////////////////////////////////////
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
      size_t sum_node_arcs = 0;
      size_t sum_sink_arcs = 0;

      // (ADIAR_STATS_EXTRA)
      size_t removed_by_rule_1 = 0;
      size_t removed_by_rule_2 = 0;
    } reduce;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain a copy of all statistics gathered.
  ///
  /// \copydoc stats_t
  //////////////////////////////////////////////////////////////////////////////
  stats_t adiar_stats();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Print statistics to an output stream (default std::cout).
  ///
  /// \copydoc stats_t
  //////////////////////////////////////////////////////////////////////////////
  void adiar_printstat(std::ostream &o = std::cout);
}

#endif // ADIAR_STATISTICS_H
