#ifndef ADIAR_STATISTICS_H
#define ADIAR_STATISTICS_H

#include <iostream>
#include <cstddef>

#include <adiar/internal/cnl.h>

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
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Levelized Priority Queue (ADIAR_STATS_EXTRA)
    //////////////////////////////////////////////////////////////////////////////
    struct levelized_priority_queue_t
    {
      // ADIAR_STATS_EXTRA
      uintwide_t push_bucket = 0;
      uintwide_t push_overflow = 0;

      uintwide_t sum_predicted_max_size = 0;
      uintwide_t sum_actual_max_size = 0;

      double sum_max_size_ratio = 0.0;
      size_t sum_destructors = 0;
    } levelized_priority_queue;

    //////////////////////////////////////////////////////////////////////////////
    struct __alg_base
    {
      struct __lpq : public levelized_priority_queue_t
      {
        // ADIAR_STATS
        uintwide_t unbucketed = 0;
        uintwide_t internal = 0;
        uintwide_t external = 0;
      } lpq;
    };

    //////////////////////////////////////////////////////////////////////////////
    // Top-down Sweeps

    // Count
    struct count_t : public __alg_base
    { } count;

    // Equality Checking statistics (ADIAR_STATS)
    struct equality_t : public __alg_base
    {
      // Early termination cases
      uintwide_t exit_on_same_file = 0;
      uintwide_t exit_on_nodecount = 0;
      uintwide_t exit_on_varcount = 0;
      uintwide_t exit_on_terminalcount = 0;
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
    struct if_else_t : public __alg_base
    { } if_else;

    // Intercut
    struct intercut_t : public __alg_base
    { } intercut;

    // Product construction
    struct product_construction_t : public __alg_base
    { } product_construction;

    // Quantification
    struct quantify_t : public __alg_base
    { } quantify;

    // Substitution
    struct substitute_t : public __alg_base
    { } substitute;

    //////////////////////////////////////////////////////////////////////////////
    // Bottom-up Sweeps

    // Reduce
    struct reduce_t : public __alg_base
    {
      // (ADIAR_STATS)
      uintwide_t sum_node_arcs = 0;
      uintwide_t sum_terminal_arcs = 0;

      // (ADIAR_STATS_EXTRA)
      uintwide_t removed_by_rule_1 = 0;
      uintwide_t removed_by_rule_2 = 0;
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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Resets all statistics to default value.
  //////////////////////////////////////////////////////////////////////////////
  void adiar_statsreset();
}

#endif // ADIAR_STATISTICS_H
