#ifndef ADIAR_FUNCTIONAL_H
#define ADIAR_FUNCTIONAL_H

#include <functional>

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
  /// \tparam ret_type  The type signature of the form `ret_t (args_t...)`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename type_signature>
  using function = std::function<type_signature>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Predicate function given value(s) of the `Args`.
  ///
  /// \tparam Args List of the argument's type in the order, they are supposed
  ///              to be given. This list may be empty.
  //////////////////////////////////////////////////////////////////////////////
  template<typename... Args>
  using predicate = function<bool (Args...)>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Consumer function of value(s) of the `Args`.
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

  // TODO: Use template specialization to simplify the use of an end value, i.e.
  //       `generator<ret_type>::end` or `generator_end<ret_type>::value`.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Generator function that *produces* a new value of `ret_type` for
  ///         each call.
  ///
  /// \remark Most functions that take a generator as the input expect it (1) to
  ///         produce values in a specific order and (2) to provide a certain
  ///         type of value to mark having reached *the end*.
  ///
  /// \tparam ret_type Type of each yielded value from the generator.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ret_type>
  using generator = function<ret_type ()>;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `begin` and `end` iterator pair into a generator function.
  ////////////////////////////////////////////////////////////////////////////
  template<typename ForwardIt>
  inline generator<typename ForwardIt::value_type>
  make_generator(ForwardIt &begin, ForwardIt &end)
  {
    return [&begin, &end]() {
      if (begin == end) {
        // TODO (DD Iterators): change return value depending on `value_type`.
        return static_cast<typename ForwardIt::value_type>(-1);
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
        return static_cast<typename Stream::value_type>(-1);
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
