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
  /// \brief Predicate function given value(s) of the `arg_types`.
  ///
  /// \tparam arg_types List of the argument's type in the order, they are
  ///                   supposed to be given. This list may be empty.
  //////////////////////////////////////////////////////////////////////////////
  template<typename... arg_types>
  using predicate = function<bool (arg_types...)>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Consumer function of value(s) of the `arg_types`.
  ///
  /// \remark Most functions that provide values to a consumer will do so in a
  ///         specific order; you may abuse this to improve the performance of
  ///         your code.
  ///
  /// \tparam arg_types List of the argument's type in the order, they are
  ///                   supposed to be given.
  //////////////////////////////////////////////////////////////////////////////
  template<typename... arg_types>
  using consumer = function<void (arg_types...)>;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `begin` and `end` iterator pair into a consumer function.
  ///
  /// \remark The resulting *consumer* function will throw a `std::out_of_range`
  ///                      if `end` is reached but more values are to be added,
  ///                      i.e. if there the given range is not large enough for
  ///                      all values to consume.
  ////////////////////////////////////////////////////////////////////////////
  template<typename iterator_t>
  inline consumer<typename iterator_t::value_type>
  make_consumer(iterator_t &begin, iterator_t &end)
  {
    return [&begin, &end](const typename iterator_t::value_type x) {
      if (begin == end) {
        throw out_of_range("Iterator range unable to contain all generated values");
      }
      *(begin++) = x;
    };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Generator function that *produces* a new value of `ret_type` for
  ///        each call.
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
  template<typename iterator_t>
  inline generator<typename iterator_t::value_type>
  make_generator(iterator_t &begin, iterator_t &end)
  {
    return [&begin, &end]() {
      if (begin == end) {
        // TODO (DD Iterators): change return value depending on `value_type`.
        return static_cast<typename iterator_t::value_type>(-1);
      }
      return *(begin++);
    };
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap an `adiar::internal::file_stream` into a generator function.
  ////////////////////////////////////////////////////////////////////////////
  template<typename stream_t>
  inline generator<typename stream_t::elem_t>
  make_generator(stream_t &s)
  {
    return [&s]() {
      if (!s.can_pull()) {
        return static_cast<typename stream_t::elem_t>(-1);
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
