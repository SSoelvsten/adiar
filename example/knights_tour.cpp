#include "examples_common.cpp"

#include <optional>

/*******************************************************************************
 * This entire example is based off of the encoding by Randal Bryant for the
 * same problem that you can find here:
 *
 * https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py
 */

/*******************************************************************************
 * We'd find it interesting to output the size of the largest ZDD and the final
 * ZDD.
 */
size_t largest_nodes = 1;

/*******************************************************************************
 *                             Variable ordering
 *
 *                          (N^2 * t) + (N * i) + j.
 *
 * That is, we encode the entire information about position and time-step into
 * the very variable label. This ordering is first by row, second by column, and
 * finally by time. Most importantly, this means that any "snapshot" of the
 * board at a specific time-step are grouped together.
 */
inline adiar::label_t label_of_position(int N, int i, int j, int t)
{
  return (N * N * t) + (N * i) + j;
}

/*******************************************************************************
 *                            Closed Tour Constraint
 *
 * To only look at the closed tours, i.e. hamiltonian cycles and not hamiltonian
 * paths, we merely fix it to start in the top-left corner. Since we are
 * counting unoriented paths, then we can fix one of the two possible next
 * squares to be the one used at t = 1.
 *
 * The following array includes these very squares. The predicate below tests
 * whether the argument is in one of the three squares.
 */
const int closed_squares [3][2] = {{0,0}, {1,2}, {2,1}};

bool is_closed_square(int i, int j)
{
  return (i == closed_squares[0][0] && j == closed_squares[0][1])
      || (i == closed_squares[1][0] && j == closed_squares[1][1])
      || (i == closed_squares[2][0] && j == closed_squares[2][1]);
}

/*******************************************************************************
 * Creates a long "don't care" chain for all the positions and times, that we do
 * not care about. If we are working with the closed tour, then we can safely
 * skip these to make the intermediate ZDDs smaller.
 */
template<bool filter_closed_squares = false>
void constraint_dont_care(adiar::node_writer &out_writer, adiar::ptr_t &curr_root,
                          int N, int max_t, int min_t)
{
  for (int curr_t = max_t; curr_t >= min_t; curr_t--) {
    for (int i = N - 1; i >= 0; i--) {
      for (int j = N - 1; j >= 0; j--) {
        if (filter_closed_squares && is_closed_square(i,j)) {
          continue;
        }

        adiar::node_t out_node = adiar::create_node(label_of_position(N,i,j,curr_t),
                                                    0,
                                                    curr_root,
                                                    curr_root);

        out_writer << out_node;
        curr_root = out_node.uid;
      }
    }
  }
}

/*******************************************************************************
 * Combining the above, we can construct the ZDD that fixes the initial
 * position, the second position, and the final position.
 */
