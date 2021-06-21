#include "examples_common.cpp"

#include <optional>

/*******************************************************************************
 * We'd find it interesting to output the size of the largest ZDD and the final
 * ZDD.
 */
size_t largest_nodes = 1;

/*******************************************************************************
 *                             Variable ordering
 *
 *                          (N^2 * t) + (N * i) + j.
 */
inline adiar::label_t label_of_position(int N, int i, int j, int t)
{
  return (N * N * t) + (N * i) + j;
}

// TODO: There must be a better way then enumerating all 8 moves?
//       Yet, these bases cases are really small...
const int row_moves[8]    = { -2, -2, -1, -1,  1,  1,  2,  2 };
const int column_moves[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

bool is_move(int N, int i_from, int j_from, int i_to, int j_to)
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

/******************************************************************************/
void constraint_dont_care(adiar::node_writer &out_writer, adiar::ptr_t &curr_root,
                          int N, int max_t, int min_t)
{
  for (int curr_t = max_t; curr_t >= min_t; curr_t--) {
    for (int i = N - 1; i >= 0; i--) {
      for (int j = N - 1; j >= 0; j--) {
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
 * Constraint: transition system
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
    constraint_dont_care(out_writer, curr_root, N, N*N-1, t+2);

    // Node chains at time-step t+1 where exactly one possible square (i',j') is
    // visited for every possible one (i,j) at time step t.
    for (int i_t1 = N-1; i_t1 >= 0; i_t1--) {
      for (int j_t1 = N-1; j_t1 >= 0; j_t1--) {
        adiar::label_t next_pos = label_of_position(N, i_t1, j_t1, t+1);

        // We encode the (i,j) inside of the node id to make linking easy.
        for (int i_t = N-1; i_t >= 0; i_t--) {
          for (int j_t = N-1; j_t >= 0; j_t--) {

            // Is (i',j') reachable from (i,j)?
            if (is_move(N, i_t, j_t, i_t1, j_t1)) {
              adiar::label_t curr_pos = label_of_position(N, i_t, j_t, t);
              adiar::ptr_t low = ptr_to_next(N, i_t, j_t, i_t1, j_t1, t);

              adiar::node_t out_node = adiar::create_node(next_pos, curr_pos, low, curr_root);
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

    // "Don't care" Previous time steps
    constraint_dont_care(out_writer, curr_root, N, t-1, 0);
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}


/*******************************************************************************
 * Constraint: Visit every square exactly once
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
uint64_t expected_paths[9] = {
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

int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  uint64_t N = 0;
  size_t M = 1;

  try {
    if (argc == 1) {
      std::cout << "Missing argument for N and M" << std::endl;
    } else {
      N = std::stoi(argv[1]);
      if (N == 0 || N > 27) {
        std::cout << "N should be in interval [1;27]: " << argv[1] << std::endl;
        N = 0;
      }
      if (argc == 3) {
        M = std::stoi(argv[2]);
        if (M <= 0) {
          std::cout << "M should at least be 1: " << argv[2] << std::endl;
        }
      }
    }
  } catch (std::invalid_argument const &ex) {
    std::cout << "Invalid number: " << argv[1] << std::endl;
  } catch (std::out_of_range const &ex) {
    std::cout << "Number out of range: " << argv[1] << std::endl;
  }

  if (N == 0 || M <= 0) {
    exit(1);
  }

  // ===== ADIAR =====
  // Initialize
  adiar::adiar_init(M);
  std::cout << "| Initialized Adiar with " << M << " MiB of memory"  << std::endl
            << "|" << std::endl;

  // ===== Knight's Tour =====

  std::cout << "| Knight's Tour (" << N << "x" << N << ")" << std::endl
            << "|" << std::endl;;

  adiar::zdd paths;
  std::cout << "| Time:" << std::endl;

  if (N == 1) {
    paths = adiar::zdd_ithvar(0);
  } else {
    auto before_paths = get_timestamp();
    paths = constraint_transition(N,0);
    for (int t = 1; t < N*N-1; t++) {
      paths &= constraint_transition(N,t);
      largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
    }
    auto after_paths = get_timestamp();

    std::cout << "|  | Transition intersection: " << std::endl
              << "|  |  " << duration_of(before_paths, after_paths) << " s" << std::endl;

    auto before_only_once = get_timestamp();
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        paths &= constraint_exactly_once(N,i,j);
        largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
      }
    }
    auto after_only_once = get_timestamp();

    std::cout << "|  | Exactly once per constraint: " << std::endl
              << "|  |  " << duration_of(before_only_once, after_only_once) << " s" << std::endl;
  }

  auto before_count = get_timestamp();
  uint64_t solutions = zdd_size(paths);
  auto after_count = get_timestamp();

  std::cout << "|  | Counting elements: " << std::endl
            << "|  |  " << duration_of(before_count, after_count) << " s" << std::endl
            << "|" << std::endl
            << "| Solutions: " << solutions << std::endl
            << "|" << std::endl
            << "| Memory use:" << std::endl
            << "|  | Largest ZDD: " << largest_nodes << " nodes" << std::endl;

  bool correct_result = solutions == expected_paths[N];

  // ===== ADIAR =====
  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  exit(correct_result ? 0 : 1);
}
