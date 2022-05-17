#ifndef ADIAR_INTERNAL_SAFE_NUMBER_H
#define ADIAR_INTERNAL_SAFE_NUMBER_H

#include <sstream>

#include <cnl/overflow_integer.h>
#include <cnl/wide_integer.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief An (overflow safe) size_t (64 bit unsigned number).
  //////////////////////////////////////////////////////////////////////////////
  typedef cnl::overflow_integer<size_t, cnl::saturated_overflow_tag> safe_size_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Unpacks the value from a 'safe_size_t'.
  //////////////////////////////////////////////////////////////////////////////
  inline size_t unpack(const safe_size_t s)
  {
    return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, size_t>(s);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A wide intger, i.e. suport of infinite size.
  //////////////////////////////////////////////////////////////////////////////
  typedef cnl::wide_integer<128, uint32_t> uintwide_t;

  //////////////////////////////////////////////////////////////////////////////
  /// Derivative of the 'cnl:_impl:to_chars_natural' (and 'cnl::_impl:itoc'
  /// inlined) for the 'cnl::wide_integer' without compiler errors.
  //////////////////////////////////////////////////////////////////////////////
  inline void to_stringstream(const uintwide_t &value, std::stringstream &s)
  {
    constexpr size_t base = 10u;
    const uintwide_t quotient = value / base;

    if (quotient) {
      to_stringstream(quotient, s);
    }

    const uintwide_t remainder = value - (quotient * base);
    const int remainder_int = cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, int>(remainder);
    const char remainder_char = static_cast<char>('0'+remainder_int);

    s << remainder_char;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert a wide integer to a string.
  //////////////////////////////////////////////////////////////////////////////
  inline std::string to_string(const uintwide_t &value)
  {
    std::stringstream s;
    to_stringstream(value, s);

    return s.str();
  }
}

#endif // ADIAR_INTERNAL_SAFE_NUMBER_H
