#ifndef ADIAR_STATISTICS_H
#define ADIAR_STATISTICS_H

#include <iostream>
#include <cstddef>

#include <adiar/internal/cnl.h>

namespace adiar
{
  // Internal/external memory (ADIAR_STATS)
  struct memory_t
  {
    uintwide_t lpq_internal = 0;
    uintwide_t lpq_external = 0;
  };

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
    // Count
    struct count_t : public memory_t
    { } count;

    // Equality Checking statistics  (ADIAR_STATS)
    struct equality_t : public memory_t
    {
      // Early termination cases
      uintwide_t exit_on_same_file = 0;
      uintwide_t exit_on_nodecount = 0;
      uintwide_t exit_on_varcount = 0;
      uintwide_t exit_on_sinkcount = 0;
      uintwide_t exit_on_levels_mismatch = 0;

      // Statistics on non-trivial cases
      struct slow_t
      {
        uintwide_t runs = 0;
        uintwide_t exit_on_root = 0;
        uintwide_t exit_on_processed_on_level = 0;
        uintwide_t exit_on_children = 0;
      } slow_check;

      struct fast_t
      {
        uintwide_t runs = 0;
        uintwide_t exit_on_mismatch = 0;
      } fast_check;
    } equality;

    // If-then-else
    struct if_else_t : public memory_t
    { } if_else;

    // Intercut
    struct intercut_t : public memory_t
    { } intercut;

    // Levelized Priority Queue (ADIAR_STATS_EXTRA)
    struct priority_queue_t
    {
      uintwide_t push_bucket = 0;
      uintwide_t push_overflow = 0;

      uintwide_t sum_predicted_max_size = 0;
      uintwide_t sum_actual_max_size = 0;
    } priority_queue;

    // Product construction
    struct product_construction_t : public memory_t
    { } product_construction;

    // Quantification
    struct quantify_t : public memory_t
    { } quantify;

    // Reduce
    struct reduce_t : public memory_t
    {
      // (ADIAR_STATS)
      uintwide_t sum_node_arcs = 0;
      uintwide_t sum_sink_arcs = 0;

      // (ADIAR_STATS_EXTRA)
      uintwide_t removed_by_rule_1 = 0;
      uintwide_t removed_by_rule_2 = 0;
    } reduce;

    // Substitution
    struct substitute_t : public memory_t
    { } substitute;

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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Resets all statistics to default value.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_statsreset();
}

#endif // ADIAR_STATISTICS_H
