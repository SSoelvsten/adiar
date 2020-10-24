#include <vector>
#include <chrono>

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
size_t largest_nodes = 0;

size_t init_nodes = 0;

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
coom::node_file construct_is_not_winning(std::array<coom::label_t, 4>& line)
{
  uint64_t idx = 4 - 1;

  coom::ptr_t no_Xs_false = coom::create_sink_ptr(false);
  coom::ptr_t no_Xs_true = coom::create_sink_ptr(true);

  coom::ptr_t some_Xs_true = coom::create_sink_ptr(false);

  coom::node_file out;
  coom::node_writer out_writer(out);

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
      out_writer << some_Xs;
    }
    out_writer << no_Xs;

    no_Xs_false = no_Xs.uid;
    if (idx == 1) { // The next is the root?
      no_Xs_true = some_Xs.uid;
    }

    some_Xs_true = some_Xs.uid;
  } while (idx-- > 0);

  return out;
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
coom::node_file construct_is_tie(uint64_t N)
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

  coom::node_file out = coom::bdd_counter(0, 63, N);

  init_nodes = bdd_nodecount(out);

  unsigned int idx = 0;
  for (auto &line : lines) {
    coom::node_file next_not_winning = construct_is_not_winning(line);

    out = coom::bdd_apply(out, next_not_winning, coom::and_op);

    largest_nodes = std::max(largest_nodes, reduce(out).size());

    idx++;
  }

  return out;
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

  auto before_tie = get_timestamp();
  coom::node_file is_tie = construct_is_tie(N);
  auto after_tie = get_timestamp();

  tpie::log_info() << "|  | constraints: 76 lines" << std::endl;
  tpie::log_info() << "|  | time: " << duration_of(before_tie, after_tie) << " s" << std::endl;

  tpie::log_info() << "|  | initial size: " << init_nodes << " nodes" << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | largest OBDD  : " << largest_nodes << " nodes" << std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "|  | final size: " << is_tie.size() << " nodes"<< std::endl;
  tpie::log_info() << "|  |             " << is_tie.file_size() / 1024 / 1024 << " MB"<< std::endl;
  tpie::log_info() << "|  |" << std::endl;

  tpie::log_info() << "| Tic-Tac-Toe (" << N << ") : Counting ties"  << std::endl;
  auto before_count = get_timestamp();
  uint64_t solutions = coom::bdd_satcount(is_tie);
  auto after_count = get_timestamp();

  tpie::log_info() << "|  | number of ties: " << solutions << std::endl;
  tpie::log_info() << "|  | time: " << duration_of(before_count, after_count) << " s" << std::endl;

  // ===== COOM =====
  // Close all of COOM down again
  coom::coom_deinit();

  // Return 'all good'
  exit(N >= 25 || solutions == expected[N] ? 0 : 1);
}

