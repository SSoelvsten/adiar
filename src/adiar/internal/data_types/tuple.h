#ifndef ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
#define ADIAR_INTERNAL_DATA_TYPES_TUPLE_H

#include <adiar/internal/data_types/ptr.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to a set of arguments.
  template<typename elem_t>
  inline elem_t fst(const elem_t t1, const elem_t t2)
  { return std::min(t1, t2); }

  template<typename elem_t>
  inline elem_t snd(const elem_t t1, const elem_t t2)
  { return std::max(t1, t2); }

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
    static_assert(CARDINALITY == 2, "No support (yet) for tuples of that cardinality.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements are supposed to be in sorted order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool IS_SORTED = is_sorted;

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
    template<typename = std::enable_if_t<1 <= CARDINALITY>>
    inline elem_t fst() const
    {
      if constexpr (IS_SORTED) {
        return _elems[0];
      } else if constexpr (CARDINALITY == 2) {
        return adiar::fst(_elems[0], _elems[1]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    template<typename = std::enable_if_t<2 <= CARDINALITY>>
    inline elem_t snd() const
    {
      if constexpr (IS_SORTED) {
        return _elems[1];
      } else if constexpr (CARDINALITY == 2) {
        return adiar::snd(_elems[0], _elems[1]);
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
    template<typename = std::enable_if_t<CARDINALITY == 2>>
    tuple(const elem_t &elem1, const elem_t &elem2) : _elems{elem1,elem2}
    {
      if constexpr (IS_SORTED) {
        adiar_debug(elem1 <= elem2,
                    "A sorted tuple should be given its elements in sorted order");
      }
    }

    /* ============================== COMPARATORS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '<'.
    ////////////////////////////////////////////////////////////////////////////
    template<typename = std::enable_if_t<CARDINALITY == 2>>
    inline bool operator< (const tuple &o) const
    {
      return this->_elems[0] < o._elems[0]
        || (this->_elems[0] == o._elems[0] && this->_elems[1] < o._elems[1]);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '<='.
    ////////////////////////////////////////////////////////////////////////////
    template<typename = std::enable_if_t<CARDINALITY == 2>>
    inline bool operator<= (const tuple &o) const
    {
      return this->_elems[0] < o._elems[0]
        || (this->_elems[0] == o._elems[0] && this->_elems[1] <= o._elems[1]);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '>'.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator> (const tuple &o) const
    { return (o < *this); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical '>='.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator>= (const tuple &o) const
    { return (o <= *this); }

    ////////////////////////////////////////////////////////////////////////////
    template<typename = std::enable_if_t<CARDINALITY == 2>>
    inline bool operator== (const tuple &o) const
    {
      return this->_elems[0] == o._elems[0] && this->_elems[1] == o._elems[1];
    }

    ////////////////////////////////////////////////////////////////////////////
    inline bool operator!= (const tuple &o) const
    { return !(*this == o); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<class tuple_t>
  struct tuple_label
  {
    static inline ptr_uint64::label_t label_of(const tuple_t &t)
    {
      return t.fst().label();
    }
  };

  template<class tuple_t>
  struct tuple_fst_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered first
      return a.fst() < b.fst() ||
        // Group requests to the same tuple together by sorting on the coordinates
        (a.fst() == b.fst() && a < b);
    }
  };

  template<class tuple_t>
  struct tuple_snd_lt
  {
    inline bool operator()(const tuple_t &a, const tuple_t &b)
    {
      // Sort primarily by the element to be encountered second
      return a.snd() < b.snd() ||
        // Group requests to the same tuple together by sorting on the coordinates
        (a.snd() == b.snd() && a < b);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
