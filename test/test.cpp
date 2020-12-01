#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <coom/coom.h>

using namespace coom;

////////////////////////////////////////////////////////////////////////////////
// To improve the relationship between the tests and the algorithms, we will not
// reverse the order in which we read the output.
//
// We also do a few little hacks, to read unreduced output from the file_union
// class.
template <typename T, size_t Files>
using meta_test_stream = meta_stream<T, Files, true>;

class node_test_stream: public node_stream<true>
{
public:
  node_test_stream(node_file &f): node_stream<true>(f) { }
  node_test_stream(bdd &f): node_stream<true>(f) { }
  node_test_stream(__bdd &f): node_stream<true>(f.get<node_file>()) { }
};

class node_arc_test_stream: public node_arc_stream<true>
{
public:
  node_arc_test_stream(arc_file &f): node_arc_stream<true>(f) { }
  node_arc_test_stream(__bdd &bdd): node_arc_stream<true>(bdd.get<arc_file>()) { }
};

class sink_arc_test_stream: public sink_arc_stream<true>
{
public:
  sink_arc_test_stream(arc_file &f): sink_arc_stream<true>(f) { }
  sink_arc_test_stream(__bdd &bdd): sink_arc_stream<true>(bdd.get<arc_file>()) { }
};
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// COOM Core unit tests
#include "coom/test_data.cpp"
#include "coom/test_union.cpp"

#include "coom/test_file.cpp"
#include "coom/test_dot.cpp"

#include "coom/test_priority_queue.cpp"

#include "coom/test_homomorphism.cpp"
#include "coom/test_reduce.cpp"

////////////////////////////////////////////////////////////////////////////////
// COOM BDD unit tests
#include "coom/bdd/test_bdd.cpp"

#include "coom/bdd/test_apply.cpp"
#include "coom/bdd/test_assignment.cpp"
#include "coom/bdd/test_build.cpp"
#include "coom/bdd/test_count.cpp"
#include "coom/bdd/test_evaluate.cpp"
#include "coom/bdd/test_negate.cpp"
#include "coom/bdd/test_quantify.cpp"
#include "coom/bdd/test_restrict.cpp"

int main(int argc, char* argv[]) {
#ifdef NDEBUG
  std::cerr << "Warning: Internal assertions are not enabled!\n\n";
#endif

  // Initialize COOM and TPIE
  coom_init(1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Close all of TPIE down again
  coom_deinit();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

