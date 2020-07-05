#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include "test_tpie.cpp"

#include "test_coom_data.cpp"
//#include "test_coom_evaluate.cpp"
//#include "test_coom_count_paths.cpp"
#include "test_coom_negate.cpp"
//#include "test_coom_reduce.cpp"
#include "test_coom_restrict.cpp"
//#include "test_coom_apply.cpp"

int main(int argc, char* argv[]) {
  // Initialize TPIE
  tpie::tpie_init();

  size_t available_memory_mb = 128;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Close all of TPIE down again
  tpie::tpie_finish();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

