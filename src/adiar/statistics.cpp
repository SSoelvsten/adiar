#include "statistics.h"

#include <iomanip>

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
  stats_t adiar_stats()
  {
#ifndef ADIAR_STATS
    std::cerr << "Statistics not gathered. Please compile with 'ADIAR_STATS' or 'ADIAR_STATS_EXTRA'" << std::endl;
#endif

    return {
      stats_levelized_priority_queue,
      stats_count,
      stats_equality,
      stats_if_else,
      stats_intercut,
      stats_product_construction,
      stats_quantify,
      stats_reduce,
      stats_substitute,
    };
  }

  void adiar_statsreset()
  {
    stats_count = {};
    stats_equality = {};
    stats_if_else = {};
    stats_intercut = {};
    stats_levelized_priority_queue = {};
    stats_product_construction = {};
    stats_quantify = {};
    stats_reduce = {};
    stats_substitute = {};
  }

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions for pretty printing (UNIX)
  int indent_level = 0;

  constexpr int FLOAT_PRECISION = 2;

  const std::string bold_on  = "\e[1m";
  const std::string bold_off = "\e[0m";
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
  ///
  /// TODO: move into 'adiar/internal/cnl.h'
  //////////////////////////////////////////////////////////////////////////////
  inline std::ostream& operator<< (std::ostream& os, const uintwide_t &s)
  {
    return os << to_string(s);
  }

#ifdef ADIAR_STATS_EXTRA
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
#else
  void __printstat_lpq(std::ostream &, const stats_t::levelized_priority_queue_t&)
  { }
#endif

  void __printstat_alg_base(std::ostream &o, const stats_t::__alg_base& stats)
  {
    o << indent << bold_on << "levelized priority queue" << bold_off << endl;

    const uintwide_t total_lpqs = stats.lpq.unbucketed + stats.lpq.internal + stats.lpq.external;
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
    uintwide_t total_runs = stats_count.lpq.internal + stats_count.lpq.external;
    o << indent << bold_on << label << "Count" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_count);
    indent_level--;
  }

  void __printstat_comparison_check(std::ostream &o)
  {
    uintwide_t total_runs = stats_equality.lpq.internal + stats_equality.lpq.external;
    o << indent << bold_on << label << "Comparison Check" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_equality);
    indent_level--;
  }

  void __printstat_ite(std::ostream &o)
  {
    uintwide_t total_runs = stats_if_else.lpq.internal + stats_if_else.lpq.external;
    o << indent << bold_on << label << "If-Then-Else" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_if_else);
    indent_level--;
  }

  void __printstat_intercut(std::ostream &o)
  {
    uintwide_t total_runs = stats_intercut.lpq.internal + stats_intercut.lpq.external;
    o << indent << bold_on << label << "Intercut" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_intercut);
    indent_level--;
  }

  void __printstat_isomorphism(std::ostream &o)
  {
    const uintwide_t total_runs = stats_equality.exit_on_same_file
                                + stats_equality.exit_on_nodecount
                                + stats_equality.exit_on_varcount
                                + stats_equality.exit_on_sinkcount
                                + stats_equality.exit_on_levels_mismatch
                                + stats_equality.slow_check.runs
                                + stats_equality.fast_check.runs;

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
    o << indent << label << "same file" << stats_equality.exit_on_same_file << endl;
    o << indent << label << "node count mismatch" << stats_equality.exit_on_nodecount << endl;
    o << indent << label << "var count mismatch" << stats_equality.exit_on_varcount << endl;
    o << indent << label << "sink count mismatch" << stats_equality.exit_on_sinkcount << endl;
    indent_level--;

    o << indent << endl;

    o << indent << "O(L/B) termination cases" << endl;
    indent_level++;
    o << indent << label << "level info mismatch" << stats_equality.exit_on_levels_mismatch << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(sort(N)) algorithm" << stats_equality.slow_check.runs << endl;
    indent_level++;
    o << indent << label << "local violation (root)" << stats_equality.slow_check.exit_on_root << endl;
    o << indent << label << "local violation (other)" << stats_equality.slow_check.exit_on_children << endl;
    o << indent << label << "too many requests" << stats_equality.slow_check.exit_on_processed_on_level << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(N/B) algorithm" << stats_equality.fast_check.runs << endl;
    indent_level++;
    o << indent << label << "node mismatch" << stats_equality.fast_check.exit_on_mismatch << endl;

    indent_level -= 2;

    o << indent << endl;

    o << indent << bold_on << "levelized priority queue" << bold_off << endl;
    indent_level++;
    o << indent << "see 'Comparison Check'" << endl;

    indent_level -= 2;
  }
  void __printstat_product_construction(std::ostream &o)
  {
    uintwide_t total_runs = stats_product_construction.lpq.internal + stats_product_construction.lpq.external;
    o << indent << bold_on << label << "Product Construction" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_product_construction);
    indent_level--;
  }

  void __printstat_reduce(std::ostream &o)
  {
    uintwide_t total_runs = stats_reduce.lpq.internal + stats_reduce.lpq.external;
    o << indent << bold_on << label << "Reduce" << bold_off << total_runs << endl;

    indent_level++;

    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }


    uintwide_t total_arcs = stats_reduce.sum_node_arcs + stats_reduce.sum_sink_arcs;
    o << indent << bold_on << label << "inputs size" << bold_off << total_arcs << " arcs = " << total_arcs / 2 << " nodes" << endl;

    indent_level++;
    o << indent << label << "node arcs:"
      << stats_reduce.sum_node_arcs << " = " << percent_frac(stats_reduce.sum_node_arcs, total_arcs) << percent << endl;

    o << indent << label << "sink arcs:"
      << stats_reduce.sum_sink_arcs << " = " << percent_frac(stats_reduce.sum_sink_arcs, total_arcs) << percent << endl;
    indent_level--;

