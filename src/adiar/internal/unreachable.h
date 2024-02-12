#ifndef ADIAR_INTERNAL_UNREACHABLE_H
#define ADIAR_INTERNAL_UNREACHABLE_H

namespace adiar
{
  // LCOV_EXCL_START

  // From: https://stackoverflow.com/a/65258501/13300643
#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or
                // above)
  [[noreturn]] inline __attribute__((always_inline)) void
  adiar_unreachable()
  {
    __builtin_unreachable();
  }
#elif defined(_MSC_VER) // MSVC
  [[noreturn]] __forceinline void
  adiar_unreachable()
  {
    __assume(false);
  }
#else                   // ???
  inline void
  adiar_unreachable()
  {}
#endif

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_UNREACHABLE_H
