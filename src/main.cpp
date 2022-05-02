// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>
#include <adiar/internal/dot.h>

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
    bdd x2 = bdd_ithvar(2);
    bdd x0 = bdd_ithvar(0);
    bdd x1 = bdd_ithvar(1);
    bdd intnode = bdd_and(x2, x0);
    bdd org = bdd_or(intnode, x1);
    
    std::vector<label_t> permutation = {1,2,0};
    bdd new_order = bdd_reorder(org, permutation);
    std::vector<label_t> permutation_inverse = {2,0,1};
    bdd org_back = bdd_reorder(new_order, permutation_inverse);

    output_dot(org, "org.dot");
    output_dot(new_order, "new.dot", permutation);
    output_dot(org_back, "org_back.dot");
    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
