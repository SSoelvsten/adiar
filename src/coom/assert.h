#ifndef COOM_ASSERT_H
#define COOM_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <coom/data.h>

#include <iostream>

namespace coom
{
  // Based on the assert with messages as described here:
  // https://stackoverflow.com/a/37264642

#   define coom_assert(Expr, Msg)                     \
  __coom_assert(#Expr, Expr, __FILE__, __LINE__, Msg)

#ifndef NDEBUG
#   define coom_debug(Expr, Msg)                      \
  __coom_assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define coom_debug(Expr, Msg) ;
#endif

  inline void __coom_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
  {
    if (!expr)
      {
        std::cerr << "Assert failed:\t" << msg << "\n"
                  << "Expected:\t" << expr_str << "\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
      }
  }

#ifndef NDEBUG
#   define coom_invariant(Expr, Name)                  \
  __coom_invariant(#Expr, Expr, __FILE__, __LINE__, Name)
#else
#   define coom_invariant(Expr, Name) ;
#endif

  inline void __coom_invariant(const char* expr_str, bool expr, const char* file, int line, const char* name)
  {
    if (!expr)
      {
        std::cerr << "Invariant '" << name << "' failed\n"
                  << "Expected:\t" << expr_str << "\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
      }
  }
}

#endif // COOM_ASSERT_H
