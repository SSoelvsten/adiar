#include "statistics.h"

#include <iomanip>

#include <adiar/bdd/if_then_else.h>
#include <adiar/internal/algorithms/count.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/algorithms/substitution.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>

namespace adiar
{
  stats_t adiar_stats()
  {
#ifndef ADIAR_STATS
    std::cerr << "Statistics not gathered. Please compile with 'ADIAR_STATS'" << std::endl;
#endif

    return {
      internal::stats_levelized_priority_queue,
      internal::stats_count,
      internal::stats_equality,
      internal::stats_intercut,
      internal::stats_prod2,
      stats_prod3,
      internal::stats_quantify,
      internal::stats_reduce,
      internal::stats_substitute
    };
  }

  void adiar_statsreset()
  {
    internal::stats_count      = {};
    internal::stats_equality   = {};
    internal::stats_intercut   = {};
    internal::stats_levelized_priority_queue = {};
    internal::stats_prod2      = {};
    stats_prod3                = {};
    internal::stats_quantify   = {};
    internal::stats_reduce     = {};
    internal::stats_substitute = {};
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
  /// \brief Convert a wide integer to a string and push to an output stream.
  //////////////////////////////////////////////////////////////////////////////
  // TODO: move into 'adiar/internal/cnl.h'
  inline std::ostream& operator<< (std::ostream& os, const uintwide_t &s)
  {
    return os << to_string(s);
  }

  void __printstat_lpq(std::ostream &o, const stats_t::levelized_priority_queue_t& stats)
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

    uintwide_t total_pushes = stats.push_bucket + stats.push_overflow;
    o << indent << bold_on << label << "push(...)" << bold_off << total_pushes << endl;

    indent_level++;
    o << indent << label << "hit bucket" << stats.push_bucket
      << " = " << percent_frac(stats.push_bucket, total_pushes) << percent
      << endl;
    o << indent << label << "hit overflow" << stats.push_overflow
      << " = " << percent_frac(stats.push_overflow, total_pushes) << percent << endl;
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
      << " = " << percent_frac(stats.sum_actual_max_size, stats.sum_predicted_max_size) << percent
      << endl;
    indent_level -= 2;
  }

  void __printstat_alg_base(std::ostream &o, const stats_t::__alg_base& stats)
  {
    o << indent << bold_on << "levelized priority queue" << bold_off << endl;

    const uintwide_t total_lpqs = stats.lpq.total();
    const uintwide_t total_internal_lpqs = stats.lpq.unbucketed + stats.lpq.internal;

    indent_level++;

    o << indent << label << "external memory"
      << stats.lpq.external << " = " << percent_frac(stats.lpq.external, total_lpqs) << percent << endl;

    o << indent << label << "internal memory"
      << total_internal_lpqs << " = " << percent_frac(total_internal_lpqs, total_lpqs) << percent << endl;

    o << indent << label << "  unbucketed"
      << stats.lpq.unbucketed << " = " << percent_frac(stats.lpq.unbucketed, total_internal_lpqs) << percent << endl;

    indent_level--;

    o << indent << endl;

    __printstat_lpq(o, stats.lpq);
  }


  void __printstat_count(std::ostream &o)
  {
    uintwide_t total_runs = internal::stats_count.lpq.total();
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
    uintwide_t total_runs = internal::stats_equality.lpq.total();
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
    uintwide_t total_runs = internal::stats_intercut.lpq.total();
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
    const uintwide_t total_runs = internal::stats_equality.exit_on_same_file
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
    uintwide_t total_runs = internal::stats_prod2.lpq.total();
    o << indent << bold_on << label << "Product Construction (2-ary)" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_prod2);
    indent_level--;
  }

  void __printstat_prod3(std::ostream &o)
  {
    uintwide_t total_runs = stats_prod3.lpq.total();
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
    uintwide_t total_runs = internal::stats_reduce.lpq.total();
    o << indent << bold_on << label << "Reduce" << bold_off << total_runs << endl;

    indent_level++;

    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    uintwide_t total_arcs = internal::stats_reduce.sum_node_arcs + internal::stats_reduce.sum_terminal_arcs;
    o << indent << bold_on << label << "inputs size" << bold_off
      << total_arcs << " arcs = " << total_arcs / 2 << " nodes" << endl;

    indent_level++;
    o << indent << label << "node arcs:"
      << internal::stats_reduce.sum_node_arcs
      << " = " << percent_frac(internal::stats_reduce.sum_node_arcs, total_arcs) << percent
      << endl;

    o << indent << label << "terminal arcs:"
      << internal::stats_reduce.sum_terminal_arcs
      << " = " << percent_frac(internal::stats_reduce.sum_terminal_arcs, total_arcs) << percent
      << endl;
    indent_level--;

    o << indent << endl;
    uintwide_t total_removed = internal::stats_reduce.removed_by_rule_1 + internal::stats_reduce.removed_by_rule_2;
    o << indent << bold_on << label << "nodes removed" << bold_off;
    if (total_removed > 0u) {
      o << total_removed << " = " << percent_frac(total_removed, total_arcs) << percent << endl;
      indent_level++;
      o << indent << label << "rule 1:"
        << internal::stats_reduce.removed_by_rule_1
        << " = " << percent_frac(internal::stats_reduce.removed_by_rule_1, total_removed) << percent
        << endl;

      o << indent << label <<  "rule 2:"
        << internal::stats_reduce.removed_by_rule_2
        << " = " << percent_frac(internal::stats_reduce.removed_by_rule_2, total_removed) << percent
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
    uintwide_t total_runs = internal::stats_quantify.lpq.total();
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
    uintwide_t total_runs = internal::stats_substitute.lpq.total();
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

  void adiar_printstat(std::ostream &o)
  {
    o << bold_on << "Adiar statistics" << bold_off << endl;
    o << endl;
#ifndef ADIAR_STATS
    o << indent << "Not gathered; please compile with 'ADIAR_STATS'." << endl;
#else
    o << std::fixed << std::setprecision(FLOAT_PRECISION);

    __printstat_lpq(o, internal::stats_levelized_priority_queue);
    o << endl;

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

    __printstat_reduce(o);
    o << endl;

    __printstat_quantify(o);
    o << endl;

    __printstat_substitute(o);
#endif
  }
}
