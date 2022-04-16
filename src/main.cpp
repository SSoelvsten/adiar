// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

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
    bdd x1 = bdd_ithvar(1);
    bdd x2 = bdd_ithvar(2);
    bdd x3 = bdd_ithvar(3);
    bdd intnode = bdd_or(x2, x1);
    bdd root = bdd_and(x3, intnode);

    uint64_t solutions = bdd_satcount(root);
    std::cout << "number of solutions: " << solutions << std::endl;

    std::vector<label_t> order = bdd_order(root);
    std::cout << "order: " << std::endl;
    for (auto l : order)
    {
      std::cout << "  " << l << std::endl;
    }
        
    bdd redordered = bdd_reorder(root);

    solutions = bdd_satcount(redordered);
    std::cout << "number of solutions after reordering: " << solutions << std::endl;

    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
