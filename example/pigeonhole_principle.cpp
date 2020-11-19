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

/* A few chrono wrappers to improve readability of the code below */
inline auto get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline auto duration_of(std::chrono::high_resolution_clock::time_point &before,
                        std::chrono::high_resolution_clock::time_point &after) {
  return std::chrono::duration_cast<std::chrono::seconds>(after - before).count();
}

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
size_t largest_nodes = 0;
size_t largest_filesize = 0;

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
  coom::bdd sat_acc = coom::bdd_true();

  const auto sat_and_clause = [&](clause_t &clause) -> void
  {
    coom::node_file clause_bdd;

    { // All bdd functions require that no writer is attached to a file. So, we
      // garbage collect the writer before the bdd_apply call.
      coom::node_writer clause_writer(clause_bdd);

      coom::ptr_t next = coom::create_sink_ptr(false);

      for (auto it = clause.rbegin(); it != clause.rend(); it++) {
        literal_t v = *it;

        coom::node n = coom::create_node(v.first, 0,
                                         v.second ? coom::create_sink_ptr(true) : next,
                                         v.second ? next : coom::create_sink_ptr(true));

        next = n.uid;

        clause_writer << n;
      }
    }

    sat_acc &= clause_bdd;

    largest_nodes = std::max(largest_nodes, bdd_nodecount(sat_acc));
  };

  const auto sat_quantify_variable = [&](uint64_t var) -> void
  {
    sat_acc = coom::bdd_exists(sat_acc, var);

    number_of_exists++;
    largest_nodes = std::max(largest_nodes, bdd_nodecount(sat_acc));
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

  tpie::log_info() << "|  | largest OBDD : " << largest_nodes << " nodes" << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | final size: " << bdd_nodecount(sat_acc) << " nodes"<< std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | time: " << std::endl;
  tpie::log_info() << "|  |  | CNF construction: " << duration_of(t1, t2) << std::endl;
  tpie::log_info() << "|  |  | OBDD solving: " << duration_of(t3, t4) << std::endl;


  // ===== COOM =====
  // Close all of COOM down again
  coom::coom_deinit();

  // Return 'all good'
  exit(satisfiable ? -1 : 0);
}
