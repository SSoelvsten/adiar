#include <vector>
#include <chrono>

// TPIE Imports
#include <tpie/tpie.h>
#include <tpie/tpie_log.h>

// Adiar Imports
#include <adiar/adiar.h>


/*******************************************************************************
 * We base our example for N-Queens on the procedure as described in the paper
 * "Parallel Disk-Based Computation for Large, Monolithic Binary Decision
 * Diagrams" by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.
 *
 * We'd find it interesting to output the size of the largest BDD and the final
 * BDD.
 */
size_t largest_nodes = 0;

/*******************************************************************************
 *                             Variable ordering
 *
 * To solve the N-Queens problem, we have to first choose some encoding of the
 * problem. Well stay with the simple row-by-row ordering of variables for now.
 * That is, we represent the position (i,j) on the N x N board board as the
 * variable with label:
 *
 *                                 N*i + j.
 */
inline adiar::label_t label_of_position(uint64_t N, uint64_t i, uint64_t j)
{
  return (N * i) + j;
}

/*******************************************************************************
 *
 * Let us first restrict our attention to the base case of expressing the state
 * of a single field (i,j). We need to express that a single queen is placed
 * here, and that it is in no conflict with any other queens on the board;
 * queens on the same row, column or diagonals.
 *        __________
 *       | \ | /    |
 *       |  \|/     |
 *       |---X------|
 *       |  /|\     |
 *       |_/_|_\____|
 *
 * This essentially is the formula x_ij /\ !is_threatened(i,j), where
 * is_threatened(i,j) is true if one or more queens are placed on conflicting
 * positions.
 *
 * We could construct this with the builders of Adiar, but we can do even better
 * than that! Since the resulting (reduced) BDD is very well structured, we can
 * explicitly construct it. All BDDs are stored on disk bottom-up, so we'll have
 * to start at the bottom rightmost and work ourselves backwards. We can skip
 * all labels for positions that are not in conflict or are not (i,j). All the
 * others, we have to check that they are not true, and for (i,j) that it is.
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
adiar::bdd n_queens_S(uint64_t N, uint64_t i, uint64_t j)
{
  adiar::node_file out;

  { // When calling `out.size()` below, we have to make it read-only. So, we
    // have to detach the node_writer before we do. This is automatically done
    // on garbage collection, which is why we add an inner scope.
    adiar::node_writer out_writer(out);

    uint64_t row = N - 1;
    adiar::ptr_t next = adiar::create_sink_ptr(true);

    do {
      uint64_t row_diff = std::max(row,i) - std::min(row,i);

      if (row_diff == 0) {
        // On row of the queen in question
        uint64_t column = N - 1;
        do {
          adiar::label_t label = label_of_position(N, row, column);

          // If (row, column) == (i,j), then the chain goes through high.
          if (column == j) {
            // Node to check whether the queen actually is placed, and if so
            // whether all remaining possible conflicts have to be checked.
            adiar::label_t label = label_of_position(N, i, j);
            adiar::node_t queen = adiar::create_node(label, 0, adiar::create_sink_ptr(false), next);

            out_writer << queen;
            next = queen.uid;
            continue;
          }

          adiar::node_t out_node = adiar::create_node(label, 0, next, adiar::create_sink_ptr(false));

          out_writer << out_node;
          next = out_node.uid;
        } while (column-- > 0);
      } else {
        // On another row
        if (j + row_diff < N) {
          // Diagonal to the right is within bounds
          adiar::label_t label = label_of_position(N, row, j + row_diff);
          adiar::node_t out_node = adiar::create_node(label, 0, next, adiar::create_sink_ptr(false));

          out_writer << out_node;
          next = out_node.uid;
        }

        // Column
        adiar::label_t label = label_of_position(N, row, j);
        adiar::node_t out_node = adiar::create_node(label, 0, next, adiar::create_sink_ptr(false));

        out_writer << out_node;
        next = out_node.uid;

        if (row_diff <= j) {
          // Diagonal to the left is within bounds
          adiar::label_t label = label_of_position(N, row, j - row_diff);
          adiar::node_t out_node = adiar::create_node(label, 0, next, adiar::create_sink_ptr(false));

          out_writer << out_node;
          next = out_node.uid;
        }
      }
    } while (row-- > 0);
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
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
 *   languages. This will minimise the number of BDDs that are "active" at any
 *   given time, since we only need to persist the input and output of each
 *   iteration.
 *
 * For Adiar to be able to achieve optimality on disk, it sacrifices the
 * possibility of a hash-table to instantiate the entire forest of all currently
 * active BDDs. In other words, each BDD is completely separate and no memory is
 * saved, if there is a major overlap. So, we will choose to do it iteratively.
 */
