#ifndef ADIAR_TYPES_H
#define ADIAR_TYPES_H

#include <optional>
#include <utility>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Possible values to assign a variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  enum class assignment : signed char
  {
    /** Assigning to `false`. */
    False = 0,
    /** Assigning to `true`. */
    True  = 1,
    /** Assigning to no value (i.e. both `true` and `false` are valid). */
    None  = -1
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Possible guarantees of a variable permutation/remapping `m` of type `int -> int`.
  ///
  /// \details The ordering on the values of `replace_type` reflect the strength of the guarantee.
  ///          For example, `replace_type::Affine < replace_type::Monotone` since any affine
  ///          function is also monotone.
  ///
  /// \see bdd_replace
  //////////////////////////////////////////////////////////////////////////////////////////////////
  enum class replace_type : signed char
  {
    /** Any variable remapping without any guarantees on `m`. */
    Non_Monotone = 3,

    /** For any `x < y` then the mapped values preserve that order, i.e. `m(x) < m(y)`. */
    Monotone     = 2,

    /* TODO (constant time variable replacement): `m(x)` is of the form `ax + b` for a positive
                                                  fraction `a` and integer `b`. */
    //Affine      = 1,

    /* TODO (faster version of 'Affine'): `m(x)` is of the form `x + b` for an integer `b`. */
    //Shift       = 0,

    /** Nothing needs to be done, as `m(x) = x`. */
    Identity      = -1
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A pair of values.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T1, typename T2>
  using pair = std::pair<T1, T2>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::pair`, deducing the target type based on the types of the arguments.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T1, typename T2>
  constexpr pair<T1, T2>
  make_pair(const T1& t1, const T2& t2)
  {
    return std::make_pair(t1, t2);
  }

  /*
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::pair`, deducing the target type based on the types of the arguments.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<typename T1, typename T2>
  constexpr pair<T1, T2>
  make_pair(T1 &&t1, T2 &&t2)
  { return std::make_pair(std::move(t1), std::move(t2)); }
  */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief An optional value, i.e. a possibly existent value.
  ///
  /// \details Not having a value is for example used to indicate the end of streams and generators.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using optional = std::optional<T>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an empty `adiar::optional`, i.e. *None*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  constexpr optional<T>
  make_optional()
  {
    return optional<T>();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::optional` with *Some* value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  constexpr optional<T>
  make_optional(const T& t)
  {
    return std::make_optional(t);
  }

  /*
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::optional` with *Some* value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<typename T>
  constexpr optional<T>
  make_optional(T &&t)
  { return std::make_optional(std::move(t)); }
  */
}

#endif // ADIAR_TYPES_H
