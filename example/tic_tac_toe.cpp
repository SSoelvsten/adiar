#include <vector>
#include <chrono>
#include <string.h>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

// COOM Imports
#include <coom/coom.h>


/*******************************************************************************
 * We base our example for 3D 4x4x4 Tic-Tac-Toe on the procedure as described in
 * the paper "Parallel Disk-Based Computation for Large, Monolithic Binary
 * Decision Diagrams" by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.
 *
 * The question to be answered is as follows: When Player X marks N spaces and
 * Player O marks the remaining 4^3 - N = 64 - N spaces, how many of these
 * placements are ties?
 *
 * We'd find it interesting to output the size of the largest OBDD, so we create
 * the following global variables. Also it would be of interest to see the best
 * and worst ratio between sink and node arcs in the unreduced OBDDs
 */
std::vector<size_t> unreduced_sizes;

float best_sink_ratio = 0.0;
float acc_sink_ratio = 0.0;
float worst_sink_ratio = 1.0;

inline void stats_unreduced(size_t node_arcs, size_t sink_arcs)
{
  size_t total_arcs = node_arcs + sink_arcs;
  unreduced_sizes.push_back(float(total_arcs) / 2.0);

  float sink_ratio = float(sink_arcs) / float(total_arcs);
  best_sink_ratio = std::max(best_sink_ratio, sink_ratio);
  acc_sink_ratio += sink_ratio;
  worst_sink_ratio = std::min(worst_sink_ratio, sink_ratio);
}

size_t largest_reduced = 0;

float best_reduction_ratio = 1.0;
float acc_reduction_ratio = 0.0;
float worst_reduction_ratio = 0.0;

inline void stats_reduced(size_t unreduced_size, size_t reduced_size)
{
  largest_reduced = std::max(largest_reduced, reduced_size);

  float reduction_ratio = float(reduced_size) / float(unreduced_size);
  best_reduction_ratio = std::min(best_reduction_ratio, reduction_ratio);
  acc_reduction_ratio += reduction_ratio;
  worst_reduction_ratio = std::max(worst_reduction_ratio, reduction_ratio);
}


/*******************************************************************************
 *                             Variable ordering
 *
 * We again go dimension by dimension in the variable ordering, assuming that
 *
 *                            0 <= i, j, k <= 4
 *
 * We interpret x_ijk as true, if an X is placed on (i,j,k).
 */
inline coom::label_t label_of_position(uint64_t i, uint64_t j, uint64_t k)
{
  return (4 * 4 * i) + (4 * j) + k;
}


/*******************************************************************************
 * We need to encapsulate two restrictions for all rows, columns, and diagonals
 * for it to be a tie:
 *
 * - Not all four spaces are marked by X?
 * - At least one space has an X.
 *
 * This again is very well structured and can be easily constructed explicitly.
 */
void construct_is_not_winning(std::array<coom::label_t, 4>& line,
                              tpie::file_stream<coom::node_t>& out_nodes,
                              tpie::file_stream<coom::meta_t>& out_meta)
{
  out_nodes.open();
  out_meta.open();
  uint64_t idx = 4 - 1;

  coom::ptr_t no_Xs_false = coom::create_sink_ptr(false);
  coom::ptr_t no_Xs_true = coom::create_sink_ptr(true);

  coom::ptr_t some_Xs_true = coom::create_sink_ptr(false);

  do {
    coom::node_t no_Xs = coom::create_node(line[idx], 0,
                                           no_Xs_false,
                                           no_Xs_true);

    coom::node_t some_Xs = coom::create_node(line[idx], 1,
                                             coom::create_sink_ptr(true),
                                             some_Xs_true);

    /* Notice, we have to write bottom-up. That is actually more precisely in
     * reverse topological order which also includes the id's. */
    if (idx != 0) {
      out_nodes.write(some_Xs);
    }
    out_nodes.write(no_Xs);
    out_meta.write({line[idx]});

    no_Xs_false = no_Xs.uid;
    if (idx == 1) { // The next is the root?
      no_Xs_true = some_Xs.uid;
    }

    some_Xs_true = some_Xs.uid;
  } while (idx-- > 0);
}

/*******************************************************************************
 * Then we need to combine it for all rows, columns and the 4 diagonals to
 * ensure that no rows actually are winning and the game ends in a tie.
 *
 * We'll place these constraints one by one onto an OBDD that is only true if
 * exactly N X's are placed. This OBDD is also easy to create. In fact, it is so
 * simple and universal, that COOM already has a builder that creates it in
 * linear time and linear I/O with regards to the output size.
 *
 * The order in which one picks applies something together is very important. An
 * Apply technically just constructs the product of both OBDDs; each state
 * reflects the states of both input OBDDs. From experiments it seems, that one
 * achieves the smallest intermediate size by starting with the ones, where the
 * difference between the label-value for the first cell and the fourth cell is
 * as small as possible.
 */
