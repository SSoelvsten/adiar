#ifndef COOM_ASSERT_CPP
#define COOM_ASSERT_CPP

#include "assert.h"

namespace coom
{
  void __coom_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
  {
    if (!expr)
      {
        std::cerr << "Assert failed:\t" << msg << "\n"
                  << "Expected:\t" << expr_str << "\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
      }
  }
}

#endif // COOM_ASSERT_CPP
