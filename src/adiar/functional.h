#ifndef ADIAR_FUNCTIONAL_H
#define ADIAR_FUNCTIONAL_H

#include <functional>
#include <type_traits>

#include <adiar/exception.h>

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__functional Function Objects
///
/// \brief The bridge between you and Adiar.
////////////////////////////////////////////////////////////////////////////////

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__functional
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \name Function Objects
  ///
  /// To bridge the gap between your algorithms and their data structures and
  /// the algorithms of Adiar, we use the abstract notion of *predicate*,
  /// *consumer*, and *generator* functions.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief General-purpose polymorphic function wrapper.
  ///
  /// \tparam TypeSignature The type signature of the form `ret_t (args_t...)`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename TypeSignature>
  using function = std::function<TypeSignature>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Predicate function given value(s) of type(s) `Args`.
  ///
  /// \tparam Args List of the argument's type in the order, they are supposed
  ///              to be given. This list may be empty.
  //////////////////////////////////////////////////////////////////////////////
  template<typename... Args>
  using predicate = function<bool (Args...)>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Consumer function of value(s) of type(s) `Args`.
  ///
  /// \remark Most functions that provide values to a consumer will do so in a
  ///         specific order; you may abuse this to improve the performance of
  ///         your code.
  ///
  /// \tparam Args List of the argument's type in the order, they are supposed
  ///              to be given.
  //////////////////////////////////////////////////////////////////////////////
  template<typename... Args>
  using consumer = function<void (Args...)>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap a `begin` and `end` iterator pair into a consumer function.
  ///
  /// \remark The resulting *consumer* function will throw an `out_of_range`
  ///         if `end` is reached but more values are to be added, i.e. if
  ///         the given range is not large enough for all values to be consumed.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  inline consumer<typename ForwardIt::value_type>
  make_consumer(ForwardIt &begin, ForwardIt &end)
  {
    return [&begin, &end](const typename ForwardIt::value_type x) {
      if (begin == end) {
        throw out_of_range("Iterator range unable to contain all generated values");
      }
      *(begin++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Generator function that *produces* a new value of `RetType` for
  ///         each call.
  ///
  /// \remark Most functions that take a generator as the input expect it (1) to
  ///         produce values in a specific order and (2) to provide a certain
  ///         type of value to mark having reached the \em end. For (1), please
  ///         see the documentation of each respective function, while for (2)
  ///         use `generator_end`.
  ///
  /// \tparam RetType Type of each yielded value from the generator.
  //////////////////////////////////////////////////////////////////////////////
  template<typename RetType>
  using generator = function<RetType ()>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Template specialization to derive the `end` (similar to `EOF`)
  ///        return-value for a generator function.
  ///
  /// \details By default, we provide the integer implementation of -1.
  //////////////////////////////////////////////////////////////////////////////
  template <typename RetType>
  struct generator_end
  {
    using value_type = RetType;

    static_assert(std::is_integral<RetType>::value);
    static constexpr value_type value = static_cast<value_type>(-1);
  };

  // TODO: Specialize for DDs (Both BDDs and ZDDs) to return an empty file.

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `begin` and `end` iterator pair into a generator function.
  ////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  inline generator<typename ForwardIt::value_type>
  make_generator(ForwardIt &begin, ForwardIt &end)
  {
    return [&begin, &end]() {
      if (begin == end) {
        return generator_end<typename ForwardIt::value_type>::value;
      }
      return *(begin++);
    };
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap an `adiar::internal::file_stream` into a generator function.
  ////////////////////////////////////////////////////////////////////////////
  template<typename Stream>
  inline generator<typename Stream::value_type>
  make_generator(Stream &s)
  {
    return [&s]() {
      if (!s.can_pull()) {
        return generator_end<typename Stream::value_type>::value;
      }
      return s.pull();
    };
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_FUNCTIONAL_H