adiar::bdd n_queens_R(uint64_t N, uint64_t row)
{
  adiar::bdd out = n_queens_S(N, row, 0);

  for (uint64_t j = 1; j < N; j++) {
    out |= n_queens_S(N, row, j);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}

/*******************************************************************************
 * Now that we have each entire row done, we only need to combine them. Here we
 * again iterate over all rows to combine them one-by-one. One can probably
 * remove the code duplication that we now introduce.
 */
adiar::bdd n_queens_B(uint64_t N)
{
  if (N == 1) {
    return n_queens_S(N, 0, 0);
  }

  adiar::bdd out = n_queens_R(N, 0);

  for (uint64_t i = 1; i < N; i++) {
    out &= n_queens_R(N, i);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}

/*******************************************************************************
 * So now that we have a single BDD that is only true, when we have placed a
 * queen on each row and none in conflict, it is only true, when the assignment
 * is a solution to the N-Queens problem. So, now we can merely count the number
 * of assignments that reach a true sink.
 */
uint64_t n_queens_count(const adiar::bdd &board)
{
  return adiar::bdd_satcount(board);
}

/*******************************************************************************
 *
 * Based on: github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex
 *
 * If we want to list all the assignments, we have to do something more than
 * merely count the number of satisfying assignments, as we did above. We could
 * then iterate over all possible assignments and use the BDD to prune our
 * search tree.
 *
 * We again construct the combined BDD of the at-most-one restrictions with the
 * at-least-one restriction for the rows. Then, starting at the left-most column
 * we may attempt an assignment in each spot, and recurse. Recursion can be
 * stopped early in two cases:
 *
 *   - If the given BDD already is trivially false. We have placed a queen, such
 *     that it conflicts with another.
 *
 *   - If the number of unique paths in the restricted BDD is exactly one. Then
 *     we are forced to place the remaining queens.
 *
 * Since we want to back track in our choices, we may keep BDDs for each column.
 * This is easily achieved by writing it as a recursive procedure. One should
 * notice though, that this will result in multiple BDDs concurrently being
 * instantiated in memory or on disk at the same time.
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
inline uint64_t i_of_label(uint64_t N, adiar::label_t label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, adiar::label_t label)
{
  return label % N;
}

/* At which point we can then implement the recursive procedure that takes care
 * of a row and possibly recurses. For a sanity check, we also count the number
 * of solutions we list. */
uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       const adiar::bdd& constraints)
{
  if (adiar::is_sink(constraints, adiar::is_false)) {
    return 0;
  }
  deepest_column = std::max(deepest_column, column);

  uint64_t solutions = 0;

  for (uint64_t row_q = 0; row_q < N; row_q++) {
    partial_assignment.push_back(row_q);

    // Construct the assignment for this entire column
    adiar::assignment_file column_assignment;

    { // The assignment_writer has to be detached, before we call any bdd
      // functions. It is automatically detached upon destruction, hence we have
      // it in this little scope.
      adiar::assignment_writer aw(column_assignment);

      for (uint64_t row = 0; row < N; row++) {
        aw << adiar::create_assignment(label_of_position(N, row, column), row == row_q);
      }
    }

    adiar::bdd restricted_constraints = adiar::bdd_restrict(constraints, column_assignment);

    if (adiar::bdd_pathcount(restricted_constraints) == 1) {
      solutions += 1;

      // Obtain the lexicographically minimal true assignment. Well, only one
      // exists, so we get the only one left.
      adiar::assignment_file forced_assignment = adiar::bdd_satmin(restricted_constraints);

      // Sort the variables back in order of the columns, rather than rows.
      struct sort_by_column
      {
      private:
        uint64_t N;

      public:
        explicit sort_by_column(const uint64_t N) : N(N) { }

        bool operator()(const adiar::assignment &a, const adiar::assignment &b)
        {
          return j_of_label(N, a.label) < j_of_label(N, b.label);
        }
      };

      adiar::sort(forced_assignment, sort_by_column(N));

      // Copy the rest of the assignment into 'partial_assignment'
      adiar::assignment_stream<> fas(forced_assignment);
      while (fas.can_pull()) {
        adiar::assignment a = fas.pull();
        if (a.value) {
          partial_assignment.push_back(i_of_label(N, a.label));
        }
      }

      n_queens_print_solution(partial_assignment);

      for (uint64_t c = N-1; c > column; c--) {
        partial_assignment.pop_back();
      }
    } else if (adiar::is_sink(restricted_constraints, adiar::is_true)) {
      n_queens_print_solution(partial_assignment);
      solutions += 1;
    } else {
      solutions += n_queens_list(N, column+1, partial_assignment, restricted_constraints);
    }
    partial_assignment.pop_back();
  }

  return solutions;
}

uint64_t n_queens_list(uint64_t N, const adiar::bdd& board)
{
  tpie::log_info() << "|  | solutions:" << std::endl;

  if (N == 1) {
    /* To make the recursive function work for N = 1 we would have to have the
     * adiar::count_paths check above at the beginning. That would in all other
     * cases merely result in an unecessary counting of paths at the very
     * start. */
    std::vector<uint64_t> assignment { 0 };
    n_queens_print_solution(assignment);

    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  return n_queens_list(N, 0, partial_assignment, board);
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

  // ===== ADIAR =====
  // Initialize
  adiar::adiar_init(M);
  tpie::log_info() << "| Initialized Adiar with " << M << " MB of memory"  << std::endl << "|" << std::endl;

  // ===== N Queens =====

  tpie::log_info() << "| " << N << "-Queens : Board construction"  << std::endl;
  auto before_board = get_timestamp();
  adiar::bdd board = n_queens_B(N);
  auto after_board = get_timestamp();

  tpie::log_info() << "|  | time: " << duration_of(before_board, after_board) << " s" << std::endl;
  tpie::log_info() << "|  | largest BDD  : " << largest_nodes << " nodes" << std::endl;
  tpie::log_info() << "|  | final size: " << bdd_nodecount(board) << " nodes"<< std::endl;

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
    tpie::log_info() << "|  | time: " << duration_of(before_list, after_list) << " s" << std::endl;
  }

  // ===== ADIAR =====
  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  exit(correct_result ? 0 : 1);
}
