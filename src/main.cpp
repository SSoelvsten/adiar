// TPIE Imports
#include <tpie/tpie.h>

// COOM Imports
#include <coom/data.cpp>
#include <coom/assignment.cpp>
#include <coom/pred.cpp>

#include <coom/debug_data.cpp>
#include <coom/debug_assignment.cpp>
#include <coom/debug.cpp>

#include <coom/assert.cpp>

#include <coom/apply.cpp>
#include <coom/count.cpp>
#include <coom/evaluate.cpp>
#include <coom/reduce.cpp>
#include <coom/restrict.cpp>

using namespace coom;


int main(int argc, char* argv[]) {
  // ===== TPIE =====
  // Initialize
  tpie::tpie_init();

  size_t available_memory_mb = 128;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // ===== COOM =====
  // Your code here...



  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(0);
}

