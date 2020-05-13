#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include "test_data.cpp"
#include "test_tpie.cpp"

#include "test_coom_evaluate.cpp"
#include "test_coom_reduce.cpp"
#include "test_coom_restrict.cpp"
#include "test_coom_apply.cpp"

int main(int argc, char* argv[]) {
  return bandit::run(argc, argv);
}

