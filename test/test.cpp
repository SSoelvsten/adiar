#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <coom/coom.h>

using namespace coom;

#include "test_tpie.cpp"

////////////////////////////////////////////////////////////////////////////////
// To improve the relationship between the tests and the algorithms, we will not
// reverse the order in which we reverse the output.
//
// We also do a few little hacks, to read unreduced output from the file_union
// class.
template <typename T, size_t Files>
class meta_test_stream: public file_stream<meta_t, false>
{
public:
  meta_test_stream(file<T,Files> &f): file_stream(f._meta_file) { }
  meta_test_stream(node_or_arc_file &f);
};

template<>
meta_test_stream<node_t, 1>::meta_test_stream(node_or_arc_file &f): file_stream(f.get<node_file>()._meta_file) { }

template<>
meta_test_stream<arc_t, 2>::meta_test_stream(node_or_arc_file &f): file_stream(f.get<arc_file>()._meta_file) { }

class node_test_stream: public file_stream<node_t, false>
{
public:
  node_test_stream(node_file &f): file_stream(f._files[0]) { }
  node_test_stream(node_or_arc_file &f): file_stream(f.get<node_file>()._files[0]) { }
};

class node_arc_test_stream: public file_stream<arc_t, false>
{
public:
  node_arc_test_stream(arc_file &f): file_stream(f._files[0]) { }
  node_arc_test_stream(node_or_arc_file &f): file_stream(f.get<arc_file>()._files[0]) { }
};

class sink_arc_test_stream: public file_stream<arc_t, false>
{
public:
  sink_arc_test_stream(arc_file &f): file_stream(f._files[1]) { }
  sink_arc_test_stream(node_or_arc_file &f): file_stream(f.get<arc_file>()._files[1]) { }
};
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// COOM Core unit tests
#include "coom/test_data.cpp"
#include "coom/test_union.cpp"

#include "coom/test_file.cpp"

#include "coom/test_priority_queue.cpp"

////////////////////////////////////////////////////////////////////////////////
// COOM BDD unit tests
#include "coom/test_reduce.cpp"

#include "coom/bdd/test_apply.cpp"
#include "coom/bdd/test_assignment.cpp"
#include "coom/bdd/test_build.cpp"
#include "coom/bdd/test_count.cpp"
#include "coom/bdd/test_evaluate.cpp"
#include "coom/bdd/test_negate.cpp"
#include "coom/bdd/test_restrict.cpp"
#include "coom/bdd/test_quantify.cpp"

int main(int argc, char* argv[]) {
  // Initialize COOM and TPIE
  coom_init(1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Close all of TPIE down again
  coom_deinit();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

