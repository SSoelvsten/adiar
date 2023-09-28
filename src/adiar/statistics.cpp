#include "statistics.h"

#include <iomanip>

#include <adiar/bdd/if_then_else.h>
#include <adiar/internal/algorithms/count.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/algorithms/substitution.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar
{
  // TODO: Move ownership of statistics object into `statistics.cpp`. This way,
  //       statistics do not need to be recompiled every time an algorithm is
  //       changed.

  statistics statistics_get()
  {
#ifndef ADIAR_STATS
    std::cerr << "Statistics not gathered. Please compile with 'ADIAR_STATS'" << std::endl;
#endif

    return {
      // i/o
      internal::stats_arc_file,
      internal::stats_node_file,

      // data structures
      internal::stats_levelized_priority_queue,

      // top-down sweeps
      internal::stats_count,
      internal::stats_equality,
      internal::stats_intercut,
      internal::stats_prod2,
      stats_prod3,
      internal::stats_quantify,
      internal::stats_substitute,

      // bottom-up sweeps
      internal::stats_reduce,

      // other algorithms
      internal::nested_sweeping::stats
    };
  }

  void statistics_reset()
  {
    // i/o
    internal::stats_arc_file                 = {};
    internal::stats_node_file                = {};

    // data structures
    internal::stats_levelized_priority_queue = {};

    // top-down sweeps
    internal::stats_count                    = {};
    internal::stats_equality                 = {};
    internal::stats_intercut                 = {};
    internal::stats_prod2                    = {};
    stats_prod3                              = {};
    internal::stats_quantify                 = {};
    internal::stats_substitute               = {};

    // bottom-up sweeps
    internal::stats_reduce                   = {};

    // other algorithms
    internal::nested_sweeping::stats         = {};
  }

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions for pretty printing (UNIX)
  int indent_level = 0;

  constexpr int FLOAT_PRECISION = 2;

  const std::string bold_on  = "\033[1m";
  const std::string bold_off = "\033[0m";
  const std::string percent  = "%";

  inline std::ostream& indent(std::ostream &os)
  { return os << "| " << std::left << std::setw(2*indent_level) << ""; }

  inline std::ostream& label(std::ostream &os)
  {
    constexpr int cell1_indent_level = 38;
    return os << std::left << std::setw(cell1_indent_level - 2*indent_level);
  }

  inline std::ostream& endl(std::ostream &os)
  { return os << std::endl; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert a wide integer to a string and push it to an output stream.
  ///
  /// \details This should be moved to `adiar::internal` in
  ///          `<adiar/internal/cnl.h>` but doing so creates an ambiguity for
  ///          the `ptr_uint64` class. Hence, we have to keep this an function
  ///          of only this file until we have added `to_string()` functions to
  ///          all basic data types.
  //////////////////////////////////////////////////////////////////////////////
  inline std::ostream& operator<< (std::ostream& os, const uintwide &s)
  {
    return os << internal::to_string(s);
  }

  void __printstat_arc_file(std::ostream &o)
  {
    o << indent << bold_on << "Arc Files" << bold_off << endl;

    indent_level++;

    uintwide total_pushes = internal::stats_arc_file.push_internal
      + internal::stats_arc_file.push_in_order
      + internal::stats_arc_file.push_out_of_order;

    if (total_pushes == 0u) {
      o << indent << "No writes" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "unsafe_push(arc ...)" << bold_off << total_pushes << endl;

    indent_level++;
    o << indent << label << "internal" << internal::stats_arc_file.push_internal
      << " = " << internal::percent_frac(internal::stats_arc_file.push_internal, total_pushes) << percent << endl;
    o << indent << label << "terminals (in-order)" << internal::stats_arc_file.push_in_order
      << " = " << internal::percent_frac(internal::stats_arc_file.push_in_order, total_pushes) << percent << endl;
    o << indent << label << "terminals (out-of-order)" << internal::stats_arc_file.push_out_of_order
      << " = " << internal::percent_frac(internal::stats_arc_file.push_out_of_order, total_pushes) << percent << endl;
    indent_level--;

    o << indent << bold_on << label << "push(level_info ...)" << bold_off << internal::stats_arc_file.push_level << endl;

    o << indent << bold_on << label << "out-of-order sortings" << bold_off << internal::stats_arc_file.sort_out_of_order << endl;

    indent_level--;
  }

  void __printstat_node_file(std::ostream &o)
  {
    o << indent << bold_on << "Node Files" << bold_off << endl;

    indent_level++;

    if (internal::stats_node_file.push_node == 0u) {
      o << indent << "No writes" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "unsafe_push(...)" << bold_off << endl;

    indent_level++;
    o << indent << label << "node" << internal::stats_node_file.push_node << endl;
    o << indent << label << "level_info" << internal::stats_node_file.push_level << endl;
    indent_level--;

    indent_level--;
  }

  void __printstat_lpq(std::ostream &o, const statistics::levelized_priority_queue_t& stats)
  {
    if (indent_level == 0) {
      o << indent << bold_on << "Levelized Priority Queue" << bold_off << endl;
    }

    indent_level++;

    if (stats.sum_destructors == 0) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    uintwide total_pushes = stats.push_bucket + stats.push_overflow;
    o << indent << bold_on << label << "push(...)" << bold_off << total_pushes << endl;

    indent_level++;
    o << indent << label << "hit bucket" << stats.push_bucket
      << " = " << internal::percent_frac(stats.push_bucket, total_pushes) << percent
      << endl;
    o << indent << label << "hit overflow" << stats.push_overflow
      << " = " << internal::percent_frac(stats.push_overflow, total_pushes) << percent << endl;
    indent_level--;

    o << indent << endl;

    o << indent << bold_on << "prediction precision ratio" << bold_off << endl;

    indent_level++;
    o << indent << label << "unweighted"
      << stats.sum_max_size_ratio << " / " << stats.sum_destructors
      << " = " << 100.0 * stats.sum_max_size_ratio / stats.sum_destructors << percent
      << endl;

    o << indent << label << "weighted"
      << stats.sum_actual_max_size << " / " << stats.sum_predicted_max_size
      << " = " << internal::percent_frac(stats.sum_actual_max_size, stats.sum_predicted_max_size) << percent
      << endl;
    indent_level -= 2;
  }


  void __printstat_alg_base(std::ostream &o, const statistics::__alg_base& stats)
  {
    o << indent << bold_on << "levelized priority queue" << bold_off << endl;

    const uintwide total_lpqs = stats.lpq.total();
    const uintwide total_internal_lpqs = stats.lpq.unbucketed + stats.lpq.internal;

    indent_level++;

    o << indent << label << "external memory"
      << stats.lpq.external << " = " << internal::percent_frac(stats.lpq.external, total_lpqs) << percent << endl;

    o << indent << label << "internal memory"
      << total_internal_lpqs << " = " << internal::percent_frac(total_internal_lpqs, total_lpqs) << percent << endl;

    o << indent << label << "  unbucketed"
      << stats.lpq.unbucketed << " = " << internal::percent_frac(stats.lpq.unbucketed, total_internal_lpqs) << percent << endl;

    indent_level--;

    o << indent << endl;

    __printstat_lpq(o, stats.lpq);
  }


  void __printstat_count(std::ostream &o)
  {
    uintwide total_runs = internal::stats_count.lpq.total();
    o << indent << bold_on << label << "Count" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_count);
    indent_level--;
  }


  void __printstat_comparison_check(std::ostream &o)
  {
    uintwide total_runs = internal::stats_equality.lpq.total();
    o << indent << bold_on << label << "Comparison Check" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_equality);
    indent_level--;
  }


  void __printstat_intercut(std::ostream &o)
  {
    uintwide total_runs = internal::stats_intercut.lpq.total();
    o << indent << bold_on << label << "Intercut" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_intercut);
    indent_level--;
  }


  void __printstat_isomorphism(std::ostream &o)
  {
    const uintwide total_runs = internal::stats_equality.exit_on_same_file
                                + internal::stats_equality.exit_on_nodecount
                                + internal::stats_equality.exit_on_varcount
                                + internal::stats_equality.exit_on_terminalcount
                                + internal::stats_equality.exit_on_levels_mismatch
                                + internal::stats_equality.slow_check.runs
                                + internal::stats_equality.fast_check.runs;

    o << indent << bold_on << label << "Isomorphism Check" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    o << indent  << bold_on << "execution trace" << bold_off << endl;

    indent_level++;
    o << indent << "O(1) termination cases" << endl;
    indent_level++;
    o << indent << label << "same file"
      << internal::stats_equality.exit_on_same_file << endl;
    o << indent << label << "node count mismatch"
      << internal::stats_equality.exit_on_nodecount << endl;
    o << indent << label << "var count mismatch"
      << internal::stats_equality.exit_on_varcount << endl;
    o << indent << label << "terminal count mismatch"
      << internal::stats_equality.exit_on_terminalcount << endl;
    indent_level--;

    o << indent << endl;

    o << indent << "O(L/B) termination cases" << endl;
    indent_level++;
    o << indent << label << "level info mismatch"
      << internal::stats_equality.exit_on_levels_mismatch << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(sort(N)) algorithm"
      << internal::stats_equality.slow_check.runs << endl;
    indent_level++;
    o << indent << label << "local violation (root)"
      << internal::stats_equality.slow_check.exit_on_root << endl;
    o << indent << label << "local violation (other)"
      << internal::stats_equality.slow_check.exit_on_children << endl;
    o << indent << label << "too many requests"
      << internal::stats_equality.slow_check.exit_on_processed_on_level << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(N/B) algorithm"
      << internal::stats_equality.fast_check.runs << endl;
    indent_level++;
    o << indent << label << "node mismatch"
      << internal::stats_equality.fast_check.exit_on_mismatch << endl;

    indent_level -= 2;

    o << indent << endl;

    o << indent << bold_on << "levelized priority queue" << bold_off << endl;
    indent_level++;
    o << indent << "see 'Comparison Check'" << endl;

    indent_level -= 2;
  }


  void __printstat_prod2(std::ostream &o)
  {
    uintwide total_runs = internal::stats_prod2.trivial_file
                          + internal::stats_prod2.trivial_terminal
                          + internal::stats_prod2.ra.runs
                          + internal::stats_prod2.pq.runs;

    o << indent << bold_on << label << "Product Construction (2-ary)" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "case [same file]" << bold_off
      << internal::stats_prod2.trivial_file
      << " = " << internal::percent_frac(internal::stats_prod2.trivial_file, total_runs) << percent
      << endl;

    o << indent << endl;

    o << indent << bold_on << label << "case [terminal]" << bold_off
      << internal::stats_prod2.trivial_terminal
      << " = " << internal::percent_frac(internal::stats_prod2.trivial_terminal, total_runs) << percent
      << endl;

    o << indent << endl;

    o << indent << bold_on << label << "case [random access]" << bold_off
      << internal::stats_prod2.ra.runs
      << " = " << internal::percent_frac(internal::stats_prod2.ra.runs, total_runs) << percent
      << endl;
    if (internal::stats_prod2.ra.runs > 0u) {
      indent_level++;
      o << indent << label << "used narrowest:"
        << internal::stats_prod2.ra.used_narrowest
        << " = " << internal::percent_frac(internal::stats_prod2.ra.used_narrowest, internal::stats_prod2.ra.runs) << percent
        << endl;

      o << indent << endl;

      o << indent << bold_on << label << "width:" << bold_off << endl;
      indent_level++;

      o << indent << label << "minimum:"
        << internal::stats_prod2.ra.min_width
        << endl;

      o << indent << label << "maximum:"
        << internal::stats_prod2.ra.max_width
        << endl;

      o << indent << label << "accumulated:"
        << internal::stats_prod2.ra.acc_width
        << " (avg = " << internal::frac(internal::stats_prod2.ra.acc_width, internal::stats_prod2.ra.runs) << ")"
        << endl;

      indent_level -= 2;
    }

    o << indent << endl;

    o << indent << bold_on << label << "case [priority queue]" << bold_off
      << internal::stats_prod2.pq.runs
      << " = " << internal::percent_frac(internal::stats_prod2.pq.runs, total_runs) << percent
      << endl;
    if (internal::stats_prod2.pq.runs > 0u) {
      indent_level++;
      o << indent << label << "pq2 elements:"
        << internal::stats_prod2.pq.pq_2_elems
        << endl;

      indent_level--;
    }


    o << indent << endl;
    __printstat_alg_base(o, internal::stats_prod2);
    indent_level--;
  }


  void __printstat_prod3(std::ostream &o)
  {
    uintwide total_runs = stats_prod3.lpq.total();
    o << indent << bold_on << label << "Product Construction (3-ary)" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_prod3);
    indent_level--;
  }


  void __printstat_reduce(std::ostream &o)
  {
    uintwide total_runs = internal::stats_reduce.lpq.total();
    o << indent << bold_on << label << "Reduce" << bold_off << total_runs << endl;

    indent_level++;

    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    uintwide total_arcs = internal::stats_reduce.sum_node_arcs + internal::stats_reduce.sum_terminal_arcs;
    o << indent << bold_on << label << "inputs size" << bold_off
      << total_arcs << " arcs = " << total_arcs / 2 << " nodes" << endl;

    indent_level++;
    o << indent << label << "node arcs:"
      << internal::stats_reduce.sum_node_arcs
      << " = " << internal::percent_frac(internal::stats_reduce.sum_node_arcs, total_arcs) << percent
      << endl;

    o << indent << label << "terminal arcs:"
      << internal::stats_reduce.sum_terminal_arcs
      << " = " << internal::percent_frac(internal::stats_reduce.sum_terminal_arcs, total_arcs) << percent
      << endl;
    indent_level--;

    o << indent << endl;
    uintwide total_removed = internal::stats_reduce.removed_by_rule_1 + internal::stats_reduce.removed_by_rule_2;
    o << indent << bold_on << label << "nodes removed" << bold_off;
    if (total_removed > 0u) {
      o << total_removed << " = " << internal::percent_frac(total_removed, total_arcs) << percent << endl;
      indent_level++;
      o << indent << label << "rule 1:"
        << internal::stats_reduce.removed_by_rule_1
        << " = " << internal::percent_frac(internal::stats_reduce.removed_by_rule_1, total_removed) << percent
        << endl;

      o << indent << label <<  "rule 2:"
        << internal::stats_reduce.removed_by_rule_2
        << " = " << internal::percent_frac(internal::stats_reduce.removed_by_rule_2, total_removed) << percent
        << endl;
      indent_level--;
    } else {
      o << "none" << endl;
    }

    o << indent << endl;
    __printstat_alg_base(o, internal::stats_reduce);

    indent_level--;
  }


  void __printstat_quantify(std::ostream &o)
  {
    uintwide total_runs = internal::stats_quantify.lpq.total();
    o << indent << bold_on << label << "Quantification" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_quantify);
    indent_level--;
  }


  void __printstat_substitute(std::ostream &o)
  {
    uintwide total_runs = internal::stats_substitute.lpq.total();
    o << indent << bold_on << label << "Substitution" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_substitute);
    indent_level--;
  }


  void statistics_print(std::ostream &o)
  {
    o << bold_on << "Adiar statistics" << bold_off << endl;
    o << endl;
#ifndef ADIAR_STATS
    o << indent << "Not gathered; please compile with 'ADIAR_STATS'." << endl;
#else
    o << std::fixed << std::setprecision(FLOAT_PRECISION);

    o << bold_on << "--== I/O ==--" << bold_off << endl << endl;

    __printstat_arc_file(o);
    o << endl;

    __printstat_node_file(o);
    o << endl;

    o << bold_on << "--== Data Structures ==--" << bold_off << endl << endl;

    __printstat_lpq(o, internal::stats_levelized_priority_queue);
    o << endl;

    o << bold_on << "--== Top-Down Sweep Algorithms ==--" << bold_off << endl << endl;

    __printstat_count(o);
    o << endl;

    __printstat_comparison_check(o);
    o << endl;

    __printstat_intercut(o);
    o << endl;

    __printstat_isomorphism(o);
    o << endl;

    __printstat_prod2(o);
    o << endl;

    __printstat_prod3(o);
    o << endl;

    __printstat_quantify(o);
    o << endl;

    __printstat_substitute(o);
    o << endl;

    o << bold_on << "--== Bottom-Up Sweep Algorithms ==--" << bold_off << endl << endl;

    __printstat_reduce(o);
#endif
  }
}