adiar::zdd constraint_closed(uint64_t N)
{
  adiar::node_file out;

  { adiar::node_writer out_writer(out);

    // Set final time step to (2,1)
    adiar::node_t n_t_end = adiar::create_node(label_of_position(N,
                                                                 closed_squares[2][0],
                                                                 closed_squares[2][1],
                                                                 N*N-1),
                                               adiar::MAX_ID,
                                               adiar::create_sink_ptr(false),
                                               adiar::create_sink_ptr(true));
    out_writer << n_t_end;

    // Create don't care on all intermediate steps
    adiar::ptr_t curr_root = n_t_end.uid;
    constraint_dont_care<true>(out_writer, curr_root, N, N*N-2, 2);

    // Set t = 1 to (1,2)
    adiar::node_t n_t_1 = adiar::create_node(label_of_position(N,
                                                               closed_squares[1][0],
                                                               closed_squares[1][1],
                                                               1),
                                             adiar::MAX_ID,
                                             adiar::create_sink_ptr(false),
                                             curr_root);
    out_writer << n_t_1;

    // Set t = 0 to (0,0)
    adiar::node_t n_t_0 = adiar::create_node(label_of_position(N,
                                                               closed_squares[0][0],
                                                               closed_squares[0][1],
                                                               0),
                                             adiar::MAX_ID,
                                             adiar::create_sink_ptr(false),
                                             n_t_1.uid);
    out_writer << n_t_0;
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}


/*******************************************************************************
 *                      Move Logic for Transition Function
 *
 * We need a little bit of logic to compute the different parts of a move. This
 * is not the bottleneck, so we will do something somewhat easily comprehensible
 * rather than fast.
 *
 * The following two arrays encode (starting from 'north west' and going
 * clockwise) the i and j coordinates of all the moves.
 */
constexpr int row_moves[8]    = { -2, -2, -1, -1,  1,  1,  2,  2 };
constexpr int column_moves[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

bool is_move(int i_from, int j_from, int i_to, int j_to)
{
  for (int idx = 0; idx < 8; idx++) {
    if (i_from + row_moves[idx] == i_to &&
        j_from + column_moves[idx] == j_to) {
      return true;
    }
  }
  return false;
}

adiar::ptr_t ptr_to_first(int N, int i_from, int j_from, int t)
{
  for (int idx = 0; idx < 8; idx++) {
    int i_to = i_from + row_moves[idx];
    int j_to = j_from + column_moves[idx];

    if (i_to < 0 || N <= i_to || j_to < 0 || N <= j_to) { continue; }
    adiar::label_t from_label = label_of_position(N, i_from, j_from, t);
    adiar::label_t to_label = label_of_position(N, i_to, j_to, t+1);

    return adiar::create_node_ptr(to_label, from_label);
  }
  return adiar::create_sink_ptr(false);
}

/* When we have the latest 'legal' move we then also want to obtain the next
   'legal' move, or 'false' if there is none.
 */
adiar::ptr_t ptr_to_next(int N, int i_from, int j_from, int i_to, int j_to, int t)
{
  bool seen_move = false;

  for (int idx = 0; idx < 8; idx++) {
    int i = i_from + row_moves[idx];
    int j = j_from + column_moves[idx];

    if (i < 0 || N <= i || j < 0 || N <= j) { continue; }

    if (seen_move) {
      adiar::label_t from_label = label_of_position(N, i_from, j_from, t);
      adiar::label_t to_label = label_of_position(N, i, j, t+1);

      return adiar::create_node_ptr(to_label, from_label);
    }
    seen_move = i == i_to && j == j_to;
  }
  return adiar::create_sink_ptr(false);
}

/*******************************************************************************
 *                            Transition System
 */
adiar::zdd constraint_transition(int N, int t)
{
  adiar::node_file out;

  { // When calling `out.size()` below, we have to make it read-only. So, we
    // have to detach the node_writer before we do. This is automatically done
    // on garbage collection, which is why we add an inner scope.
    adiar::node_writer out_writer(out);

    adiar::ptr_t curr_root = adiar::create_sink_ptr(true);

    // "Don't care" for future time steps
    constraint_dont_care<>(out_writer, curr_root, N, N*N-1, t+2);

    // Node chains at time-step t+1 where exactly one possible square (i',j') is
    // visited for every possible one (i,j) at time step t.
    for (int i_t1 = N-1; i_t1 >= 0; i_t1--) {
      for (int j_t1 = N-1; j_t1 >= 0; j_t1--) {
        adiar::label_t next_pos = label_of_position(N, i_t1, j_t1, t+1);

        // We encode the (i,j) inside of the node id to make linking easy.
        for (int i_t = N-1; i_t >= 0; i_t--) {
          for (int j_t = N-1; j_t >= 0; j_t--) {

            // Is (i',j') reachable from (i,j)?
            if (is_move(i_t, j_t, i_t1, j_t1)) {
              adiar::label_t curr_pos = label_of_position(N, i_t, j_t, t);
              adiar::ptr_t low = ptr_to_next(N, i_t, j_t, i_t1, j_t1, t);

              adiar::node_t out_node = adiar::create_node(next_pos,
                                                          curr_pos,
                                                          low,
                                                          curr_root);
              out_writer << out_node;
            }
          }
        }
      }
    }

    // Node chain at time step t
    curr_root = adiar::create_sink_ptr(false);

    for (int i = N-1; i >= 0; i--) {
      for (int j = N-1; j >= 0; j--) {
        adiar::label_t out_label = label_of_position(N, i, j, t);

        adiar::ptr_t high = ptr_to_first(N, i, j, t);

        adiar::node_t out_node = adiar::create_node(out_label, 0, curr_root, high);

        out_writer << out_node;
        curr_root = out_node.uid;
      }
    }

    // "Don't care" for previous time steps
    constraint_dont_care<>(out_writer, curr_root, N, t-1, 0);
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}


/*******************************************************************************
 *                           Hamiltonian Constraint
 *
 * Finally, for each position on the board, we want to add the constraint, that
 * you can only be there once.
 *
 * This essentially is an automaton with two states 'not seen' and 'seen'. If
 * the position is 'seen' as taken then one goes to a second chain of nodes,
 * where it may not be seen again.
 */
adiar::zdd constraint_exactly_once(uint64_t N, uint64_t i, uint64_t j)
{
  adiar::node_file out;

  { adiar::node_writer out_writer(out);

    adiar::ptr_t next0 = adiar::create_sink_ptr(false);
    adiar::ptr_t next1 = adiar::create_sink_ptr(true);

    for (int curr_t = N*N-1; curr_t >= 0; curr_t--) {
      for (int curr_i = N-1; curr_i >= 0; curr_i--) {
        for (int curr_j = N-1; curr_j >= 0; curr_j--) {
          adiar::label_t out_label = label_of_position(N, curr_i, curr_j, curr_t);

          bool is_ij = i == curr_i && j == curr_j;

          // If we are talking about (i,j) and we already have counted it once, then
          // it is forced to zero. In ZDDs this is represented by skipping it.
          if (!is_ij && (curr_t > 0 || curr_i > i)) {
            adiar::node_t out_n1 = adiar::create_node(out_label, 1, next1, next1);
            out_writer << out_n1;
            next1 = out_n1.uid;
          }

          adiar::node_t out_n0 = adiar::create_node(out_label, 0,
                                                    next0,
                                                    is_ij ? next1 : next0);
          out_writer << out_n0;
          next0 = out_n0.uid;
        }
      }
    }
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}

// expected number of directed tours taken from:
//  https://en.wikipedia.org/wiki/Knight%27s_tour#Number_of_tours
uint64_t expected_all[9] = {
  0,
  1,
  0,
  0,
  0,
  1728,
  6637920,
  165575218320,
  19591828170979904
};

uint64_t expected_closed[10] = {
  0,
  1,
  0,
  0,
  0,
  0,
  9862,
  0,
  13267364410532,
  0
};

int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  bool only_closed = false;

  {
    int c;
    bool should_error_exit = init_cl_arguments(argc, argv);

    while ((c = getopt(argc, argv, "c")) != -1) {
      switch(c) {
      case 'c':
        // HACK: 'constraint_closed' only works for 3x3 boards and bigger.
        only_closed = 3u <= N;
        continue;

      case 'h':
        std::cout << "        -c       [ ]          Enumerate only closed tours (rather open and closed)" << std::endl;
        exit(0);
      }
    }

    if (!should_error_exit && N == 0) {
      std::cout << "  Must specify positive number for N" << std::endl;
      should_error_exit = true;
    }

    if (should_error_exit) { exit(1); }
  }

  // ===== ADIAR =====
  // Initialize
  adiar::adiar_init(M * 1024 * 1024);
  std::cout << "| Initialized Adiar with " << M << " MiB of memory"  << std::endl
            << "|" << std::endl;

  // ===== Knight's Tour =====

  std::cout << "| " << N << "x" << N << " Knight's Tour "
            << "(" << (only_closed ? "Closed" : "Open and Closed") << ")" << std::endl
            << "|" << std::endl;;

  adiar::zdd paths;
  std::cout << "| Time:" << std::endl;

  if (N == 1) {
    paths = adiar::zdd_ithvar(0);
  } else {
    timestamp_t before_paths = get_timestamp();
    paths = only_closed ? constraint_closed(N) : constraint_transition(N,0);
    for (int t = 1; t < N*N-1; t++) {
      paths &= constraint_transition(N,t);
      largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
    }
    timestamp_t after_paths = get_timestamp();

    std::cout << "|  | Transition intersection: " << std::endl
              << "|  |  " << duration_of(before_paths, after_paths) << " s" << std::endl;

    timestamp_t before_only_once = get_timestamp();
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (only_closed && is_closed_square(i,j)) { continue; }

        paths &= constraint_exactly_once(N,i,j);
        largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
      }
    }
    timestamp_t after_only_once = get_timestamp();

    std::cout << "|  | Exactly once per constraint: " << std::endl
              << "|  |  " << duration_of(before_only_once, after_only_once) << " s" << std::endl;
  }

  timestamp_t before_count = get_timestamp();
  uint64_t solutions = zdd_size(paths);
  timestamp_t after_count = get_timestamp();

  std::cout << "|  | Counting elements: " << std::endl
            << "|  |  " << duration_of(before_count, after_count) << " s" << std::endl
            << "|" << std::endl
            << "| Solutions: " << solutions << std::endl
            << "|" << std::endl
            << "| Memory use:" << std::endl
            << "|  | Largest ZDD: " << largest_nodes << " nodes" << std::endl
            << "|  | Final ZDD: " << zdd_nodecount(paths) << " nodes" << std::endl;

  // ===== ADIAR =====
  // Print statistics, if compiled with those flags.
  adiar::adiar_printstat();

  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  bool correct_result = only_closed
    ? (N < 10 ? solutions == expected_closed[N] : true)
    : (N < 9 ? solutions == expected_all[N] : true);

  exit(correct_result ? 0 : 1);
}