size_t construct_is_tie(uint64_t N,
                        tpie::file_stream<coom::node_t>& out_nodes,
                        tpie::file_stream<coom::meta_t>& out_meta)
{
  // Compute all rows, columns, and diagonals. Most likely the optimiser already
  // precomputes this one.
  std::vector<std::array<coom::label_t,4>> lines { };

  // 4 planes and the rows in these
  for (uint64_t i = 0; i < 4; i++) { // (dist: 4)
    for (uint64_t j = 0; j < 4; j++) {
      lines.push_back({ label_of_position(i,j,0), label_of_position(i,j,1), label_of_position(i,j,2), label_of_position(i,j,3) });
    }
  }
  // 4 planes and a diagonal within
  for (uint64_t i = 0; i < 4; i++) { // (dist: 10)
    lines.push_back({ label_of_position(i,0,3), label_of_position(i,1,2), label_of_position(i,2,1), label_of_position(i,3,0) });
  }
  // 4 planes... again, now the columns
  for (uint64_t i = 0; i < 4; i++) { // (dist: 13)
    for (uint64_t k = 0; k < 4; k++) {
      lines.push_back({ label_of_position(i,0,k), label_of_position(i,1,k), label_of_position(i,2,k), label_of_position(i,3,k) });
    }
  }
  // 4 planes and the other diagonal within
  for (uint64_t i = 0; i < 4; i++) { // (dist: 16)
    lines.push_back({ label_of_position(i,0,0), label_of_position(i,1,1), label_of_position(i,2,2), label_of_position(i,3,3) });
  }

  // Diagonal of the entire cube (dist: 22)
  lines.push_back({ label_of_position(0,3,3), label_of_position(1,2,2), label_of_position(2,1,1), label_of_position(3,0,0) });

  // Diagonal of the entire cube (dist: 40)
  lines.push_back({ label_of_position(0,3,0), label_of_position(1,2,1), label_of_position(2,1,2), label_of_position(3,0,3) });

  // Diagonals in the vertical planes
  for (uint64_t j = 0; j < 4; j++) { // (dist: 46)
    lines.push_back({ label_of_position(0,j,3), label_of_position(1,j,2), label_of_position(2,j,1), label_of_position(3,j,0) });
  }

  // 16 vertical lines (dist: 48)
  for (uint64_t j = 0; j < 4; j++) {
    for (uint64_t k = 0; k < 4; k++) {
      lines.push_back({ label_of_position(0,j,k), label_of_position(1,j,k), label_of_position(2,j,k), label_of_position(3,j,k) });
    }
  }

  // Diagonals in the vertical planes
  for (uint64_t j = 0; j < 4; j++) { // (dist: 49)
    lines.push_back({ label_of_position(0,j,0), label_of_position(1,j,1), label_of_position(2,j,2), label_of_position(3,j,3) });
  }

  for (uint64_t k = 0; k < 4; k++) { // (dist: 36)
    lines.push_back({ label_of_position(0,3,k), label_of_position(1,2,k), label_of_position(2,1,k), label_of_position(3,0,k) });
  }
  for (uint64_t k = 0; k < 4; k++) { // (dist: 60)
    lines.push_back({ label_of_position(0,0,k), label_of_position(1,1,k), label_of_position(2,2,k), label_of_position(3,3,k) });
  }

  // The 4 diagonals of the entire cube (dist: 61)
  lines.push_back({ label_of_position(0,0,3), label_of_position(1,1,2), label_of_position(2,2,1), label_of_position(3,3,0) });

  // The 4 diagonals of the entire cube (dist: 64)
  lines.push_back({ label_of_position(0,0,0), label_of_position(1,1,1), label_of_position(2,2,2), label_of_position(3,3,3) });

  tpie::file_stream<coom::arc_t> reduce_node_arcs;
  tpie::file_stream<coom::arc_t> reduce_sink_arcs;
  tpie::file_stream<coom::meta_t> reduce_meta;

  tpie::file_stream<coom::node_t> next_not_winning;
  tpie::file_stream<coom::meta_t> next_not_winning_meta;

  out_nodes.open();
  out_meta.open();
  coom::build_counter(0, 63, N, out_nodes, out_meta);

  size_t operations = 0;
  for (auto &line : lines) {
    next_not_winning.open();
    next_not_winning_meta.open();

    construct_is_not_winning(line, next_not_winning, next_not_winning_meta);

    reduce_node_arcs.open();
    reduce_sink_arcs.open();
    reduce_meta.open();

    coom::apply(out_nodes, out_meta,
                next_not_winning, next_not_winning_meta,
                coom::and_op,
                reduce_node_arcs, reduce_sink_arcs, reduce_meta);

    // close (and clean up) prior result
    out_nodes.close();
    out_meta.close();
    next_not_winning.close();
    next_not_winning_meta.close();

    stats_unreduced(reduce_node_arcs.size(), reduce_sink_arcs.size());

    // open for next result
    out_nodes.open();
    out_meta.open();

    coom::reduce(reduce_node_arcs, reduce_sink_arcs, reduce_meta, out_nodes, out_meta);

    stats_reduced((reduce_node_arcs.size() + reduce_sink_arcs.size()) / 2, out_nodes.size());

    reduce_node_arcs.close();
    reduce_sink_arcs.close();
    reduce_meta.close();

    operations++;
    if (coom::is_sink(out_nodes)) {
      break;
    }
  }

  return operations;
}

