#include <vector>
#include <chrono>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

// COOM Imports
#include <coom/data.cpp>
#include <coom/assignment.cpp>

#include <coom/debug_data.cpp>
#include <coom/debug_assignment.cpp>
#include <coom/debug.cpp>
#include <coom/dot.cpp>

#include <coom/assert.cpp>

#include <coom/build.cpp>

#include <coom/apply.cpp>
#include <coom/pred.cpp>
#include <coom/count.cpp>
#include <coom/reduce.cpp>
#include <coom/restrict.cpp>
#include <coom/negate.cpp>

/*******************************************************************************
 * We base our example for N-Queens on the procedure as described in the paper
 * "Parallel Disk-Based Computation for Large, Monolithic Binary Decision
 * Diagrams" by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.
 *
 * We'd find it interesting to output the size of the largest OBDD, so we create
 * the following global variables. Also it would be of interest to see the best
 * and worst ratio between sink and node arcs in the unreduced OBDDs
 */
size_t largest_unreduced = 0;

float best_sink_ratio = 0.0;
float worst_sink_ratio = 1.0;

inline void stats_unreduced(size_t node_arcs, size_t sink_arcs)
{
  size_t total_arcs = node_arcs + sink_arcs;
  largest_unreduced = std::max(largest_unreduced, total_arcs / 2);

  float sink_ratio = float(sink_arcs) / float(total_arcs);
  best_sink_ratio = std::max(best_sink_ratio, sink_ratio);
  worst_sink_ratio = std::min(worst_sink_ratio, sink_ratio);
}

size_t largest_reduced = 0;

float best_reduction_ratio = 1.0;
float worst_reduction_ratio = 0.0;

inline void stats_reduced(size_t unreduced_size, size_t reduced_size)
{
  largest_reduced = std::max(largest_reduced, reduced_size);

  float reduction_ratio = float(reduced_size) / float(unreduced_size);
  best_reduction_ratio = std::min(best_reduction_ratio, reduction_ratio);
  worst_reduction_ratio = std::max(worst_reduction_ratio, reduction_ratio);
}

/*******************************************************************************
 *                             Variable ordering
 *
 * To solve the N-Queens problem, we have to first choose some encoding of the
 * problem. Well stay with the simple row-by-row ordering of variables for now
 * (see issue #42 for other suggestions). That is, we represent the position
 * (i,j) on the N x N board board as the variable with label:
 *
 *                                 N*i + j.
 */
inline coom::label_t label_of_position(uint64_t N, uint64_t i, uint64_t j)
{
  return (N * i) + j;
}

/*******************************************************************************
 *
 * Let us first restrict our attention to the base case of expressing a single
 * field (i,j). We need to express that a single queen is placed here, and that
 * it is in no conflict with any other queens on the board; queens on the same
 * row, column or diagonals. A single queen placed on (i,j) creates one single
 * set of other
 *        __________
 *       | \ | /    |
 *       |  \|/     |
 *       |---X------|
 *       |  /|\     |
 *       |_/_|_\____|
 *
 * This essentially is the formula x_ij /\ !conflicts(i,j), where conflicts(i,j)
 * is true if one or more queens are placed on conflicting positions.
 *
 * We could construct this with the builders of COOM, but we can do even better
 * than that! Since the resulting (reduced) OBDD is very well structured, we can
 * explicitly construct it. All OBDDs are stored on disk bottom-up, so we'll
 * have to start at the bottom rightmost and work ourselves backwards. We can
 * skip all labels for positions that are not in conflict or are not (i,j). All
 * the others, we have to check that they are not true, and for (i,j) that it
 * is.
 *
 *            (k1,l1)
 *             /   \
 *       (k2,l2)   F
 *        /   \
 *        |   F
 *        .
 *        |
 *      (i,j)
 *       / \
 *      T  |
 *         .
 *         |
 *     (k3n,l3n)
 *       /   \
 *       T   F
 *
 * Since we construct it explicitly, then the work we do in the base case goes
 * down to only O(N) time and O(N/B) I/Os rather than O(sort(N)) in both time
 * and I/Os. One pretty much cannot do this base case faster.
 */
