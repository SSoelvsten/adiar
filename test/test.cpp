#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <adiar/adiar.h>

using namespace adiar;

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
// ADIAR Core unit tests
#include "adiar/test_data.cpp"
#include "adiar/test_union.cpp"

#include "adiar/test_file.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR Debug unit tests
#include "adiar/test_dot.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR Internal unit tests
#include "adiar/internal/test_levelized_priority_queue.cpp"

#include "adiar/internal/test_isomorphism.cpp"
#include "adiar/internal/test_reduce.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR BDD unit tests
#include "adiar/bdd/test_bdd.cpp"

#include "adiar/bdd/test_apply.cpp"
#include "adiar/bdd/test_assignment.cpp"
#include "adiar/bdd/test_build.cpp"
#include "adiar/bdd/test_count.cpp"
#include "adiar/bdd/test_evaluate.cpp"
#include "adiar/bdd/test_if_then_else.cpp"
#include "adiar/bdd/test_negate.cpp"
#include "adiar/bdd/test_quantify.cpp"
#include "adiar/bdd/test_restrict.cpp"

int main(int argc, char* argv[]) {
#ifdef NDEBUG
  std::cerr << "Warning: Internal assertions are not enabled!\n\n";
#endif

  // Initialize ADIAR and TPIE
  adiar_init(1024 * 1024 * 1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Close all of TPIE down again
  adiar_deinit();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

