#ifndef ADIAR_INTERNAL_ASSERT_H
#define ADIAR_INTERNAL_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <iostream>

namespace adiar
{
  // LCOV_EXCL_START

  // Based on:
  // - Assert with messages: https://stackoverflow.com/a/37264642
  // - Variadic macro arguments: https://stackoverflow.com/a/11763277
#define adiar_assert_macro(_1,_2,NAME,...) NAME
#define adiar_assert(...) adiar_assert_macro(__VA_ARGS__, adiar_assert2, adiar_assert1)(__VA_ARGS__)

#ifndef NDEBUG
#   define adiar_assert1(Expr)      __adiar_assert(#Expr, Expr, __FILE__, __LINE__)
#   define adiar_assert2(Expr, Msg) __adiar_assert(#Expr, Expr, __FILE__, __LINE__, Msg)

  inline
  void
  __adiar_assert(const char* expr_str, bool expr, const char* file, int line)
  {
    if (!expr) {
      std::cerr << "Assert failed!\n"
                << "Expected:\t" << expr_str << "\n"
                << "Source:\t\t" << file << ", line " << line << std::endl;
      abort();
    }
  }

  inline
  void
  __adiar_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
  {
    if (!expr) {
      std::cerr << "Assert failed:\t" << msg << "\n"
                << "Expected:\t" << expr_str << "\n"
                << "Source:\t\t" << file << ", line " << line << std::endl;
      abort();
    }
  }
#else
#   define adiar_assert1(Expr) ;
#   define adiar_assert2(Expr, Msg) ;
#endif

  // From: https://stackoverflow.com/a/65258501/13300643
#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above)
  [[noreturn]] inline __attribute__((always_inline)) void adiar_unreachable() {__builtin_unreachable();}
#elif defined(_MSC_VER) // MSVC
  [[noreturn]] __forceinline void adiar_unreachable() {__assume(false);}
#else // ???
  inline void adiar_unreachable() {}
#endif

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_ASSERT_H