void n_queens_S(uint64_t N,
                uint64_t i, uint64_t j,
                tpie::file_stream<coom::node_t>& out_nodes)
{
  out_nodes.open();

  uint64_t row = N - 1;
  coom::ptr_t next = coom::create_sink_ptr(true);

  do {
    uint64_t row_diff = std::max(row,i) - std::min(row,i);

    if (row_diff == 0) {
      // On row of the queen in question
      uint64_t column = N - 1;
      do {
        coom::label_t label = label_of_position(N, row, column);

        // If (row, column) == (i,j), then the chain goes through high.
        if (column == j) {
          // Node to check whether the queen actually is placed, and if so
          // whether all remaining possible conflicts have to be checked.
          coom::label_t label = label_of_position(N, i, j);
          coom::node_t queen = coom::create_node(label, 0, coom::create_sink_ptr(false), next);

          out_nodes.write(queen);
          next = queen.uid;
          continue;
        }

        coom::node_t out_node = coom::create_node(label, 0, next, coom::create_sink_ptr(false));

        out_nodes.write(out_node);
        next = out_node.uid;
      } while (column-- > 0);
    } else {
      // On another row
      if (j + row_diff < N) {
        // Diagonal to the right is within bounds
        coom::label_t label = label_of_position(N, row, j + row_diff);
        coom::node_t out_node = coom::create_node(label, 0, next, coom::create_sink_ptr(false));

        out_nodes.write(out_node);
        next = out_node.uid;
      }

      // Column
      coom::label_t label = label_of_position(N, row, j);
      coom::node_t out_node = coom::create_node(label, 0, next, coom::create_sink_ptr(false));

      out_nodes.write(out_node);
      next = out_node.uid;

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        coom::label_t label = label_of_position(N, row, j - row_diff);
        coom::node_t out_node = coom::create_node(label, 0, next, coom::create_sink_ptr(false));

        out_nodes.write(out_node);
        next = out_node.uid;
      }
    }
  } while (row-- > 0);

  largest_reduced = std::max(largest_reduced, out_nodes.size());
}

/*******************************************************************************
 * Now that we have a formula n_queens_S that is true only when the queen is
 * set, then we can combine them with an OR to ensure there is at-least-one
 * queen on the row. Since n_queens_S is also only true when said queen has no
 * conflicts, this also immediately contains all the at-most-one constraints on
 * these queens.
 *
 * We could do the below in two ways:
 *
 * - Recursively split the row in half until we reach the base case of
 *   n_queens_S. This will minimise the number of Apply's that we will make.
 *
 * - Iteratively combine them similar to a list.fold in functional programming
 *   languages. This will minimise the number of OBDDs that are "active" at any
 *   given time, since we only need to persist the input and output of each
 *   iteration.
 *
 * For COOM to be able to achieve optimality on disk, it sacrifices the
 * possibility of a hash-table to instantiate the entire forest of all currently
 * active OBDDs. In other words, each OBDD is completely separate and no memory
 * is saved, if there is a major overlap. So, we will choose to do it
 * iteratively.
 */
void n_queens_R(uint64_t N,
                uint64_t row,
                tpie::file_stream<coom::node_t>& out_nodes)
{
  /* The main interface for the functions of COOM are of the form of one or more
   * input streams converted into one output node stream. Most of these
   * algorithms create in one sweep through the OBDD an intermediate result,
   * which is then reduced afterwards in another sweep back again. The important
   * thing to notice is, that the reduction is only dependent on the
   * intermediate result and not the given input streams.
   *
   * When calling coom::apply(in1, in2, op, out) then even though in1 and in2
   * may not be needed after the first sweep, the C++ compiler first sees it
   * being a dead variable after the entire procedure has finished. That means,
   * that both the input, the intermediate result and the output may have to
   * exist in memory or on disk at the same time.
   *
   * If we explicitly call the reduce, we can make sure both inputs are cleaned
   * up before we reduce us to the output. This will minimise the amount of
   * space we concurrently occupy.
   */
  tpie::file_stream<coom::arc_t> reduce_node_arcs;
  tpie::file_stream<coom::arc_t> reduce_sink_arcs;

  tpie::file_stream<coom::node_t> next_S;

  out_nodes.open();
  n_queens_S(N, row, 0, out_nodes);

  for (uint64_t j = 1; j < N; j++) {
    next_S.open();
    n_queens_S(N, row, j, next_S);

    reduce_node_arcs.open();
    reduce_sink_arcs.open();

    coom::apply(out_nodes, next_S, coom::or_op, reduce_node_arcs, reduce_sink_arcs);

    // close (and clean up) prior result
    next_S.close();
    out_nodes.close();

    stats_unreduced(reduce_node_arcs.size(), reduce_sink_arcs.size());

    // open for next result
    out_nodes.open();

    coom::reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);

    stats_reduced((reduce_node_arcs.size() + reduce_sink_arcs.size()) / 2, out_nodes.size());

    reduce_node_arcs.close();
    reduce_sink_arcs.close();
  }
}

