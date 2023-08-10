#ifndef ADIAR_INTERNAL_ASSERT_H
#define ADIAR_INTERNAL_ASSERT_H

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

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_ASSERT_H
