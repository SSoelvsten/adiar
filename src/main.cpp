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
    bdd x1 = bdd_ithvar(2);
    bdd x2 = bdd_ithvar(0);
    bdd x3 = bdd_ithvar(1);
    bdd intnode = bdd_and(x1, x2);
    bdd org = bdd_or(intnode, x3);
    
    std::vector<label_t> permutation = {1,2,0};
    bdd new_order = bdd_reorder(org, permutation);
    std::vector<label_t> permutation_inverse = {2,0,1};
    bdd org_back = bdd_reorder(new_order, permutation_inverse);

    output_dot(org, "org.dot", permutation);
    output_dot(new_order, "new.dot");
    output_dot(org_back, "org_back.dot", permutation);

    /*
    // Test for reverse path
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

    assignment_stream<> ass_stream(rev_path);
    while (ass_stream.can_pull())
    {
      assignment_t ass = ass_stream.pull();
      std::cout << ass.label << " = " << ass.value << std::endl;
    }
    */
    // =====  Your code ends here  =====
  }

  adiar_printstat();

  adiar_deinit();
  exit(0);
}