/*******************************************************************************
 * Now that we have each entire row done, we only need to combine them. Here we
 * again iterate over all rows to combine them one-by-one. One can probably
 * remove the code duplication that we now introduce.
 */
void n_queens_B(uint64_t N, tpie::file_stream<coom::node_t>& out_nodes)
{
  if (N == 1) {
    n_queens_S(N, 0, 0, out_nodes);
    return;
  }

  tpie::file_stream<coom::arc_t> reduce_node_arcs;
  tpie::file_stream<coom::arc_t> reduce_sink_arcs;

  tpie::file_stream<coom::node_t> next_R;

  out_nodes.open();
  n_queens_R(N, 0, out_nodes);

  for (uint64_t i = 1; i < N; i++) {
    n_queens_R(N, i, next_R);

    reduce_node_arcs.open();
    reduce_sink_arcs.open();

    coom::apply(out_nodes, next_R, coom::and_op, reduce_node_arcs, reduce_sink_arcs);

    next_R.close();
    out_nodes.close();

    stats_unreduced(reduce_node_arcs.size(), reduce_sink_arcs.size());

    out_nodes.open();

    coom::reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);

    stats_reduced((reduce_node_arcs.size() + reduce_sink_arcs.size()) / 2, out_nodes.size());

    reduce_node_arcs.close();
    reduce_sink_arcs.close();
  }
}

/*******************************************************************************
 * So now that we have a single OBDD that is only true, when we have placed a
 * queen on each row and none in conflict, it is only true, when the assignment
 * is a solution to the N-Queens problem. So, now we can merely count the number
 * of assignments that reach a true sink.
 */
uint64_t n_queens_count(tpie::file_stream<coom::node_t>& board)
{
  return coom::count_assignments(board, coom::is_true);
}

/*******************************************************************************
 *
 * Based on: github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex
 *
 * If we want to list all the assignments, we have to do something more than
 * merely count the number of satisfying assignments, as we did above. We could
 * then iterate over all possible assignments and use the OBDD to prune our
 * search tree.
 *
 * We again construct the combined OBDD of the at-most-one restrictions with the
 * at-least-one restriction for the rows. Then, starting at the left-most column
 * we may attempt an assignment in each spot, and recurse. Recursion can be
 * stopped early in two cases:
 *
 *   - If the given OBDD already is trivially false. We have placed a queen,
 *     such that it conflicts with another.
 *
 *   - If the number of unique paths in the restricted OBDD is exactly one. Then
 *     we are forced to place the remaining queens.
 *
 * Since we want to back track in our choices, we may keep OBDDs for each
 * column. This is easily achieved by writing it as a recursive procedure. One
 * should notice though, that this will result in multiple OBDDs concurrently
 * being instantiated in memory or on disk at the same time.
 *
 * In fact, let us keep track of that!
 */
uint64_t deepest_column = 0;

/* Let us isolate/hide the printing function, to keep the rest a little more
 * clean. */
void n_queens_print_solution(std::vector<uint64_t>& assignment)
{
  tpie::log_info() << "|  |  | ";
  for (uint64_t r : assignment) {
    tpie::log_info() << r << " ";
  }
  tpie::log_info() << std::endl;
}

/* At this point, we now also need to convert an assignment back into a position
 * on the board. So, we'll also need the following two small functions. */
