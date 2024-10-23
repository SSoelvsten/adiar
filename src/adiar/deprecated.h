#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

namespace adiar
{
  // LCOV_EXCL_START

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Copy all of the variable labels (in \em ascending order) that occur in the BDD into the
  ///        given container.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  [[deprecated("Use an output iterator")]]
  ForwardIt
  bdd_support(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    bdd_support(f, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<is_mutable<typename ForwardIt::reference>>>
  [[deprecated("Use an output iterator (Did you mean to provide values? Use a constant iterator, for now)")]]
  ForwardIt
  bdd_satmin(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    bdd_satmin(f, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically largest x such that f(x) is true.
  //////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<is_mutable<typename ForwardIt::reference>>>
  [[deprecated("Use an output iterator (Did you mean to provide values? Use a constant iterator, for now)")]]
  ForwardIt
  bdd_satmax(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    bdd_satmax(f, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Copy all of the variable labels (in \em ascending order) that occur in the family into
  ///        the given container.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  [[deprecated("Use an output iterator")]]
  ForwardIt
  zdd_support(const zdd& A, ForwardIt begin, ForwardIt end)
  {
    zdd_support(A, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically smallest set a in A.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  [[deprecated("Use an output iterator")]]
  ForwardIt
  zdd_minelem(const zdd& A, ForwardIt begin, ForwardIt end)
  {
    zdd_minelem(A, make_consumer(begin, end));
    return begin;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Retrieves the lexicographically largest set a in A.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  [[deprecated("Use an output iterator")]]
  ForwardIt
  zdd_maxelem(const zdd& A, ForwardIt begin, ForwardIt end)
  {
    zdd_maxelem(A, make_consumer(begin, end));
    return begin;
  }

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
