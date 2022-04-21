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
    bdd x1 = bdd_ithvar(4);
    bdd x2 = bdd_ithvar(2);
    bdd x3 = bdd_ithvar(3);
    bdd intnode = bdd_and(x1, x2);
    bdd root = bdd_or(intnode, x3);

    std::cout << min_label(root) << std::endl;

    output_dot(root, "root.dot");

    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