inline uint64_t i_of_label(uint64_t N, coom::label_t label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, coom::label_t label)
{
  return label % N;
}

/* At which point we can then implement the recursive procedure that takes care
 * of a row and possibly recurses. For a sanity check, we also count the number
 * of solutions we list. */
uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       tpie::file_stream<coom::node_t>& constraints)
{
  if (coom::is_sink(constraints, coom::is_false)) {
    return 0;
  }
  deepest_column = std::max(deepest_column, column);

  uint64_t solutions = 0;

  tpie::file_stream<coom::assignment_t> column_assignment;
  tpie::file_stream<coom::node_t> restricted_constraints;

  for (uint64_t row_q = 0; row_q < N; row_q++) {
    partial_assignment.push_back(row_q);

    // Construct the assignment for this entire column
    column_assignment.open();

    for (uint64_t row = 0; row < N; row++) {
      coom::assignment assignment = { label_of_position(N, row, column), row == row_q };
      column_assignment.write(assignment);
    }

    restricted_constraints.open();

    coom::restrict(constraints, column_assignment, restricted_constraints);

    if (coom::count_paths(restricted_constraints, coom::is_true) == 1) {
      solutions += 1;

      tpie::file_stream<coom::assignment> forced_assignment;
      forced_assignment.open();

      // Request a true assignment (well, only one exists), and have it ordered
      // by the columns, such that we can use the output for our purposes.
      auto sort_by_column = [&N](const coom::assignment &a, const coom::assignment &b) -> bool {
          return j_of_label(N, a.label) < j_of_label(N, b.label);
        };

      coom::get_assignment<decltype(sort_by_column)>(restricted_constraints,
                                                     coom::is_true,
                                                     forced_assignment,
                                                     sort_by_column);

      forced_assignment.seek(0);
      while (forced_assignment.can_read()) {
        coom::assignment a = forced_assignment.read();
        if (a.value) {
          partial_assignment.push_back(i_of_label(N, a.label));
        }
      }

      n_queens_print_solution(partial_assignment);

      for (uint64_t c = N-1; c > column; c--) {
        partial_assignment.pop_back();
      }
    } else if (coom::is_sink(restricted_constraints, coom::is_true)) {
      n_queens_print_solution(partial_assignment);
      solutions += 1;
    } else {
      solutions += n_queens_list(N, column+1, partial_assignment, restricted_constraints);
    }
    column_assignment.close();
    restricted_constraints.close();
    partial_assignment.pop_back();
  }

  return solutions;
}

