////////////////////////////////////////////////////////////////////////////////
// Copy paste from src/pigeon_hole.cpp of
//
//                https://github.com/SSoelvsten/obdd-benchmark
//
// Comments have been removed.

#include <stdint.h>
#include <functional>
#include <utility>

#include <assert.h>

#include <vector>
#include <set>
#include <queue>

typedef std::pair<uint64_t, bool> literal_t;
typedef std::vector<literal_t> clause_t;
typedef std::vector<clause_t> cnf_t;

class sat_solver
{
private:
  cnf_t clauses;
  bool should_sort = false;

public:
  void add_clause(clause_t clause)
  {
    clauses.push_back(clause);
    should_sort = true;
  }

  size_t cnf_size()
  {
    return clauses.size();
  }

  bool is_unsatisfiable(const std::function<void(clause_t&)> &on_and_clause,
                        const std::function<void(uint64_t)> &on_quantify_variable,
                        const std::function<bool()> &on_is_false)
  {
    if (should_sort)
    {
      // TODO: Based on the footnote in pigeonhole formulas, we may just want to
      // construct it in the same order they used?
      std::sort(clauses.begin(), clauses.end(),
                [](clause_t &a, clause_t &b) {
                  return a.back().first > b.back().first;
                });
    }

    std::set<uint64_t> unique_labels;
    std::priority_queue<uint64_t> priority_labels;

    for (clause_t &clause : clauses)
    {
      while (!priority_labels.empty() && clause.back().first < priority_labels.top())
      {
        on_quantify_variable(priority_labels.top());

        unique_labels.erase(priority_labels.top());
        priority_labels.pop();
      }

      for (literal_t &x : clause)
      {
        if (unique_labels.insert(x.first).second)
        {
          priority_labels.push(x.first);
        }
      }

      on_and_clause(clause);

      if (on_is_false())
      {
        return true;
      }
    }

    while (!priority_labels.empty())
    {
      on_quantify_variable(priority_labels.top());

      unique_labels.erase(priority_labels.top());
      priority_labels.pop();
    }

    return on_is_false();
  }

  bool is_satisfiable(const std::function<void(clause_t&)> &on_and_clause,
                      const std::function<void(uint64_t)> &on_quantify_variable,
                      const std::function<bool()> &on_is_false)
  {
    return !is_unsatisfiable(on_and_clause, on_quantify_variable, on_is_false);
  }
};

uint64_t label_of_Pij(uint64_t i, uint64_t j, uint64_t N)
{
  assert(i <= N+1);
  assert(j <= N);
  return (N+1)*i + j;
}

void construct_PHP_cnf(sat_solver &solver, uint64_t N)
{
  // PC_n
  for (uint64_t i = 1; i <= N+1; i++)
  {
    clause_t clause;
    for (uint64_t j = 1; j <= N; j++)
    {
      clause.push_back(literal_t (label_of_Pij(i,j,N), false));
    }
    solver.add_clause(clause);
  }

  // NC_n
  for (uint64_t i = 1; i < N+1; i++)
  {
    for (uint64_t j = i+1; j <= N+1; j++)
    {
      for (uint64_t k = 1; k <= N; k++)
      {
        clause_t clause;
        clause.push_back(literal_t (label_of_Pij(i,k,N), true));
        clause.push_back(literal_t (label_of_Pij(j,k,N), true));
        solver.add_clause(clause);
      }
    }
  }
}

// Copy paste end
////////////////////////////////////////////////////////////////////////////////


#include <vector>
#include <chrono>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

// COOM Imports
#include <coom/coom.h>


/*******************************************************************************
 * We base our example for the Pigeonhole principle on the formula described in
 * the paper "Ordered Binary Decision Diagrams, Pigeonhole Formulas and Beyond"
 * by Olga Tveretina, Carsten Sinz and Hans Zantema.
 *
 * The question to be answered is as follows: Does there exist an isomorphism
 * between the sets {1, 2, ..., N+1} (pigeons) and {1, 2, ..., N}. We will prove
 * it to be false by showing the corresponding CNF formula of at-least-one and
 * at-most-one collapses to the false sink.
 *
 * We'd find it interesting to output the size of the largest OBDD, so we create
 * the following global variables. Also it would be of interest to see the best
 * and worst ratio between sink and node arcs in the unreduced OBDDs
 */
