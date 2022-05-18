// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>
#include <adiar/internal/dot.h>

// TIME Imports
#include <chrono>

// Queens
#include <queens.h>

#define EXP_BDD 0
#define QUEENS_BDD 1
#define OUR_BDD 0

using namespace adiar;

int main(int argc, char *argv[])
{
  size_t M = 1024;

  try
  {
    if (argc > 1)
    {
      M = std::stoi(argv[1]);
    }
  }
  catch (std::invalid_argument const &ex)
  {
    std::cout << "Invalid number: " << argv[1] << std::endl;
  }
  catch (std::out_of_range const &ex)
  {
    std::cout << "Number out of range: " << argv[1] << std::endl;
  }

  adiar_init(M * 1024 * 1024);

  {
    // ===== Your code starts here =====
#if EXP_BDD
    bdd root = bdd_and(bdd_ithvar(0), bdd_ithvar(1));
    int num_of_vars = 28;
    for (int i = 2; i < num_of_vars; i += 2)
    {
      root = bdd_or(root, bdd_and(bdd_ithvar(i), bdd_ithvar(i + 1)));
    }

    std::vector<label_t> permutation;
    for (int i = 0; i < num_of_vars; i += 2)
    {
      permutation.push_back(i);
    }
    for (int i = 1; i < num_of_vars; i += 2)
    {
      permutation.push_back(i);
    }

    std::vector<label_t> permutation_inverse = std::vector<label_t>(permutation.size(), 0);
    for (unsigned long i = 0; i < permutation.size(); i++)
    {
      permutation_inverse[permutation[i]] = i;
    }
#endif

#if OUR_BDD
    bdd x0 = bdd_ithvar(0);
    bdd x1 = bdd_ithvar(1);
    bdd x2 = bdd_ithvar(2);
    bdd intnode = bdd_and(x0, x2);
    bdd root = bdd_or(intnode, x1);

    std::vector<label_t> permutation = {2, 0, 1};
    std::vector<label_t> permutation_inverse = {1, 2, 0};
#endif

#if QUEENS_BDD
    N = 8;
    
    std::vector<label_t> permutation;
    int dir = 1;

    auto walk = [&](int dir, int dist, int start) {
      for (int j = 1; j <= dist; j++) {
        if (dir % 4 == 0) {
          // right
          permutation.push_back(start + j);
        } else if (dir % 4 == 1) {
          // down
          permutation.push_back(start + j * N);
        } else if (dir % 4 == 2) {
          // left
          permutation.push_back(start - j);
        } else if (dir % 4 == 3) {
          // up
          permutation.push_back(start - j * N);
        }
      }
    };

    for (int i = N; i > 0; i--)
    {
      if (i == N)
      {
          for (int j = 0; j < N; j++)
            permutation.push_back(j);
      }
      else
      {
        for (int j = 0; j < 2; j++)
        {
          walk(dir, i, permutation.back());
          dir++;
        }
      }
    }
    
    std::reverse(permutation.begin(), permutation.end());

    std::vector<label_t> permutation_inverse = std::vector<label_t>(permutation.size(), 0);
    for (unsigned long i = 0; i < permutation.size(); i++)
    {
      permutation_inverse[permutation[i]] = i;
    }

    std::cout << "Permutation" << std::endl;
    for (auto e : permutation)
    {
      std::cout << e << " ";
    }
    std::cout << std::endl;

    bdd root = n_queens_B();
    std::cout << "Created board" << std::endl;

#endif
    
    std::chrono::steady_clock::time_point begin_new = std::chrono::steady_clock::now();
    bdd new_order = bdd_reorder(root, permutation);
    std::chrono::steady_clock::time_point end_new = std::chrono::steady_clock::now();
    std::cout << "Time elapsed reordering = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_new - begin_new).count() << "[ms]" << std::endl;

    adiar_printstat();
    adiar_statsreset();

    std::chrono::steady_clock::time_point begin_back = std::chrono::steady_clock::now();
    bdd org_back = bdd_reorder(new_order, permutation_inverse);
    std::chrono::steady_clock::time_point end_back = std::chrono::steady_clock::now();

    std::cout << "Time elapsed reordering back = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_back - begin_back).count() << "[ms]" << std::endl;
    adiar_printstat();
    
    output_dot(root, "orginal_order.dot");
    output_dot(new_order, "new_order.dot", permutation);
    output_dot(org_back, "orginal_order_back.dot");

    std::cout << "Input node count: " << bdd_nodecount(root) << " Reordered node count: " << bdd_nodecount(new_order) << " original_back node count: " << bdd_nodecount(org_back) << std::endl;
    std::cout << "Input sat count: " << bdd_satcount(root) << " Reorder sat count: " << bdd_satcount(new_order) << std::endl;

    // =====  Your code ends here  =====
  }

  adiar_deinit();
  exit(0);
}