/* A few chrono wrappers to improve readability of the code below */
inline auto get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline auto duration_of(std::chrono::high_resolution_clock::time_point &before,
                        std::chrono::high_resolution_clock::time_point &after) {
  return std::chrono::duration_cast<std::chrono::seconds>(after - before).count();
}

/* Expected numbers taken from "Parallel Disk-Based Computation for Large,
 * Monolithic Binary Decision Diagrams" by Daniel Kunkle, Vlad Slavici, and Gene
 * Cooperman. */
uint64_t expected[25] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  304,
  136288,
  9734400,
  296106640,
  5000129244,
};

/* TODO: File size calculations should be available from COOM. */
inline auto MB_of_size(tpie::stream_size_type size) {
  return size * sizeof(coom::node) /* bytes */ / (1024 * 1024) /* in MB */;
}
int main(int argc, char* argv[])
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

  // ===== Tic-Tac-Toe =====

  tpie::log_info() << "| Tic-Tac-Toe (" << N << ") : Is-tie construction"  << std::endl;
  tpie::file_stream<coom::node_t> is_tie;
  tpie::file_stream<coom::meta_t> is_tie_meta;

  auto before_tie = get_timestamp();
  size_t operations = construct_is_tie(N, is_tie, is_tie_meta);
  auto after_tie = get_timestamp();

  tpie::log_info() << "|  | constraints:" << std::endl;
  tpie::log_info() << "|  |   | total:   76 lines" << std::endl;
  tpie::log_info() << "|  |   | done:    " << operations << " lines" << std::endl;
  tpie::log_info() << "|  | time: " << duration_of(before_tie, after_tie) << " s" << std::endl;

  auto init_size = (N+1)*64-(N*N); // See coom::build_counter implementation
  auto init_MB = MB_of_size(init_size);
  tpie::log_info() << "|  | initial size: " << init_size << " nodes" << std::endl;
  tpie::log_info() << "|  |               " << (init_MB > 0 ? std::to_string(init_MB) : "< 1") << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  std::sort(unreduced_sizes.begin(), unreduced_sizes.end(), std::less<>());

  size_t acc_unreduced = 0;
  size_t largest_unreduced = 0;
  for (const size_t s : unreduced_sizes) {
    acc_unreduced += s;
    largest_unreduced = std::max(largest_unreduced, s);
  }
  assert(unreduced_sizes.size() == operations);

  auto largest_unreduced_MB = (MB_of_size(unreduced_sizes[operations-1]) * 3) / 2;
  tpie::log_info() << "|  | largest OBDD (unreduced): " << unreduced_sizes[operations-1] << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_unreduced_MB > 0 ? std::to_string(largest_unreduced_MB) : "< 1")
                                                        << " MB"<< std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | avg OBDD (unreduced):     " << float(acc_unreduced) / float(operations) << " nodes" << std::endl;

  tpie::log_info() << "|  | 1/4 OBDD (unreduced):     " << unreduced_sizes[operations / 4] << " nodes" << std::endl;

  tpie::log_info() << "|  | 1/2 OBDD (unreduced):     " << unreduced_sizes[operations / 2] << " nodes" << std::endl;

  tpie::log_info() << "|  | 3/4 OBDD (unreduced):     " << unreduced_sizes[(operations * 3) / 4] << " nodes" << std::endl;

  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | sink ratio: " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_sink_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_sink_ratio / float(operations) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_sink_ratio << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  auto largest_reduced_MB = MB_of_size(largest_reduced);
  tpie::log_info() << "|  | largest OBDD (reduced)  : " << largest_reduced << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_reduced_MB > 0 ? std::to_string(largest_reduced_MB) : "< 1")
                                                        << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | reduction ratio: " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_reduction_ratio << std::endl;
  tpie::log_info() << "|  |  | avg:   " << acc_reduction_ratio / float(operations) << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_reduction_ratio << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  auto final_MB = MB_of_size(is_tie.size());
  tpie::log_info() << "|  | final size: " << is_tie.size() << " nodes"<< std::endl;
  tpie::log_info() << "|  |             " << (final_MB > 0 ? std::to_string(final_MB) : "< 1") << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "| Tic-Tac-Toe (" << N << ") : Counting ties"  << std::endl;
  auto before_count = get_timestamp();
  uint64_t solutions = coom::count_assignments(is_tie, is_tie_meta, coom::is_true);
  auto after_count = get_timestamp();

  tpie::log_info() << "|  | number of ties: " << solutions << std::endl;
  tpie::log_info() << "|  | time: " << duration_of(before_count, after_count) << " s" << std::endl;

  // ===== COOM =====
  // Close all of COOM down again
  coom::coom_deinit();

  // Return 'all good'
  exit(N >= 25 || solutions == expected[N] ? 0 : 1);
}

