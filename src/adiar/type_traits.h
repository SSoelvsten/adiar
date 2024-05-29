#ifndef ADIAR_TYPE_TRAITS_H
#define ADIAR_TYPE_TRAITS_H

#include <type_traits>

namespace adiar
{
  template <bool Condition, typename Type>
  using enable_if = typename std::enable_if<Condition, Type>::type;

  template <typename A, typename B>
  inline constexpr bool is_convertible = std::is_convertible<A, B>::value;

  template <typename A, typename B>
  inline constexpr bool is_same = std::is_same<A, B>::value;

  template <typename A, typename B>
  inline constexpr bool needs_conversion = !is_same<A, B> && is_convertible<A, B>;

  template <typename A>
  inline constexpr bool is_integral = std::is_integral<A>::value;
}

#endif // ADIAR_TYPE_TRAITS_H
