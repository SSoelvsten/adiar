#include <tpie/tpie.h>
#include <coom/restrict.cpp>

using namespace coom;

go_bandit([]() {
    tpie::tpie_init();

    size_t available_memory_mb = 128;
    tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

    describe("COOM: Restrict", [&]() {
      });

    tpie::tpie_finish();
  });