#ifdef ADIAR_STATS_EXTRA
    o << indent << endl;
    uintwide_t total_removed = stats_reduce.removed_by_rule_1 + stats_reduce.removed_by_rule_2;
    o << indent << bold_on << label << "nodes removed" << bold_off;
    if (total_removed > 0u) {
      o << total_removed << " = " << percent_frac(total_removed, total_arcs) << percent << endl;
      indent_level++;
      o << indent << label << "rule 1:"
        << stats_reduce.removed_by_rule_1 << " = " << percent_frac(stats_reduce.removed_by_rule_1, total_removed) << percent << endl;

      o << indent << label <<  "rule 2:"
        << stats_reduce.removed_by_rule_2 << " = " << percent_frac(stats_reduce.removed_by_rule_2, total_removed) << percent << endl;
      indent_level--;
    } else {
      o << "none" << endl;
    }

#endif
    o << indent << endl;
    __printstat_alg_base(o, stats_reduce);

    indent_level--;
  }

  void __printstat_quantify(std::ostream &o)
  {
    uintwide_t total_runs = stats_quantify.lpq.internal + stats_quantify.lpq.external;
    o << indent << bold_on << label << "Quantification" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_quantify);
    indent_level--;
  }

  void __printstat_substitute(std::ostream &o)
  {
    uintwide_t total_runs = stats_substitute.lpq.internal + stats_substitute.lpq.external;
    o << indent << bold_on << label << "Substitution" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_substitute);
    indent_level--;
  }

  void adiar_printstat(std::ostream &o)
  {
    o << bold_on << "Adiar statistics" << bold_off << endl;
    o << endl;
#ifndef ADIAR_STATS
    o << indent << "Not gathered; please compile with 'ADIAR_STATS' and/or 'ADIAR_STATS_EXTRA'." << endl;
#else
    o << std::fixed << std::setprecision(FLOAT_PRECISION);

    __printstat_lpq(o, stats_levelized_priority_queue);
    o << endl;

    __printstat_count(o);
    o << endl;

    __printstat_comparison_check(o);
    o << endl;

    __printstat_ite(o);
    o << endl;

    __printstat_intercut(o);
    o << endl;

    __printstat_isomorphism(o);
    o << endl;

    __printstat_product_construction(o);
    o << endl;

    __printstat_reduce(o);
    o << endl;

    __printstat_quantify(o);
    o << endl;

    __printstat_substitute(o);
#endif
  }
}
