#include "statistics.h"

#include <adiar/internal/pred.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/reduce.h>
#include <adiar/internal/count.h>
#include <adiar/internal/product_construction.h>
#include <adiar/internal/quantify.h>
#include <adiar/bdd/if_then_else.h>
#include <adiar/internal/substitution.h>
#include <adiar/internal/intercut.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert a wide integer to a string and push to an output stream.
  ///
  /// TODO: move into 'adiar/internal/cnl.h'
  //////////////////////////////////////////////////////////////////////////////
  inline std::ostream& operator<< (std::ostream& os, const uintwide_t &s)
  {
    return os << to_string(s);
  }

  // Define the available function
  stats_t adiar_stats()
  {
#ifndef ADIAR_STATS
    std::cerr << "Statistics not gathered. Please compile with 'ADIAR_STATS' or 'ADIAR_STATS_EXTRA'" << std::endl;
#endif

    return {
      stats_count,
      stats_equality,
      stats_if_else,
      stats_intercut,
      stats_priority_queue,
      stats_product_construction,
      stats_quantify,
      stats_reduce,
      stats_substitute,
    };
  }

  // Helper functions for pretty printing (UNIX)
  inline std::ostream& bold_on(std::ostream& os)  { return os << "\e[1m"; }
  inline std::ostream& bold_off(std::ostream& os) { return os << "\e[0m"; }
  inline std::ostream& percent(std::ostream& os)  { return os << "%"; }
  inline std::ostream& indent(std::ostream& os)   { return os << "  "; }
  inline std::ostream& endl(std::ostream& os)     { return os << std::endl; }

  void adiar_printstat(std::ostream &o)
  {
    o << bold_on << "Adiar statistics" << bold_off << endl;
    o << endl;
#ifndef ADIAR_STATS
    o << indent << "Not gathered; please compile with 'ADIAR_STATS' and/or 'ADIAR_STATS_EXTRA'." << endl;
#else
    o << std::fixed << std::setprecision(2);

    o << indent << bold_on << "Equality checking" << bold_off << " (trace)" << endl;
    o << indent << indent << "same file               " << indent << stats_equality.exit_on_same_file << endl;
    o << indent << indent << "node count              " << indent << stats_equality.exit_on_nodecount << endl;
    o << indent << indent << "var count               " << indent << stats_equality.exit_on_varcount << endl;
    o << indent << indent << "sink count              " << indent << stats_equality.exit_on_sinkcount << endl;
    o << indent << indent << "levels mismatch         " << indent << stats_equality.exit_on_levels_mismatch << endl;
    o << endl;
    o << indent << indent << "O(sort(N)) algorithm    " << endl;
    o << indent << indent << indent << "runs                    " << stats_equality.slow_check.runs << endl;
    o << indent << indent << indent << "root                    " << stats_equality.slow_check.exit_on_root << endl;
    o << indent << indent << indent << "requests on a level     " << stats_equality.slow_check.exit_on_processed_on_level << endl;
    o << indent << indent << indent << "child violation         " << stats_equality.slow_check.exit_on_children << endl;
    o << endl;
    o << indent << indent << "O(N/B) algorithm" << endl;
    o << indent << indent << indent << "runs                    " << stats_equality.fast_check.runs << endl;
    o << indent << indent << indent << "node mismatch           " << stats_equality.fast_check.exit_on_mismatch << endl;
    o << endl;

#ifdef ADIAR_STATS_EXTRA
    o << indent << bold_on << "Levelized Priority Queue" << bold_off << endl;

    uintwide_t total_pushes = stats_priority_queue.push_bucket + stats_priority_queue.push_overflow;
    o << indent << indent << "pushes to bucket        " << indent << stats_priority_queue.push_bucket
      << " = " << percent_frac(stats_priority_queue.push_bucket, total_pushes) << percent << endl;
    o << indent << indent << "pushes to overflow      " << indent << stats_priority_queue.push_overflow
      << " = " << percent_frac(stats_priority_queue.push_overflow, total_pushes) << percent << endl;
    o << endl;

    o << indent << indent << "max size precision ratio" << indent
      << stats_priority_queue.sum_actual_max_size << " / " << stats_priority_queue.sum_predicted_max_size
      << " = " << percent_frac(stats_priority_queue.sum_actual_max_size, stats_priority_queue.sum_predicted_max_size) << percent
      << endl;

    o << endl;

#endif
    uintwide_t total_arcs = stats_reduce.sum_node_arcs + stats_reduce.sum_sink_arcs;
    o << indent << bold_on << "Reduce" << bold_off << endl;

    o << indent << indent << "input size              " << indent << total_arcs << " arcs = " << total_arcs / 2 << " nodes" << endl;

    o << indent << indent << indent << "node arcs:            " << indent
      << stats_reduce.sum_node_arcs << " = " << percent_frac(stats_reduce.sum_node_arcs, total_arcs) << percent << endl;

    o << indent << indent << indent << "sink arcs:            " << indent
      << stats_reduce.sum_sink_arcs << " = " << percent_frac(stats_reduce.sum_sink_arcs, total_arcs) << percent << endl;

#ifdef ADIAR_STATS_EXTRA
    uintwide_t total_removed = stats_reduce.removed_by_rule_1 + stats_reduce.removed_by_rule_2;
    o << indent << indent << "nodes removed           " << indent
      << total_removed << " = " << percent_frac(total_removed, total_arcs) << percent << endl;

    if (total_removed > 0) {
      o << indent << indent << indent << "rule 1:               " << indent
        << stats_reduce.removed_by_rule_1 << " = " << percent_frac(stats_reduce.removed_by_rule_1, total_removed) << percent << endl;

      o << indent << indent << indent << "rule 2:               " << indent
        << stats_reduce.removed_by_rule_2 << " = " << percent_frac(stats_reduce.removed_by_rule_2, total_removed) << percent << endl;
    }
    o << endl;
#endif

    o << indent << bold_on << "Type of auxilliary data structures" << bold_off << endl;
    uintwide_t total_reduce = stats_reduce.lpq_internal + stats_reduce.lpq_external;
    o << indent << indent << "Reduce" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_reduce.lpq_internal << " = " << percent_frac(stats_reduce.lpq_internal, total_reduce) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_reduce.lpq_external << " = " << percent_frac(stats_reduce.lpq_external, total_reduce) << percent << endl;

    uintwide_t total_count = stats_count.lpq_internal + stats_count.lpq_external;
    o << indent << indent << "Count" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_count.lpq_internal << " = " << percent_frac(stats_count.lpq_internal, total_count) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_count.lpq_external << " = " << percent_frac(stats_count.lpq_external, total_count) << percent << endl;

    uintwide_t total_product = stats_product_construction.lpq_internal + stats_product_construction.lpq_external;
    o << indent << indent << "Product construction" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_product_construction.lpq_internal << " = " << percent_frac(stats_product_construction.lpq_internal, total_product) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_product_construction.lpq_external << " = " << percent_frac(stats_product_construction.lpq_external, total_product) << percent << endl;

    uintwide_t total_quantify = stats_quantify.lpq_internal + stats_quantify.lpq_external;
    o << indent << indent << "Quantification" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_quantify.lpq_internal << " = " << percent_frac(stats_quantify.lpq_internal, total_quantify) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_quantify.lpq_external << " = " << percent_frac(stats_quantify.lpq_external, total_quantify) << percent << endl;

    uintwide_t total_if_else = stats_if_else.lpq_internal + stats_if_else.lpq_external;
    o << indent << indent << "If-then-else" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_if_else.lpq_internal << " = " << percent_frac(stats_if_else.lpq_internal, total_if_else) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_if_else.lpq_external << " = " << percent_frac(stats_if_else.lpq_external, total_if_else) << percent << endl;

    uintwide_t total_substitute = stats_substitute.lpq_internal + stats_substitute.lpq_external;
    o << indent << indent << "Substitution" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_substitute.lpq_internal << " = " << percent_frac(stats_substitute.lpq_internal, total_substitute) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_substitute.lpq_external << " = " << percent_frac(stats_substitute.lpq_external, total_substitute) << percent << endl;

    uintwide_t total_compare = stats_equality.lpq_internal + stats_equality.lpq_external;
    o << indent << indent << "Comparison" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_equality.lpq_internal << " = " << percent_frac(stats_equality.lpq_internal, total_compare) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_equality.lpq_external << " = " << percent_frac(stats_equality.lpq_external, total_compare) << percent << endl;

    uintwide_t total_intercut = stats_intercut.lpq_internal + stats_intercut.lpq_external;
    o << indent << indent << "Intercut" << endl;
    o << indent << indent << indent << "Internal" << indent
      << stats_intercut.lpq_internal << " = " << percent_frac(stats_intercut.lpq_internal, total_intercut) << percent << endl;
    o << indent << indent << indent << "External" << indent
      << stats_intercut.lpq_external << " = " << percent_frac(stats_intercut.lpq_external, total_intercut) << percent << endl;

    o << endl;
#endif
  }

  void adiar_statsreset()
  {
    stats_count = {};
    stats_equality = {};
    stats_if_else = {};
    stats_intercut = {};
    stats_priority_queue = {};
    stats_product_construction = {};
    stats_quantify = {};
    stats_reduce = {};
    stats_substitute = {};
  }
}
