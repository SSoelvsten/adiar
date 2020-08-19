#include <vector>

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

/* We represent the position (i,j) on the N x N board board as N*i + j. */
inline uint64_t label_of_position(uint64_t N, uint64_t i, uint64_t j)
{
  return (N * i) + j;
}

inline uint64_t i_of_label(uint64_t N, uint64_t label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, uint64_t label)
{
  return label % N;
}

/* Given N we then know the last label  */
inline uint64_t calc_max_label(uint64_t N)
{
  return label_of_position(N, N-1, N-1);
}

/*                 N - QUEENS : at-most-one (AMO) base case
 *
 * A single queen placed on (i,j) creates one single set of other illegal
 * positions. This represents is a check whether there is a conflict for this
 * queen. If the queen is not placed at (i,j) by x_ij being false, then it is
 * trivially true. What we construct in this case is x_ij ==> !conflicts(x_ij),
 * where conflicts is an OBDD that is an OR of all variables x_kl that are on
 * the same diagonals, row or column.
 *        __________
 *       | \ | /    |
 *       |  \|/     |
 *       |---X------|
 *       |  /|\     |
 *       |_/_|_\____|
 *
 * This we can construct pretty simply with the builders of coom to create the
 * OBDD for x_ij (build_x) and the conflicts (build_or) to then negate the
 * conflicts (negate) and finally create the final OBDD (apply). We could also
 * do without all of this, since the resulting OBDD is so well structured.
 *
 * This reduces the work we have to do down to only O(N) time and O(N/B) I/Os
 * rather than O(sort(N)) in both time and I/Os. One pretty much cannot do this
 * base case faster.
 */
void n_queens_amo(uint64_t N,
                  uint64_t i, uint64_t j,
                  tpie::file_stream<coom::node>& out_nodes)
{
  if (N == 1) {
    out_nodes.write(coom::create_sink_node(true));
  }

  uint64_t row = N - 1;

  uint64_t low = coom::create_sink(true);
  uint64_t high = coom::create_sink(false);

  do {
    uint64_t row_diff = std::max(row,i) - std::min(row,i);

    if (row_diff == 0) {
      // On row of the queen in question
      uint64_t column = N - 1;
      do {
        uint64_t label = label_of_position(N, row, column);

        // If (row, column) == (i,j), then the chain goes through high.
        if (column == j) {
          // Node to check if the queen in question actually isn't placed anyway
          coom::node no_queen_placed = coom::create_node(label_of_position(N,i,j), 1,
                                                         coom::create_sink(true),
                                                         coom::create_sink(false));

          if (label != 0) {
            out_nodes.write(no_queen_placed);
            high = no_queen_placed.node_ptr;
          }

          // Node to check whether the chain has to be further followed
          coom::node queen_placed = coom::create_node(label_of_position(N,i,j), 0,
                                                      coom::create_sink(true),
                                                      low);

          if (low != coom::create_sink(true)) {
            // This is the end of the chain
            out_nodes.write(queen_placed);
            low = queen_placed.node_ptr;
          }
          continue;
        }

        coom::node out_node = coom::create_node(label, 0, low, high);

        out_nodes.write(out_node);
        low = out_node.node_ptr;
      } while (column-- > 0);
    } else {
      // On another row
      if (j + row_diff < N) {
        // Diagonal to the right is within bounds
        auto label = label_of_position(N, row, j + row_diff);
        auto out_node = coom::create_node(label, 0, low, high);

        out_nodes.write(out_node);
        low = out_node.node_ptr;
      }

      // Column
      uint64_t label = label_of_position(N, row, j);
      coom::node out_node = coom::create_node(label, 0, low, high);

      out_nodes.write(out_node);
      low = out_node.node_ptr;

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        uint64_t label = label_of_position(N, row, j - row_diff);
        coom::node out_node = coom::create_node(label, 0, low, high);

        out_nodes.write(out_node);
        low = out_node.node_ptr;
      }
    }
  } while (row-- > 0);
}

/*                  N - QUEENS : at-most-one (AMO) recursion
 *
 * We can now split the board into four quadrants and then AND the conflicts together.
 *           _______                          _______
 *          |   |   |                        |       |
 *          |---|---|   == APPLY(AND) ==>    |       |
 *          |___|___|                        |_______|
 */
