#include <coom/coom.h>

int main(const int argc, const char* argv[]) {
  // ===== FAIL FAST ON MISSING ARGUMENT =====
  if (argc <= 1) {
    std::cerr << "Please provide one or more filename(s) to print DOT files for";
    exit(1);
  }

  coom::coom_init(512);

  // ===== OUTPUT DOT FILE =====
  int arg = 1; // argument 0 is the executable

  while (arg < argc) {
    coom::node_file nf(argv[arg]);
    coom::output_dot(nf, argv[arg]);

    arg += 1;
  }

  coom::coom_deinit();
  exit(0);
}