uint64_t n_queens_list(uint64_t N, tpie::file_stream<coom::node_t>& amo_alo)
{
  tpie::log_info() << "|  | solutions:" << std::endl;

  if (N == 1) {
    /* To make the recursive function work for N = 1 we would have to have the
     * coom::count_paths check above at the beginning. That would in all other
     * cases merely result in an unecessary counting of paths at the very
     * start. */
    std::vector<uint64_t> assignment { 0 };
    n_queens_print_solution(assignment);

    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  return n_queens_list(N, 0, partial_assignment, amo_alo);
}

// expected number taken from:
//  https://en.wikipedia.org/wiki/Eight_queens_puzzle#Counting_solutions
uint64_t expected_result[28] = {
  0,
  1,
  0,
  0,
  2,
  10,
  4,
  40,
  92,
  352,
  724,
  2680,
  14200,
  73712,
  365596,
  2279184,
  14772512,
  95815104,
  666090624,
  4968057848,
  39029188884,
  314666222712,
  2691008701644,
  24233937684440,
  227514171973736,
  2207893435808352,
  22317699616364044,
  234907967154122528
};


/* A few chrono wrappers to improve readability of the code below */
inline auto get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline auto duration_of(std::chrono::high_resolution_clock::time_point &before,
                        std::chrono::high_resolution_clock::time_point &after) {
  return std::chrono::duration_cast<std::chrono::seconds>(after - before).count();
}


/* TODO: File size calculations should be available from COOM. */
inline auto MB_of_size(tpie::stream_size_type size) {
  return size * sizeof(coom::node) /* bytes */ / (1024 * 1024) /* in MB */;
}

/* Finally, to run the above we need to initialize the underlying TPIE library
 * first, from which point all the file_streams, priority queues and sorters
 * used are at our disposal.
 */
int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  uint64_t N = 0;
  size_t M = 1;

  try {
    if (argc == 1) {
      tpie::log_info() << "Missing argument for N and M" << std::endl;
    } else {
      N = std::stoi(argv[1]);
      if (N == 0 || N > 27) {
        tpie::log_info() << "N should be in interval [1;27]: " << argv[1] << std::endl;
        N = 0;
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

  if (N == 0 || M <= 0) {
    exit(1);
  }

  // ===== TPIE =====
  // Initialize
  tpie::tpie_init();

  tpie::get_memory_manager().set_limit(M * 1024 * 1024 * 1024);

  tpie::log_info() << "| Initialized TPIE with " << M << " GB of memory"  << std::endl << "|" << std::endl;

  // ===== N Queens =====

  tpie::file_stream<coom::node> board;

  tpie::log_info() << "| " << N << "-Queens : Board construction"  << std::endl;
  auto before_board = get_timestamp();
  n_queens_B(N, board);
  auto after_board = get_timestamp();

  tpie::log_info() << "|  | time: " << duration_of(before_board, after_board) << " s" << std::endl;

  auto largest_unreduced_MB = (MB_of_size(largest_unreduced) * 3) / 2;
  tpie::log_info() << "|  | largest OBDD (unreduced): " << largest_unreduced << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_unreduced_MB > 0 ? std::to_string(largest_unreduced_MB) : "< 1")
                                                        << " MB"<< std::endl;

  tpie::log_info() << "|  | sink ratio: " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_sink_ratio << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_sink_ratio << std::endl;

  auto largest_reduced_MB = MB_of_size(largest_reduced);
  tpie::log_info() << "|  | largest OBDD (reduced)  : " << largest_reduced << " nodes" << std::endl;
  tpie::log_info() << "|  |                           " << (largest_reduced_MB > 0 ? std::to_string(largest_reduced_MB) : "< 1")
                                                        << " MB"<< std::endl;

  tpie::log_info() << "|  | reduction ratio: " << std::endl;
  tpie::log_info() << "|  |  | best:  " << best_reduction_ratio << std::endl;
  tpie::log_info() << "|  |  | worst: " << worst_reduction_ratio << std::endl;


  auto final_MB = MB_of_size(board.size());
  tpie::log_info() << "|  | final size: " << board.size() << " nodes"<< std::endl;
  tpie::log_info() << "|  |             " << (final_MB > 0 ? std::to_string(final_MB) : "< 1") << " MB"<< std::endl;

  // Run counting example
  auto before_count = get_timestamp();
  uint64_t solutions = n_queens_count(board);
  auto after_count = get_timestamp();

  tpie::log_info() << "| " << N << "-Queens : Counting assignments"  << std::endl;
  tpie::log_info() << "|  | number of solutions: " << solutions << std::endl;
  tpie::log_info() << "|  | time: " << duration_of(before_count, after_count) << " s" << std::endl;

  bool correct_result = solutions == expected_result[N];

  // Run enumeration example (for reasonably small N)
  if (N <= 8) {
    tpie::log_info() << "| " << N << "-Queens (Pruning search)"  << std::endl;
    auto before_list = get_timestamp();
    uint64_t listed_solutions = n_queens_list(N, board);
    auto after_list = get_timestamp();

    correct_result = correct_result && listed_solutions == expected_result[N];

    tpie::log_info() << "|  | number of solutions: " << listed_solutions << std::endl;
    tpie::log_info() << "|  | deepest recursion: " << deepest_column << " columns" << std::endl;

    auto estimated_MB = MB_of_size(deepest_column * 2 * board.size());
    tpie::log_info() << "|  | estimated maximal memory usage: " << (estimated_MB > 0 ? std::to_string(estimated_MB) : "< 1") << " MB" << std::endl;
    tpie::log_info() << "|  | time: " << duration_of(before_list, after_list) << " s" << std::endl;
  }

  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(correct_result ? 0 : 1);
}