size_t number_of_exists = 0;
size_t number_of_applies = 0;
size_t largest_unreduced = 0;

std::vector<size_t> apply_unreduced_sizes;

float best_apply_sink_ratio = 0.0;
float acc_apply_sink_ratio = 0.0;
float worst_apply_sink_ratio = 1.0;

inline void stats_apply_unreduced(size_t node_arcs, size_t sink_arcs)
{
  number_of_applies++;

  size_t total_arcs = node_arcs + sink_arcs;
  largest_unreduced = std::max(largest_unreduced, total_arcs / 2);

  apply_unreduced_sizes.push_back(total_arcs / 2);

  float sink_ratio = float(sink_arcs) / float(total_arcs);
  best_apply_sink_ratio = std::max(best_apply_sink_ratio, sink_ratio);
  acc_apply_sink_ratio += sink_ratio;
  worst_apply_sink_ratio = std::min(worst_apply_sink_ratio, sink_ratio);
}

std::vector<size_t> exists_unreduced_sizes;

float best_exists_sink_ratio = 0.0;
float acc_exists_sink_ratio = 0.0;
float worst_exists_sink_ratio = 1.0;

inline void stats_exists_unreduced(size_t node_arcs, size_t sink_arcs)
{
  number_of_exists++;

  size_t total_arcs = node_arcs + sink_arcs;
  largest_unreduced = std::max(largest_unreduced, total_arcs / 2);

  exists_unreduced_sizes.push_back(total_arcs / 2);

  float sink_ratio = float(sink_arcs) / float(total_arcs);
  best_exists_sink_ratio = std::max(best_exists_sink_ratio, sink_ratio);
  acc_exists_sink_ratio += sink_ratio;
  worst_exists_sink_ratio = std::min(worst_exists_sink_ratio, sink_ratio);
}

size_t largest_reduced = 0;

float best_apply_reduction_ratio = 1.0;
float acc_apply_reduction_ratio = 0.0;
float worst_apply_reduction_ratio = 0.0;

inline void stats_apply_reduced(size_t unreduced_size, size_t reduced_size)
{
  largest_reduced = std::max(largest_reduced, reduced_size);

  float reduction_ratio = float(reduced_size) / float(unreduced_size);
  best_apply_reduction_ratio = std::min(best_apply_reduction_ratio, reduction_ratio);
  acc_apply_reduction_ratio += reduction_ratio;
  worst_apply_reduction_ratio = std::max(worst_apply_reduction_ratio, reduction_ratio);
}

float best_exists_reduction_ratio = 1.0;
float acc_exists_reduction_ratio = 0.0;
float worst_exists_reduction_ratio = 0.0;

inline void stats_exists_reduced(size_t unreduced_size, size_t reduced_size)
{
  largest_reduced = std::max(largest_reduced, reduced_size);

  float reduction_ratio = float(reduced_size) / float(unreduced_size);
  best_exists_reduction_ratio = std::min(best_exists_reduction_ratio, reduction_ratio);
  acc_exists_reduction_ratio += reduction_ratio;
  worst_exists_reduction_ratio = std::max(worst_exists_reduction_ratio, reduction_ratio);
}

/* A few chrono wrappers to improve readability of the code below */
inline auto get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline auto duration_of(std::chrono::high_resolution_clock::time_point &before,
                        std::chrono::high_resolution_clock::time_point &after) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
}

/* TODO: File size calculations should be available from COOM. */
inline auto MB_of_size(tpie::stream_size_type size) {
  return size * sizeof(coom::node) /* bytes */ / (1024 * 1024) /* in MB */;
}

/*******************************************************************************
 *                             Variable ordering
 *
 * Following the paper, we use the variable ordering for variables Pij such that
 * it is first sorted by i, then by j. See the function label_of_Pij above
 * copied from the other repository.
 */