void n_queens_amo(uint64_t N,
                  uint64_t row_start, uint64_t row_end,        // inclusive
                  uint64_t column_start, uint64_t column_end,  // exclusive
                  tpie::file_stream<coom::node>& out_nodes)
{
  // Recursed to a single level?
  if (row_start + 1 == row_end && column_start + 1 == column_end) {
    n_queens_amo(N, row_start, column_start, out_nodes);
    return;
  }

  auto row_split = row_end - (row_end - row_start) / 2;
  auto column_split = column_end - (column_end - column_start) / 2;

  if (row_start + 1 == row_end) {
    // Recurse by only splitting on the column
    tpie::file_stream<coom::node> out_left;
    out_left.open();

    n_queens_amo(N, row_start, row_end, column_start, column_split, out_left);

    tpie::file_stream<coom::node> out_right;
    out_right.open();

    n_queens_amo(N, row_start, row_end, column_split, column_end, out_right);

    coom::apply(out_left, out_right, coom::and_op, out_nodes);
  } else if (column_start + 1 == column_end) {
    // Recurse by only splitting on the row
    tpie::file_stream<coom::node> out_top;
    out_top.open();

    n_queens_amo(N, row_start, row_split, column_start, column_end, out_top);

    tpie::file_stream<coom::node> out_bottom;
    out_bottom.open();

    n_queens_amo(N, row_split, row_end, column_start, column_end, out_bottom);

    coom::apply(out_top, out_bottom, coom::and_op, out_nodes);
  } else {
    // Recurse into all four quadrants
    tpie::file_stream<coom::node> out_left;
    tpie::file_stream<coom::node> out_right;

    // Top two quadrants
    out_left.open();
    out_right.open();

    n_queens_amo(N, row_start, row_split, column_start, column_split, out_left);
    n_queens_amo(N, row_start, row_split, column_split, column_end, out_right);

    tpie::file_stream<coom::node> out_top;
    out_top.open();

    coom::apply(out_left, out_right, coom::and_op, out_top);
    out_left.close();
    out_right.close();

    // Bottom two quadrants
    out_left.open();
    out_right.open();

    n_queens_amo(N, row_split, row_end, column_start, column_split, out_left);
    n_queens_amo(N, row_split, row_end, column_split, column_end, out_right);

    tpie::file_stream<coom::node> out_bottom;
    out_bottom.open();

    coom::apply(out_left, out_right, coom::and_op, out_bottom);
    out_left.close();
    out_right.close();

    // Combine top and bottom half
    coom::apply(out_top, out_bottom, coom::and_op, out_nodes);
  }
}

/*                  N - QUEENS : at-least-one (ALO) on rows
 *
 * The at-least-one for each row is, just like the AMO base case, very
 * structured. Due to our variable ordering, each row is an N'th of a chain,
 * that may on a high arc go the next N'th part of the chain or if one reaches
 * the end of the subchain without anyone satisfied, then one can reject.
 */
void n_queens_alo_row(uint64_t N, tpie::file_stream<coom::node>& out_nodes)
{
  uint64_t row = N-1;

  uint64_t high = coom::create_sink(true);

  do {
    uint64_t low = coom::create_sink(false);
    uint64_t col = N-1;

    do {
      coom::node next_node = coom::create_node(label_of_position(N, row, col), 0, low, high);
      low = next_node.node_ptr;

      out_nodes.write(next_node);
      if (col == 0) {
        high = next_node.node_ptr;
      }
    } while (col-- > 0);
  } while (row-- > 0);
}

/*                  N - QUEENS : at-least-one (ALO) on columns
 *
 * The variable ordering does not allow us to do the at-least-one constraint for
 * columns as easily as above. The resulting OBDD would have most nodes encode
 * whether the other N-1 columns already have been satisfied. That would be
 * 2^(N-1) nodes for most variables. While variables on the first and last row
 * may shortcut quite a few cases, that is not enough to compensate for the
 * O(N^2) nodes for all the other O(N^2) variables. While we could construct
 * this in a complicated iterative procedure, the size of the OBDD does not seem
 * to make it worth much.
 *
 *           N - QUEENS : counting by number of satisfying assignments.
 *
 * Yet, we realise, that if one has to place at least one queen on every row,
 * but is not allowed to place more than one, then that implies that one has to
 * place the queen in another non-conflicting column. So, with merely the AMO
 * constraint and the ALO constraint for rows, we already force all satisfying
 * assignments to be solutions to the N-Queens problem.
 */
