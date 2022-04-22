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
    bdd x1 = bdd_ithvar(4);
    bdd x2 = bdd_ithvar(2);
    bdd x3 = bdd_ithvar(3);
    bdd intnode = bdd_and(x1, x2);
    __bdd root = bdd_or(intnode, x3);
    // get arc file: root.get<arc_file>().. Vigtigt at det er en __bdd, og vi includer adiar/internal/dot.h
    //std::cout << min_label(root) << std::endl;
    auto root_arc_file = root.get<arc_file>();
    output_dot(root.get<arc_file>(), "root.dot");

    arc_t a;
    {
      adiar::node_arc_stream<false> fs(root_arc_file);
      a = fs.pull();
    }
    auto rev_path = reverse_path(root_arc_file, a.target);

    if (rev_path.empty()) {
      std::cout << "No path found" << std::endl;
    }

    for (auto a : rev_path)
    {
      std::cout << a.label << " = " << a.value << std::endl;
    }
    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
