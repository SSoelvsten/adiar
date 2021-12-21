#ifndef ADIAR_ASSERT_H
#define ADIAR_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <adiar/data.h>

#include <iostream>

namespace adiar
{
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

  // LCOV_EXCL_START

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

#   define adiar_unreachable()                                                  \
  std::cerr << "Reached \"unreachable\" statement\n"                            \
            << "Source:\t\t" << __FILE__ << ", line " << __LINE__ << std::endl; \
  __builtin_unreachable()

  // LCOV_EXCL_STOP
}

#endif // ADIAR_ASSERT_H
