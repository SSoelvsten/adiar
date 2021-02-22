// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

int main(int argc, char* argv[]) {
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

  adiar::adiar_init(M);

  {
    // ===== Your code starts here =====



    // =====  Your code ends here  =====
  }

  adiar::adiar_deinit();
  exit(0);
}

