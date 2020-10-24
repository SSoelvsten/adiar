// TPIE Imports
#include <tpie/tpie.h>

// COOM Imports
#include <coom/coom.h>

int main(int argc, char* argv[]) {
  // ========== COOM =============
  size_t M = 1024;

  try {
    if (argc > 1) {
      M = std::stoi(argv[1]);
    }
  } catch (std::invalid_argument const &ex) {
    tpie::log_info() << "Invalid number: " << argv[1] << std::endl;
  } catch (std::out_of_range const &ex) {
    tpie::log_info() << "Number out of range: " << argv[1] << std::endl;
  }

  coom::coom_init(M);

  // ===== Your code here... =====



  // ========== COOM =============
  coom::coom_deinit();
  exit(0);
}

