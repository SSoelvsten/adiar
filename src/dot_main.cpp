#include <adiar/adiar.h>

int main(const int argc, const char* argv[]) {
  // ===== FAIL FAST ON MISSING ARGUMENT =====
  if (argc <= 1) {
    std::cerr << "Please provide one or more filename(s) to print DOT files for";
    exit(1);
  }

  adiar::adiar_init(512);

  // ===== OUTPUT DOT FILE =====
  int arg = 1; // argument 0 is the executable

  while (arg < argc) {
    adiar::node_file nf(argv[arg]);
    adiar::output_dot(nf, argv[arg]);

    arg += 1;
  }

  adiar::adiar_deinit();
  exit(0);
}