int main(int argc, char** argv)
{
  // ===== Parse argument =====
  int N = -1;
  size_t M = 1;

  try {
    if (argc == 1) {
      tpie::log_info() << "Missing argument for N and M" << std::endl;
    } else {
      N = std::stoi(argv[1]);
      if (N < 0 || N > 32) {
        tpie::log_info() << "N should be in interval [0;32]: " << argv[1] << std::endl;
        N = -1;
      }
      if (argc == 3) {
        M = std::stoi(argv[2]);
        if (M <= 0) {
          tpie::log_info() << "M should at least be 1: " << argv[2] << std::endl;
        }
      }
    }
  } catch (std::invalid_argument const &ex) {
    tpie::log_info() << "Invalid number: " << argv[1] << std::endl;
  } catch (std::out_of_range const &ex) {
    tpie::log_info() << "Number out of range: " << argv[1] << std::endl;
  }

  if (N == -1 || M <= 0) {
    exit(1);
  }


  // ===== COOM =====
  // Initialize
  coom::coom_init(M);
  tpie::log_info() << "| Initialized COOM with " << M << " MB of memory"  << std::endl;


  // =========================================================================
  tpie::file_stream<coom::node_t> sat_acc;
  sat_acc.open();

  tpie::file_stream<coom::meta_t> sat_acc_meta;
  sat_acc_meta.open();

  coom::build_sink(true, sat_acc, sat_acc_meta);

  const auto sat_and_clause = [&](clause_t &clause) -> void
  {
    tpie::file_stream<coom::node_t> clause_nodes;
    clause_nodes.open();

    tpie::file_stream<coom::meta_t> clause_meta;
    clause_meta.open();

    coom::ptr_t next = coom::create_sink_ptr(false);

    for (auto it = clause.rbegin(); it != clause.rend(); it++)
    {
      literal_t v = *it;

      coom::node n = coom::create_node(v.first, 0,
                                       v.second ? coom::create_sink_ptr(true) : next,
                                       v.second ? next : coom::create_sink_ptr(true));

      next = n.uid;

      clause_nodes.write(n);
      clause_meta.write({ v.first });
    }

    tpie::file_stream<coom::arc_t> reduce_node_arcs;
    tpie::file_stream<coom::arc_t> reduce_sink_arcs;
    tpie::file_stream<coom::meta_t> reduce_meta;

    reduce_node_arcs.open();
    reduce_sink_arcs.open();
    reduce_meta.open();

    coom::apply(sat_acc, sat_acc_meta,
                clause_nodes, clause_meta,
                coom::and_op,
                reduce_node_arcs, reduce_sink_arcs, reduce_meta);

    sat_acc.close();
    sat_acc_meta.close();
    clause_nodes.close();
    clause_meta.close();

    stats_apply_unreduced(reduce_node_arcs.size(), reduce_sink_arcs.size());

    sat_acc.open();
    sat_acc_meta.open();

    coom::reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta, sat_acc, sat_acc_meta);

    stats_apply_reduced((reduce_node_arcs.size() + reduce_sink_arcs.size()) / 2, sat_acc.size());

    reduce_node_arcs.close();
    reduce_sink_arcs.close();
    reduce_meta.close();
  };

  const auto sat_quantify_variable = [&](uint64_t var) -> void
  {
    tpie::file_stream<coom::arc_t> reduce_node_arcs;
    tpie::file_stream<coom::arc_t> reduce_sink_arcs;
    tpie::file_stream<coom::meta_t> reduce_meta;

    reduce_node_arcs.open();
    reduce_sink_arcs.open();
    reduce_meta.open();

    coom::exists(var,
                 sat_acc, sat_acc_meta,
                 reduce_node_arcs, reduce_sink_arcs, reduce_meta);

    sat_acc.close();
    sat_acc_meta.close();

    stats_exists_unreduced(reduce_node_arcs.size(), reduce_sink_arcs.size());

    sat_acc.open();
    sat_acc_meta.open();

    coom::reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta, sat_acc, sat_acc_meta);

    stats_exists_reduced((reduce_node_arcs.size() + reduce_sink_arcs.size()) / 2, sat_acc.size());

    reduce_node_arcs.close();
    reduce_sink_arcs.close();
    reduce_meta.close();
  };

  const auto sat_is_false = [&]() -> bool
  {
    return coom::is_sink(sat_acc, coom::is_false);
  };

  // =========================================================================
  auto t1 = get_timestamp();

  sat_solver solver;
  construct_PHP_cnf(solver, N);

  auto t2 = get_timestamp();

  // =========================================================================
  auto t3 = get_timestamp();

  bool satisfiable = solver.is_satisfiable(sat_and_clause,
                                           sat_quantify_variable,
                                           sat_is_false);

  auto t4 = get_timestamp();

  // =========================================================================
  tpie::log_info() << "| Pigeonhole Principle (" << N << ") :"  << std::endl;
  tpie::log_info() << "|  | solution:     " << (satisfiable ? "SATISFIABLE" : "UNSATISFIABLE") << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | CNF: " << std::endl;
  tpie::log_info() << "|  |  | variables:        " << label_of_Pij(N+1, N, N) << std::endl;
  tpie::log_info() << "|  |  | clauses:          " << solver.cnf_size() << std::endl;
  tpie::log_info() << "|  |  | number of exists: " << number_of_exists << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  auto largest_unreduced_MB = (MB_of_size(largest_unreduced) * 3) / 2;
  tpie::log_info() << "|  | largest OBDD (unreduced): " << largest_unreduced << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_unreduced_MB > 0 ? std::to_string(largest_unreduced_MB) : "< 1")
                                                        << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  std::sort(apply_unreduced_sizes.begin(), apply_unreduced_sizes.end(), std::less<>());
  size_t acc_unreduced = 0;
  for (const size_t s : apply_unreduced_sizes) {
    acc_unreduced += s;
  }
  assert(apply_unreduced_sizes.size() == number_of_applies);

  tpie::log_info() << "|  | apply:" << std::endl;

  tpie::log_info() << "|  |  | avg OBDD (unreduced):     " << float(acc_unreduced) / float(number_of_applies) << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/8 OBDD (unreduced):     " << apply_unreduced_sizes[number_of_applies / 8] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/4 OBDD (unreduced):     " << apply_unreduced_sizes[number_of_applies / 4] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/2 OBDD (unreduced):     " << apply_unreduced_sizes[number_of_applies / 2] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 3/4 OBDD (unreduced):     " << apply_unreduced_sizes[(number_of_applies * 3) / 4] << " nodes" << std::endl;

  std::sort(exists_unreduced_sizes.begin(), exists_unreduced_sizes.end(), std::less<>());
  acc_unreduced = 0;
  for (const size_t s : exists_unreduced_sizes) {
    acc_unreduced += s;
  }
  assert(exists_unreduced_sizes.size() == number_of_exists);

  tpie::log_info() << "|  | exists:" << std::endl;

  tpie::log_info() << "|  |  | avg OBDD (unreduced):     " << float(acc_unreduced) / float(number_of_exists) << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/8 OBDD (unreduced):     " << exists_unreduced_sizes[number_of_exists / 8] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/4 OBDD (unreduced):     " << exists_unreduced_sizes[number_of_exists / 4] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 1/2 OBDD (unreduced):     " << exists_unreduced_sizes[number_of_exists / 2] << " nodes" << std::endl;

  tpie::log_info() << "|  |  | 3/4 OBDD (unreduced):     " << exists_unreduced_sizes[(number_of_exists * 3) / 4] << " nodes" << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | sink ratio (apply): " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_apply_sink_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_apply_sink_ratio / float(number_of_applies) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_apply_sink_ratio << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | sink ratio (exists): " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_exists_sink_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_exists_sink_ratio / float(number_of_exists) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_exists_sink_ratio << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  auto largest_reduced_MB = MB_of_size(largest_reduced);
  tpie::log_info() << "|  | largest OBDD (reduced)  : " << largest_reduced << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_reduced_MB > 0 ? std::to_string(largest_reduced_MB) : "< 1")
                                                        << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | reduction ratio (apply): " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_apply_reduction_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_apply_reduction_ratio / float(number_of_applies) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_apply_reduction_ratio << std::endl;

  tpie::log_info() << "|  | reduction ratio (exists): " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_exists_reduction_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_exists_reduction_ratio / float(number_of_exists) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_exists_reduction_ratio << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  auto final_MB = MB_of_size(sat_acc.size());
  tpie::log_info() << "|  | final size: " << sat_acc.size() << " nodes"<< std::endl;
  tpie::log_info() << "|  |             " << (final_MB > 0 ? std::to_string(final_MB) : "< 1") << " MB"<< std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | time: " << std::endl;
  tpie::log_info() << "|  |  | CNF construction: " << duration_of(t1, t2) << " ms" << std::endl;
  tpie::log_info() << "|  |  | OBDD solving: " << duration_of(t3, t4) << << " ms" std::endl;


  // ===== COOM =====
  // Close all of COOM down again
  coom::coom_deinit();

  // Return 'all good'
  exit(satisfiable ? -1 : 0);
}
