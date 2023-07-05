#ifndef ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
#define ADIAR_INTERNAL_DATA_TYPES_TUPLE_H

#include <algorithm>

#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  // TODO: when used with something else than just integers: turn into taking
  //       and returning 'const&' arguments.

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to one elements.

  // 'first(t1)' is not needed, it's just the one element...

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to two elements.
  template<typename elem_t>
  inline elem_t first(const elem_t t1, const elem_t t2)
  { return std::min(t1, t2); }

  template<typename elem_t>
  inline elem_t second(const elem_t t1, const elem_t t2)
  { return std::max(t1, t2); }

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to three elements.
  template<typename elem_t>
  inline elem_t first(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::min({t1, t2, t3}); }

  template<typename elem_t>
  inline elem_t second(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::max(std::min(t1, t2), std::min(std::max(t1,t2),t3)); }

  template<typename elem_t>
  inline elem_t third(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::max({t1, t2, t3}); }

  // TODO (QMDD):
  //   Add 4-ary tuples

  // TODO (Nested Sweeping Clean Up):
  //   - Map function to create a tuple<B> from tupla<A> and f : A -> B

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Tuple holding two elements and providing an ordered access.
  ///
  /// \tparam elem_type   The type of elements within the tuple. This part
  ///                     should have a partial ordering '<'.
  ///
  /// \tparam cardinality The number of elements in the tuple
  ///                     (default: `2`)
  ///
  /// \tparam is_sorted   Whether elements are given in sorted order
  ///                     (default: `false`).
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_type, uint8_t CARDINALITY = 2, bool IS_SORTED = false>
  class tuple
  {
    /* ============================== CONSTANTS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the elements in the tuple.
    ////////////////////////////////////////////////////////////////////////////
    typedef elem_type elem_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements stored in the tuple.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t cardinality = CARDINALITY;

    static_assert(CARDINALITY > 0, "A tuple cannot be 'unit' type.");
    static_assert(CARDINALITY <= 3, "No support (yet) for tuples of that cardinality.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements are supposed to be in sorted order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool is_sorted = cardinality == 1 || IS_SORTED;

    /* ============================== ELEMENTS ============================== */
  private:
    elem_t _elems [cardinality];

    /* ============================ DIRECT ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Direct access to tuple elements in the order they were given.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t operator [] (const size_t idx) const
    {
      adiar_debug(idx < cardinality, "Tuple index must be within its cardinality.");
      return _elems[idx];
    }

    /* =========================== ORDERED ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the first element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t first() const
    {
      static_assert(1 <= cardinality,
                    "Must at least be a 1-ary tuple to retrieve the first element.");

      if constexpr (is_sorted) {
        return _elems[0];
      } else if constexpr (cardinality == 2) {
        return adiar::internal::first(_elems[0], _elems[1]);
      } else if constexpr (cardinality == 3) {
        return adiar::internal::first(_elems[0], _elems[1], _elems[2]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t second() const
    {
      static_assert(2 <= cardinality,
                    "Must at least be a 2-ary tuple to retrieve the second element.");

      if constexpr (is_sorted) {
        return _elems[1];
      } else if constexpr (cardinality == 2) {
        return adiar::internal::second(_elems[0], _elems[1]);
      } else if constexpr (cardinality == 3) {
        return adiar::internal::second(_elems[0], _elems[1], _elems[2]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t third() const
    {
      static_assert(3 <= cardinality,
                    "Must at least be a 3-ary tuple to retrieve the third element.");

      if constexpr (is_sorted) {
        return _elems[2];
      } else if constexpr (cardinality == 3) {
        return adiar::internal::third(_elems[0], _elems[1], _elems[2]);
      }
    }

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    tuple() = default;
    tuple(const tuple &p) = default;
    ~tuple() = default;

  public:
    // Provide other constructors for ease of use

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initialize a tuple with all elements being the given one.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const elem_t &elem)
      : _elems{elem}
    {
      // TODO: replace with a (templated) default value (?)
      if constexpr (2 <= CARDINALITY) _elems[1] = elem;
      if constexpr (3 <= CARDINALITY) _elems[2] = elem;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 2-ary tuple with the two given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const elem_t &elem1, const elem_t &elem2)
      : _elems{elem1,elem2}
    {
      static_assert(cardinality == 2,
                    "Constructor is only designed for 2-ary tuples.");

      if constexpr (is_sorted) {
        adiar_precondition(elem1 <= elem2);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 3-ary tuple with the three given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const elem_t &elem1, const elem_t &elem2, const elem_t &elem3)
      : _elems{elem1,elem2,elem3}
    {
      static_assert(cardinality == 3,
                    "Constructor is only designed for 3-ary tuples.");

      if constexpr (is_sorted) {
        adiar_precondition(elem1 <= elem2);
        adiar_precondition(elem2 <= elem3);
        }
    }

    /* ============================== COMPARATORS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Less-Than.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const tuple &o) const
    {
      // TODO: manual loop unrolling?
      for (size_t i = 0; i < cardinality; i++) {
        if (this->_elems[i] < o._elems[i]) { return true; };
        if (this->_elems[i] > o._elems[i]) { return false; };
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Greater-Than.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator> (const tuple &o) const
    { return (o < *this); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Point-wise Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator== (const tuple &o) const
    {
      // TODO: manual loop unrolling?
      for (size_t i = 0; i < cardinality; i++) {
        if (this->_elems[i] != o._elems[i]) { return false; };
      }
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Point-wise Inequality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator!= (const tuple &o) const
    { return !(*this == o); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Less-Than and Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator<= (const tuple &o) const
    { return (*this < o) || (*this == o); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Greater-Than and Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator>= (const tuple &o) const
    { return (*this > o) || (*this == o); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Non-lexicographical comparators
  template<class tuple_t>
  struct tuple_first_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered first.
      // If non-singleton, sort secondly lexicographically.
      if constexpr (tuple_t::cardinality == 1) {
        return a.first() < b.first();
      } else {
        return a.first() < b.first() || (a.first() == b.first() && a < b);
      }
    }
  };

  template<class tuple_t>
  struct tuple_second_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered second
      return a.second() < b.second() ||
        // Sort secondly lexicographically.
        (a.second() == b.second() && a < b);
    }
  };

  template<class tuple_t>
  struct tuple_third_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered third
      return a.third() < b.third() ||
        // Sort secondly lexicographically.
        (a.third() == b.third() && a < b);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
