#ifndef ADIAR_INTERNAL_ASSERT_H
#define ADIAR_INTERNAL_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <iostream>

namespace adiar
{
  // LCOV_EXCL_START

  // Based on the assert with messages as described here:
  // https://stackoverflow.com/a/37264642

#   define adiar_assert(Expr, Msg)                      \
  __adiar_assert(#Expr, Expr, __FILE__, __LINE__, Msg)

#ifndef NDEBUG
#   define adiar_debug(Expr, Msg)                       \
  __adiar_assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define adiar_debug(Expr, Msg) ;
#endif

  inline void __adiar_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
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
#   define adiar_invariant(Expr, Name)                      \
  __adiar_invariant(#Expr, Expr, __FILE__, __LINE__, Name)
#else
#   define adiar_invariant(Expr, Name) ;
#endif

  inline void __adiar_invariant(const char* expr_str, bool expr, const char* file, int line, const char* name)
  {
    if (!expr)
      {
        std::cerr << "Invariant '" << name << "' failed\n"
                  << "Expected:\t" << expr_str << "\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
      }
  }


#ifndef NDEBUG
#   define adiar_precondition(Expr)                     \
  __adiar_precondition(#Expr, Expr, __FILE__, __LINE__)
#else
#   define adiar_precondition(Expr) ;
#endif

  inline void __adiar_precondition(const char* expr_str, bool expr, const char* file, int line)
  {
    if (!expr)
      {
        std::cerr << "Precondition \t'" << expr_str << "' failed\n"
                  << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
      }
  }

  // From: https://stackoverflow.com/a/65258501/13300643
#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above)
  [[noreturn]] inline __attribute__((always_inline)) void adiar_unreachable() {__builtin_unreachable();}
#elif defined(_MSC_VER) // MSVC
  [[noreturn]] __forceinline void adiar_unreachable() {__assume(false);}
#else // ???
  inline void unreachable() {}
#endif

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_ASSERT_H
