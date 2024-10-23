#ifndef ADIAR_FUNCTIONAL_H
#define ADIAR_FUNCTIONAL_H

#include <functional>
#include <limits>

#include <adiar/exception.h>
#include <adiar/types.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__functional Function Objects
///
/// \brief The bridge between you and Adiar.
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__functional
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Function Objects
  ///
  /// To bridge the gap between your algorithms and their data structures and the algorithms of
  /// Adiar, we use the abstract notion of *predicate*, *consumer*, and *generator* functions.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief General-purpose polymorphic function wrapper.
  ///
  /// \tparam TypeSignature
  ///    The type signature of the form `ret_t (args_t...)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename TypeSignature>
  using function = std::function<TypeSignature>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Predicate function given value(s) of type(s) `Args`.
  ///
  /// \tparam Args
  ///    List of the argument's type in the order, they are supposed to be given. This list may be
  ///    empty.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename... Args>
  using predicate = function<bool(Args...)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Consumer function of value(s) of type(s) `Args`.
  ///
  /// \remark Most functions that provide values to a consumer will do so in a specific order; you
  ///         may abuse this to improve the performance of your code.
  ///
  /// \tparam Args
  ///    List of the argument's type in the order, they are supposed to be given.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Arg>
  using consumer = function<void(Arg)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap an iterator into a consumer function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ValueType, typename OutputIt>
  inline consumer<ValueType>
  make_consumer(OutputIt& iter)
  {
    return [&iter](const ValueType& x) {
      *(iter++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap an iterator into a consumer function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ValueType, typename OutputIt>
  inline consumer<ValueType>
  make_consumer(OutputIt&& iter)
  {
    return [_iter = std::move(iter)](const ValueType& x) mutable {
      *(_iter++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap an iterator into a consumer function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename OutputIt>
  inline consumer<typename OutputIt::container_type::value_type>
  make_consumer(OutputIt& iter)
  {
    using value_type = typename OutputIt::container_type::value_type;

    return [&iter](const value_type& x) {
      *(iter++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap an iterator into a consumer function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename OutputIt>
  inline consumer<typename OutputIt::container_type::value_type>
  make_consumer(OutputIt&& iter)
  {
    using value_type = typename OutputIt::container_type::value_type;

    return [_iter = std::move(iter)](const value_type& x) mutable {
      *(_iter++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap a `begin` and `end` iterator pair into a consumer function.
  ///
  /// \remark The resulting *consumer* function will throw an `out_of_range` if `end` is reached but
  ///         more values are to be added, i.e. if the given range is not large enough for all
  ///         values to be consumed.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  inline consumer<typename ForwardIt::value_type>
  make_consumer(ForwardIt& begin, ForwardIt& end)
  {
    using value_type = typename ForwardIt::value_type;

    return [&begin, &end](const value_type& x) {
      if (begin == end) {
        throw out_of_range("Iterator range unable to contain all generated values");
      }
      *(begin++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Wrap a `begin` and `end` iterator pair into a consumer function.
  ///
  /// \remark The resulting *consumer* function will throw an `out_of_range` if `end` is reached but
  ///         more values are to be added, i.e. if the given range is not large enough for all
  ///         values to be consumed.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  inline consumer<typename ForwardIt::value_type>
  make_consumer(ForwardIt&& begin, ForwardIt&& end)
  {
    using value_type = typename ForwardIt::value_type;

    return [_begin = std::move(begin), _end = std::move(end)](const value_type& x) mutable {
      if (_begin == _end) {
        throw out_of_range("Iterator range unable to contain all generated values");
      }
      *(_begin++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Cost function that assigns a cost to each variable.
  ///
  /// \tparam VarType
  ///    Type of variable labels.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename VarType>
  using cost = function<double(VarType)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Generator function that *produces* a new value of `RetType` for each call.
  ///
  /// \remark Most functions that take a generator as the input expect it (1) to produce values in a
  ///         specific order and (2) to provide a certain type of value to mark having reached the
  ///         \em end. For (1), please see the documentation of each respective function, while for
  ///         (2) use `make_optional<T>()`.
  ///
  /// \tparam RetType
  ///    Type of each yielded value from the generator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename RetType>
  using generator = function<optional<RetType>()>;

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `begin` and `end` iterator pair into a generator function.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  inline generator<typename ForwardIt::value_type>
  make_generator(ForwardIt& begin, ForwardIt& end)
  {
    using value_type = typename ForwardIt::value_type;

    return [&begin, &end]() -> optional<value_type> {
      if (begin == end) { return {}; }
      return *(begin++);
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `begin` and `end` iterator pair into a generator function.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  inline generator<typename ForwardIt::value_type>
  make_generator(ForwardIt&& begin, ForwardIt&& end)
  {
    using value_type = typename ForwardIt::value_type;

    return [_begin = std::move(begin), _end = std::move(end)]() mutable -> optional<value_type> {
      if (_begin == _end) { return {}; }
      return *(_begin++);
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap an `adiar::internal::file_stream` into a generator function.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Stream>
  inline generator<typename Stream::value_type>
  make_generator(Stream& s)
  {
    using value_type = typename Stream::value_type;

    return [&s]() -> optional<value_type> {
      if (!s.can_pull()) { return {}; }
      return s.pull();
    };
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a single value into a generator.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename RetType>
  inline generator<RetType>
  make_generator(const RetType& r)
  {
    return [=, _end = false]() mutable -> optional<RetType> {
      if (_end) { return {}; }

      _end = true;
      return r;
    };
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_FUNCTIONAL_H
