#ifndef ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
#define ADIAR_INTERNAL_DATA_TYPES_TUPLE_H

#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to a set of arguments.
  // TODO: move into tuple template when no longer used anywhere else but here.

  template<typename elem_t>
  inline elem_t fst(const elem_t t1, const elem_t t2)
  { return std::min(t1, t2); }

  template<typename elem_t>
  inline elem_t snd(const elem_t t1, const elem_t t2)
  { return std::max(t1, t2); }

  template<typename elem_t>
  inline elem_t fst(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::min({t1, t2, t3}); }

  template<typename elem_t>
  inline elem_t snd(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::max(std::min(t1, t2), std::min(std::max(t1,t2),t3)); }

  template<typename elem_t>
  inline elem_t trd(const elem_t t1, const elem_t t2, const elem_t t3)
  { return std::max({t1, t2, t3}); }

  // TODO (QMDD):
  //   Add 4-ary tuples (and possibly a generic implementation?)

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
  template<typename elem_type, uint8_t cardinality = 2, bool is_sorted = false>
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
    static constexpr uint8_t CARDINALITY = cardinality;

    static_assert(CARDINALITY > 0, "A tuple cannot be 'unit' type.");
    static_assert(CARDINALITY <= 3, "No support (yet) for tuples of that cardinality.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements are supposed to be in sorted order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool IS_SORTED = cardinality == 1 || is_sorted;

    /* ============================== ELEMENTS ============================== */
  private:
    elem_t _elems [CARDINALITY];

    /* ============================ DIRECT ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Direct access to tuple elements in the order they were given.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t operator [] (const size_t idx) const
    {
      adiar_debug(idx < CARDINALITY, "Tuple index must be within its cardinality.");
      return _elems[idx];
    }

    /* =========================== ORDERED ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the first element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t fst() const
    {
      if constexpr (IS_SORTED) {
        return _elems[0];
      } else if constexpr (CARDINALITY == 2) {
        return adiar::internal::fst(_elems[0], _elems[1]);
      } else if constexpr (CARDINALITY == 3) {
        return adiar::internal::fst(_elems[0], _elems[1], _elems[2]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t snd() const
    {
      adiar_debug(2 <= CARDINALITY,
                  "Need at least a 2-ary tuple to retrieve the second element.");

      if constexpr (IS_SORTED) {
        return _elems[1];
      } else if constexpr (CARDINALITY == 2) {
        return adiar::internal::snd(_elems[0], _elems[1]);
      } else if constexpr (CARDINALITY == 3) {
        return adiar::internal::snd(_elems[0], _elems[1], _elems[2]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline elem_t trd() const
    {
      adiar_debug(3 <= CARDINALITY,
                  "Need at least a 3-ary tuple to retrieve the third element.");

      if constexpr (IS_SORTED) {
        return _elems[2];
      } else if constexpr (CARDINALITY == 3) {
        return adiar::internal::trd(_elems[0], _elems[1], _elems[2]);
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
      if constexpr (2 <= CARDINALITY) _elems[1] = elem;
      if constexpr (3 <= CARDINALITY) _elems[2] = elem;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 2-ary tuple with the two given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const elem_t &elem1, const elem_t &elem2)
      : _elems{elem1,elem2}
    {
      adiar_debug(CARDINALITY == 2,
                  "Constructor is only designed for 2-ary tuples.");

      if constexpr (IS_SORTED) {
        adiar_debug(elem1 <= elem2,
                    "A sorted tuple should be given its elements in sorted order");
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 3-ary tuple with the three given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const elem_t &elem1, const elem_t &elem2, const elem_t &elem3)
      : _elems{elem1,elem2,elem3}
    {
      adiar_debug(CARDINALITY == 3,
                  "Constructor is only designed for 3-ary tuples.");

      if constexpr (IS_SORTED) {
          adiar_debug(elem1 <= elem2,
                      "A sorted tuple should be given its elements in sorted order");
          adiar_debug(elem2 <= elem3,
                      "A sorted tuple should be given its elements in sorted order");
        }
    }

    /* ============================== COMPARATORS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '<'.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const tuple &o) const
    {
      // TODO: turn into a loop and trust the compiler?
      const bool lt_1 = this->_elems[0] < o._elems[0];
      if constexpr (CARDINALITY == 1) return lt_1;

      const bool lt_2 = lt_1 || (this->_elems[0] == o._elems[0]
                                 && this->_elems[1] < o._elems[1]);
      if constexpr (CARDINALITY == 2) return lt_2;

      const bool lt_3 = lt_2 || (this->_elems[0] == o._elems[0]
                                 && this->_elems[1] == o._elems[1]
                                 && this->_elems[2] < o._elems[2]);
      return lt_3;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '>'.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator> (const tuple &o) const
    { return (o < *this); }

    ////////////////////////////////////////////////////////////////////////////
    inline bool operator== (const tuple &o) const
    {
      // TODO: turn into a loop and trust the compiler?
      const bool eq_1 = this->_elems[0] == o._elems[0];
      if constexpr (CARDINALITY == 1) return eq_1;

      const bool eq_2 = eq_1 && this->_elems[1] == o._elems[1];
      if constexpr (CARDINALITY == 2) return eq_2;

      const bool eq_3 = eq_2 && this->_elems[2] == o._elems[2];
      return eq_3;
    }

    ////////////////////////////////////////////////////////////////////////////
    inline bool operator!= (const tuple &o) const
    { return !(*this == o); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Non-lexicographical comparators
  template<class tuple_t>
  struct tuple_fst_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered first.
      // If non-singleton, sort secondly lexicographically.
      if constexpr (tuple_t::CARDINALITY == 1) {
        return a.fst() < b.fst();
      } else {
        return a.fst() < b.fst() || (a.fst() == b.fst() && a < b);
      }
    }
  };

  template<class tuple_t>
  struct tuple_snd_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered second
      return a.snd() < b.snd() ||
        // Sort secondly lexicographically.
        (a.snd() == b.snd() && a < b);
    }
  };

  template<class tuple_t>
  struct tuple_trd_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered second
      return a.trd() < b.trd() ||
        // Sort secondly lexicographically.
        (a.trd() == b.trd() && a < b);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
