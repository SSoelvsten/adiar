// TPIE Imports
#include <tpie/tpie.h>

// COOM Imports
#include "data.cpp"
#include "data_pty.cpp"
#include "reduce.cpp"


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