void n_queens_amo_alo(uint64_t N, tpie::file_stream<coom::node>& out_nodes)
{
  tpie::log_info() << "| " << N << "-Queens (AMO construction)"  << std::endl;
  tpie::file_stream<coom::node> amo;
  amo.open();

  n_queens_amo(N, 0, N, 0, N, amo);

  tpie::log_info() << "| " << N << "-Queens (ALO row construction)"  << std::endl;
  tpie::file_stream<coom::node> alo_row;
  alo_row.open();

  n_queens_alo_row(N, alo_row);

  tpie::log_info() << "| " << N << "-Queens (AMO-ALO construction)"  << std::endl;
  coom::apply(amo, alo_row, coom::and_op, out_nodes);
}

/* So after combining the AMO and ALO constraints from above, we merely count
 * the number of satisfying assignments.
 */
uint64_t n_queens_count(uint64_t N, tpie::file_stream<coom::node>& amo_alo)
{
  uint64_t solutions = coom::count_assignments(amo_alo, coom::is_true);

  tpie::log_info() << "| " << N << "-Queens (Counting assignments)"  << std::endl;
  tpie::log_info() << "|  | number of solutions: " << solutions << std::endl << std::endl;

  return solutions;
}

/*         N - QUEENS : pruning the search by partial assignment
 *
 * Based on: github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex
 *
 * If we want to list all the assignments, we have to do something else than
 * merely count the number of satisfying assignments. We could then iterate over
 * all possible assignments and use the OBDD to prune our search.
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
 * For a sanity check, we also count the number of solutions we list.
 */
void n_queens_print_solution(std::vector<uint64_t>& assignment)
{
  tpie::log_info() << "|  |  | ";
  for (uint64_t r : assignment) {
    tpie::log_info() << r << " ";
  }
  tpie::log_info() << std::endl;
}

uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       tpie::file_stream<coom::node>& constraints)
{
  if (coom::is_sink(constraints, coom::is_false)) {
    return 0;
  }
  uint64_t solutions = 0;

  tpie::file_stream<coom::assignment> column_assignment;
  tpie::file_stream<coom::node> restricted_constraints;

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

      for (int c = N-1; c > column; c--) {
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

uint64_t n_queens_list(uint64_t N, tpie::file_stream<coom::node>& amo_alo)
{
  tpie::log_info() << "| " << N << "-Queens (Pruning search)"  << std::endl;
  tpie::log_info() << "|  | solutions:" << std::endl;

  if (N == 1) {
    /* To make the recursive function work for N = 1 we would have to have the
     * coom::count_paths check above at the beginning. That would in all other
     * cases merely result in an unecessary counting of paths at the very
     * start. */
    std::vector<uint64_t> assignment { 0 };
    n_queens_print_solution(assignment);

    tpie::log_info() << "|  | number of solutions: " << 1 << std::endl << std::endl;
    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  uint64_t solutions = n_queens_list(N, 0, partial_assignment, amo_alo);

  tpie::log_info() << "|  | number of solutions: " << solutions << std::endl << std::endl;

  return solutions;
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

/* Finally, to run the above we need to initialize the underlying TPIE library
 * first, from which point all the file_streams, priority queues and sorters
 * used are at our disposal.
 */
int main(int argc, char* argv[])
{
  // ===== TPIE =====
  // Initialize
  tpie::tpie_init();

  size_t available_memory_mb = 2 * 1024;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // ===== Parse argument =====
  uint64_t N = 0;

  try {
    if (argc == 1) {
      tpie::log_info() << "Missing argument for upper bound on N" << std::endl;
    } else {
      N = std::stoi(argv[1]);
      if (N == 0 || N > 27) {
        tpie::log_info() << "N should be in interval [1;27]: " << argv[1] << std::endl;
        N = 0;
      }
    }
  } catch (std::invalid_argument const &ex) {
    tpie::log_info() << "Invalid number: " << argv[1] << std::endl;
  } catch (std::out_of_range const &ex) {
    tpie::log_info() << "Number out of range: " << argv[1] << std::endl;
  }

  if (N == 0) {
    tpie::tpie_finish();
    exit(1);
  }

  // ===== N Queens =====

  tpie::file_stream<coom::node> amo_alo;
  amo_alo.open();

  n_queens_amo_alo(N, amo_alo);

  bool correct_result = true;

  // Run counting example
  if (n_queens_count(N, amo_alo) != expected_result[N]) {
    correct_result = false;
  }

  // Run enumeration example (for reasonably small N)
  if (N <= 8 && n_queens_list(N, amo_alo) != expected_result[N]) {
    correct_result = false;
  }

  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(correct_result ? 0 : 1);
}
