// TODO: Fix header files
#include <coom/data.cpp>
#include <coom/dot.cpp>

int main(const int argc, const char* argv[]) {
  // ===== TPIE =====
  // Initialize
  tpie::tpie_init();

  size_t available_memory_mb = 128;
  tpie::get_memory_manager().set_limit(available_memory_mb*1024*1024);

  // ===== FAIL FAST ON MISSING ARGUMENT =====
  if (argc == 1) {
    tpie::log_info() << "Please provide a filename(s) to print" << std::endl;
    tpie::tpie_finish();
    exit(1);
  }

  // ===== OUTPUT DOT FILE =====
  int arg = 1; // argument 0 is the executable

  while (arg < argc) {
    const char* filename = argv[arg];

    tpie::file_stream<coom::node> file_stream;
    file_stream.open(filename, tpie::open::read_only | tpie::open::compression_normal);

    std::string filename_str(filename);
    coom::output_dot(file_stream, filename_str);
    file_stream.close();

    arg += 1;
  }

  // ===== TPIE =====
  // Close all of TPIE down again
  tpie::tpie_finish();

  // Return 'all good'
  exit(0);
}
