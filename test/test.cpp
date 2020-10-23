#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <coom/coom.h>

using namespace coom;

// TPIE setup unit tests
#include "test_tpie.cpp"

// COOM unit tests
#include "coom/test_data.cpp"

#include "coom/test_priority_queue.cpp"

#include "coom/test_assignment.cpp"

#include "coom/test_evaluate.cpp"
#include "coom/test_count.cpp"
#include "coom/test_negate.cpp"
#include "coom/test_restrict.cpp"
#include "coom/test_reduce.cpp"
#include "coom/test_apply.cpp"
#include "coom/test_quantify.cpp"

#include "coom/test_build.cpp"

int main(int argc, char* argv[]) {
  // Initialize TPIE
  tpie::tpie_init();

  size_t available_memory_mb = 1024;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Close all of TPIE down again
  tpie::tpie_finish();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

