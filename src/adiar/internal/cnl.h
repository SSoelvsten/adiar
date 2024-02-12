#ifndef ADIAR_INTERNAL_CNL_H
#define ADIAR_INTERNAL_CNL_H

#include <sstream>

#include <cnl/fraction.h>
#include <cnl/overflow_integer.h>
#include <cnl/wide_integer.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Wide integer, i.e. a non-overflowing unsigned integer capable of
  ///          holding arbitrarily large values.
  //////////////////////////////////////////////////////////////////////////////
  using uintwide = cnl::wide_integer<128, uint32_t>;

  namespace internal
  {
    ////////////////////////////////////////////////////////////////////////////
    /// \brief An (overflow safe) size_t (64 bit unsigned number).
    ////////////////////////////////////////////////////////////////////////////
    using safe_size_t = cnl::overflow_integer<size_t, cnl::saturated_overflow_tag>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Unpacks the value from a 'safe_size_t'.
    ////////////////////////////////////////////////////////////////////////////
    inline size_t
    to_size(const safe_size_t s)
    {
      return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, size_t>(s);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Compute nominator / denominator as a double.
    ////////////////////////////////////////////////////////////////////////////
    inline double
    frac(uintwide numerator, uintwide denominator)
    {
      using frac_type = cnl::fraction<uintwide, uintwide>;

      const frac_type x(numerator, denominator);
      return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, double>(x);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Compute (100 * nominator) / denominator as a double.
    ////////////////////////////////////////////////////////////////////////////
    inline double
    percent_frac(uintwide numerator, uintwide denominator)
    {
      return 100.0 * frac(numerator, denominator);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Derivative of the 'cnl:_impl:to_chars_natural' (and 'cnl::_impl:itoc'
    /// inlined) for the 'cnl::wide_integer' without compiler errors.
    ////////////////////////////////////////////////////////////////////////////
    inline void
    to_stringstream(const uintwide& value, std::stringstream& s)
    {
      constexpr size_t base   = 10u;
      const uintwide quotient = value / base;

      if (quotient) { to_stringstream(quotient, s); }

      const uintwide remainder = value - (quotient * base);
      const int remainder_int =
        cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, int>(remainder);
      const char remainder_char = static_cast<char>('0' + remainder_int);

      s << remainder_char;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Convert a wide integer to a string.
    ////////////////////////////////////////////////////////////////////////////
    inline std::string
    to_string(const uintwide& value)
    {
      std::stringstream s;
      to_stringstream(value, s);

      return s.str();
    }
  } // namespace adiar::internal
}

#endif // ADIAR_INTERNAL_CNL_H
