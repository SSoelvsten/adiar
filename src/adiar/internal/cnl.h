#ifndef ADIAR_INTERNAL_CNL_H
#define ADIAR_INTERNAL_CNL_H

#include <sstream>

#include <cnl/fraction.h>
#include <cnl/overflow_integer.h>
#include <cnl/wide_integer.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief An (overflow safe) size_t (64 bit unsigned number).
  //////////////////////////////////////////////////////////////////////////////
  using safe_size_t = cnl::overflow_integer<size_t, cnl::saturated_overflow_tag>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Unpacks the value from a 'safe_size_t'.
  //////////////////////////////////////////////////////////////////////////////
  inline size_t to_size(const safe_size_t s)
  {
    return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, size_t>(s);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wide intger, i.e. support of infinite size.
  //////////////////////////////////////////////////////////////////////////////
  using uintwide_t = cnl::wide_integer<128, uint32_t>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Fraction using wide intger.
  //////////////////////////////////////////////////////////////////////////////
  using uintwide_frac_t = cnl::fraction<uintwide_t, uintwide_t>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute nominator / denominator as a double.
  //////////////////////////////////////////////////////////////////////////////
  inline double frac(uintwide_t numerator, uintwide_t denominator) {
    const uintwide_frac_t frac(numerator, denominator);
    return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, double>(frac);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute (100 * nominator) / denominator as a double.
  //////////////////////////////////////////////////////////////////////////////
  inline double percent_frac(uintwide_t numerator, uintwide_t denominator) {
    return 100.0 * frac(numerator, denominator);
  }

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

#endif // ADIAR_INTERNAL_CNL_H
