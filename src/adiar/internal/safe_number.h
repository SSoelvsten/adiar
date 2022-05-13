#ifndef ADIAR_INTERNAL_SAFE_NUMBER_H
#define ADIAR_INTERNAL_SAFE_NUMBER_H

#include <cnl/overflow_integer.h>

namespace adiar
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief An overflow safe variant of the 'size_t' unsigned integer. If a
  ///        value overflows, then it is capped at the maximum value.
  ////////////////////////////////////////////////////////////////////////////
  typedef cnl::overflow_integer<size_t, cnl::saturated_overflow_tag> safe_size_t;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Unpacks the value from a 'safe_size_t'
  ////////////////////////////////////////////////////////////////////////////
  inline size_t unpack(const safe_size_t s)
  {
    return cnl::convert<cnl::saturated_overflow_tag, cnl::saturated_overflow_tag, size_t>(s);
  }
}

#endif // ADIAR_INTERNAL_SAFE_NUMBER_H
