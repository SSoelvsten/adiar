// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>
#include <adiar/internal/dot.h>

#define EXP_BDD 1
#define OUR_BDD 0
#define EXP_BDD_SML 0

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
    bdd x0 = bdd_ithvar(0);
    bdd x1 = bdd_ithvar(1);
    bdd x2 = bdd_ithvar(2);
    bdd x3 = bdd_ithvar(3);
    bdd x4 = bdd_ithvar(4);
    bdd x5 = bdd_ithvar(5);
    bdd x6 = bdd_ithvar(6);
    bdd x7 = bdd_ithvar(7);
    bdd x0Ax1 = bdd_and(x0, x1);
    bdd x2Ax3 = bdd_and(x2, x3);
    bdd x4Ax5 = bdd_and(x4, x5);
    bdd x6Ax7 = bdd_and(x6, x7);
    bdd int1 = bdd_or(x0Ax1, x2Ax3);
    bdd int2 = bdd_or(x4Ax5, x6Ax7);
    bdd root = bdd_or(int1, int2);

    std::vector<label_t> permutation = {0, 2, 4, 6, 1, 3, 5, 7};
    bdd new_order = bdd_reorder(root, permutation);
    std::vector<label_t> permutation_inverse = {0, 4, 1, 5, 2, 6, 3, 7};
    bdd org_back = bdd_reorder(new_order, permutation_inverse);
#endif

#if EXP_BDD_SML
    bdd x0 = bdd_ithvar(0);
    bdd x1 = bdd_ithvar(1);
    bdd x2 = bdd_ithvar(2);
    bdd x3 = bdd_ithvar(3);
    bdd x0Ax1 = bdd_and(x0, x1);
    bdd x2Ax3 = bdd_and(x2, x3);
    bdd root = bdd_or(x0Ax1, x2Ax3);

    std::vector<label_t> permutation = {0, 2, 1, 3};
    bdd new_order = bdd_reorder(root, permutation);
    std::vector<label_t> permutation_inverse = {0, 2, 1, 3};
    bdd org_back = bdd_reorder(new_order, permutation_inverse);
#endif

#if OUR_BDD
    bdd x0 = bdd_ithvar(0);
    bdd x1 = bdd_ithvar(1);
    bdd x2 = bdd_ithvar(2);
    bdd intnode = bdd_and(x0, x2);
    bdd root = bdd_or(intnode, x1);

    std::vector<label_t> permutation = {2, 0, 1};
    bdd new_order = bdd_reorder(root, permutation);
    std::vector<label_t> permutation_inverse = {1, 2, 0};
    bdd org_back = bdd_reorder(new_order, permutation_inverse);
#endif

    output_dot(root, "orginal_order.dot");
    output_dot(new_order, "new_order.dot", permutation);
    output_dot(org_back, "orginal_order_back.dot");
    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
