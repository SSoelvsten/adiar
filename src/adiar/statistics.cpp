#include "statistics.h"

#include <adiar/internal/pred.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/reduce.h>

namespace adiar
{
  // Define the available function
  stats_t adiar_stats()
  {
    return {
      stats_equality,
      stats_priority_queue,
      stats_reduce,
    };
  }

  // Helper functions for pretty printing (UNIX)
  std::ostream& bold_on(std::ostream& os)  { return os << "\e[1m"; }
  std::ostream& bold_off(std::ostream& os) { return os << "\e[0m"; }
  std::ostream& percent(std::ostream& os)  { return os << "%"; }
  std::ostream& indent(std::ostream& os)   { return os << "  "; }
  std::ostream& endl(std::ostream& os)     { return os << std::endl; }

  double compute_percent(size_t s, size_t of) { return (static_cast<double>(s) / static_cast<double>(of)) * 100; }

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
    size_t total_pushes = stats_priority_queue.push_bucket + stats_priority_queue.push_overflow;

    o << indent << bold_on << "Levelized Priority Queue" << bold_off << endl;
    o << indent << indent << "pushes to bucket        " << indent << stats_priority_queue.push_bucket
      << " = " << compute_percent(stats_priority_queue.push_bucket, total_pushes) << percent << endl;
    o << indent << indent << "pushes to overflow      " << indent << stats_priority_queue.push_overflow
      << " = " << compute_percent(stats_priority_queue.push_overflow, total_pushes) << percent << endl;
    o << endl;
#endif

    size_t total_arcs = stats_reduce.sum_node_arcs + stats_reduce.sum_sink_arcs;
    o << indent << bold_on << "Reduce" << bold_off << endl;

    o << indent << indent << "input size              " << indent << total_arcs << " arcs = " << total_arcs / 2 << " nodes" << endl;

    o << indent << indent << indent << "node arcs:            " << indent
      << stats_reduce.sum_node_arcs << " = " << compute_percent(stats_reduce.sum_node_arcs, total_arcs) << percent << endl;

    o << indent << indent << indent << "sink arcs:            " << indent
      << stats_reduce.sum_sink_arcs << " = " << compute_percent(stats_reduce.sum_sink_arcs, total_arcs) << percent << endl;
#ifdef ADIAR_STATS_EXTRA
    size_t total_removed = stats_reduce.removed_by_rule_1 + stats_reduce.removed_by_rule_2;
    o << indent << indent << "nodes removed           " << indent
      << total_removed << " = " << compute_percent(total_removed, total_arcs) << percent << endl;

    if (total_removed > 0) {
      o << indent << indent << indent << "rule 1:               " << indent
        << stats_reduce.removed_by_rule_1 << " = " << compute_percent(stats_reduce.removed_by_rule_1, total_removed) << percent << endl;

      o << indent << indent << indent << "rule 2:               " << indent
        << stats_reduce.removed_by_rule_2 << " = " << compute_percent(stats_reduce.removed_by_rule_2, total_removed) << percent << endl;
    }
#endif
    o << endl;
#endif
  }
}
