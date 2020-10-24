#ifndef COOM_ASSERT_H
#define COOM_ASSERT_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <coom/data.h>

#include <iostream>

namespace coom
{
  // Taken from: https://stackoverflow.com/a/37264642
#ifndef NDEBUG
#   define coom_assert(Expr, Msg)                     \
  __coom_assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define coom_assert(Expr, Msg) ;
#endif

  void __coom_assert(const char* expr_str, bool expr, const char* file, int line, const char* msg);
}

#endif // COOM_ASSERT_H
