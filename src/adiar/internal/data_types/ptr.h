#ifndef ADIAR_INTERNAL_DATA_TYPES_PTR_H
#define ADIAR_INTERNAL_DATA_TYPES_PTR_H

#include <stdint.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  // TODO (ADD (32-bit)):
  //   template 'ptr_uint64' with 'value_t' of how to interpret the bits of a
  //   terminal. To this end, one wants to use 'std::bit_cast' in the internal
  //   logic. Use 'static_assert' to ensure the desired type indeed fits into
  //   62 bits of memory.

  // TODO (ADD (64-bit)):
  // TODO (10+ TiB Decision Diagrams):
  //   Create a new 'ptr_templ' class that does not compress all information
  //   into a single 64-bit unsigned integer. The 'label_t' and 'id_t' should be
  //   provided as template parameters and the 'MAX_ID' and 'MAX_LABEL' should
  //   be derived based on 'std::numeric_limits<XXXX_t>::max()'.
  //
  //   For ADDs it should furthermore be templated with 'value_t'.

  // TODO (LDD):
  //   add new decorator class for 'ptr' templated with a 'data' struct and use
  //   it to store the value. Use this as the an alternative type in the
  //   'node::ptr_t'.

  // TODO (QMDD):
  //   add new decorator class for 'ptr' templated with a 'data' struct and use
  //   it to store the complex-valued weight. Use this as the new type for the
  //   'node::children_t'.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A (possibly flagged) unique identifier of a terminal, an internal
  ///        node, or nothing (`NIL`).
  ///
  /// \remark The layout of a pointer is such, that unique identifiers precede
  ///         terminals which in turn precede NIL. The ordering on unique
  ///         identifiers and terminals are lifted to pointers.
  ///
  /// \remark A pointer may be flagged. For an arc's source this marks the arc
  ///         being a 'high' rather than a 'low' arc.
  ///
  /// \sa uid_t
  //////////////////////////////////////////////////////////////////////////////
  class ptr_uint64
  {
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// To condense almost everything down to mere integer comparisons we
    /// reserve specific parts of a single 64 bit unsigned integer to different
    /// variables.
    ///
    ///   | S | ???????????????????????????????????????????????????? | F |
    ///
    /// Where these three parts represent the following variables:
    ///
    ///  - S : the is_terminal flag. If the terminal flag is set, the L and I
    ///        areas differ (see below for the terminal type description).
    ///
    ///  - ? : The layout of these 62 bits change based on whether it describes
    ///        a terminal, an internal node, or NIL.
    ///
    ///  - F : A boolean flag. This is currently only used in arcs to identify
    ///        high and low arcs (see below).
    ///
    /// We ensure, that the S and ? areas combined uniquely identify all
    /// terminals and nodes. We also notice, that sorting these pointers
    /// directly enforce terminal pointers are sorted after nodes. Finally, two
    /// pointers for the same uid will finally be sorted by the flag.
    ////////////////////////////////////////////////////////////////////////////
    uint64_t _raw;

    // --------------------------------------------------
    // befriend other functions that need access to 'raw'
    template <typename T>
    friend void output_dot(const T& nodes, std::ostream &out);

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of terminal values.
    ////////////////////////////////////////////////////////////////////////////
    typedef bool value_t;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type able to hold the label of a variable.
    ////////////////////////////////////////////////////////////////////////////
    typedef uint32_t label_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for a label.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t LABEL_BITS = 24;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr label_t MAX_LABEL = (1ull << LABEL_BITS) - 1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    typedef uint64_t id_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t ID_BITS = 64 - 2 - LABEL_BITS;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr id_t MAX_ID = (1ull << ID_BITS) - 1;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    ptr_uint64() = default;
    ptr_uint64(const ptr_uint64 &p) = default;
    ~ptr_uint64() = default;

  protected:
    constexpr ptr_uint64(const uint64_t raw) : _raw(raw)
    { }

    /* ============================= ATTRIBUTES ============================= */
  public:
    friend ptr_uint64 flag(const ptr_uint64 &p);
    friend ptr_uint64 unflag(const ptr_uint64 &p);

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generic bit-flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t FLAG_BIT = 0x0000000000000001ull;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Retrieve the value of the bit-flag within a pointer.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_flagged() const
    {
      return _raw & ptr_uint64::FLAG_BIT;
    }

    /* ================================= NIL ================================ */
  protected:
    static constexpr uint64_t NIL_VAL = UINT64_MAX - 1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   A pointer to nothing.
    ///
    /// \details Due to how we create the identifiers for all nodes and
    ///          terminals, we cannot use the common null with value 0. So,
    ///          instead we provide a special value that works with this
    ///          specific setup.
    ///
    /// \remark  A NIL value always comes after all other types of pointers.
    ////////////////////////////////////////////////////////////////////////////
    static inline constexpr ptr_uint64 NIL()
    { return ptr_uint64{ NIL_VAL }; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a pointer is NIL.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_nil() const
    {
      // Check for flagged and unflagged NIL
      return _raw >= NIL_VAL;
    }

    /* ================================ NODES =============================== */

    ////////////////////////////////////////////////////////////////////////////
    /// When the <tt>is_node</tt> flag is true, then it is a pointer to a node,
    /// which is identifiable by two variables:
    ///
    ///  - L : the variable label. For nodes n1 and n2 with n1.label < n2.label,
    ///        we guarantee that n1 comes before n2 in the stream reading order.
    ///
    ///  - I : a unique identifier for the nodes on the same level. For nodes n1
    ///        and n2 with n1.label == n2.label but n1.id < n2.id, we guarantee
    ///        that n1 comes before n2 in the stream reading order.
    ///
    /// These are spaced out in the middle area as follows
    ///
    ///   | S | LLLLLLLLLLLLLLLLLLLL | IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII | F |
    ///
    /// That means that nodes are to be sorted first by their label, and then by
    /// their level-identifier.
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const label_t label, const id_t id)
      : _raw(((uint64_t) label << (ID_BITS + 1)) | (id << 1))
    {
      adiar_debug(label <= MAX_LABEL, "Cannot represent given label");
      adiar_debug(id <= MAX_ID, "Cannot represent given id");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a pointer is for an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_node() const
    {
      return _raw <= ~ptr_uint64::TERMINAL_BIT;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the label from an internal node (label, id).
    ///
    /// \pre `is_node()` evaluates to `true.`
    ////////////////////////////////////////////////////////////////////////////
    inline label_t label() const
    {
      return _raw >> (ID_BITS + 1);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the level identifier from an internal node (label, id).
    ///
    /// \pre `is_node()` evaluates to `true.`
    ////////////////////////////////////////////////////////////////////////////
    inline id_t id() const
    {
      return (_raw >> 1) & MAX_ID;
    }

    /* ============================== TERMINALS ============================= */
  public:
    friend inline ptr_uint64 negate(ptr_uint64 p);

    ////////////////////////////////////////////////////////////////////////////
    /// When the terminal flag is set, then we interpret the middle bits as the
    /// value of the terminal.
    ///
    ///     | S | VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV | F |
    ///
    /// Notice, that this means we will never have to actually visit to retrieve
    /// its value. That is, the only time a terminal has to be explicitly
    /// represented as a node is when the BDD only consists of said terminal.
    ////////////////////////////////////////////////////////////////////////////

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Terminal bit-flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t TERMINAL_BIT = 0x8000000000000000ull;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const value_t v) : _raw(TERMINAL_BIT | (v << 1))
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to a terminal node.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    {
      return !is_nil() && _raw >= ptr_uint64::TERMINAL_BIT;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The value of the terminal this pointer points to.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    ////////////////////////////////////////////////////////////////////////////
    inline value_t value() const
    {
      adiar_precondition(is_terminal());
      // TODO (Attributed Edges):
      //   Negate resulting value based on 'is_flagged()'? It might actually be
      //   better to completely ditch the flag for terminals; this will
      //   simplify quite a lot of the logic.
      return (_raw & ~TERMINAL_BIT) >> 1;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to the `false` terminal.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_false() const
    {
      return is_terminal() && !value();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to the `true` terminal.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_true() const
    {
      return is_terminal() && value();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a pointer is for a node on a given level.
    ////////////////////////////////////////////////////////////////////////////
    inline bool on_level(const label_t level)
    {
      return is_terminal() ? false : label() == level;
    }

    /* ============================== COMPARATOR ============================ */
  public:
    inline bool operator< (const ptr_uint64 &o) const
    { return this->_raw < o._raw; }

    inline bool operator<= (const ptr_uint64 &o) const
    { return this->_raw <= o._raw; }

    inline bool operator> (const ptr_uint64 &o) const
    { return (o < *this); }

    inline bool operator>= (const ptr_uint64 &o) const
    { return (o <= *this); }

    inline bool operator== (const ptr_uint64 &o) const
    { return this->_raw == o._raw; }

    inline bool operator!= (const ptr_uint64 &o) const
    { return !(*this == o); }

    /* ================================ OPERATORS ============================= */
  public:

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the negated value of this
    ///        pointer. The 'flag' is kept as-is.
    ///
    /// \pre   `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator~ () const
    {
      adiar_precondition(this->is_terminal());

      // TODO (ADD):
      //   bit-flip all values inside of the 'value' area.
      return ptr_uint64(2u ^ _raw);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the XOR value of both
    ///        pointers. The 'flag' is also XORed.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator^ (const ptr_uint64 &o) const
    {
      adiar_precondition(this->is_terminal());
      adiar_precondition(o.is_terminal());

      return ptr_uint64(TERMINAL_BIT | (this->_raw ^ o._raw));
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the AND value of both
    ///        pointers. The 'flag' is also ANDed.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator& (const ptr_uint64 &o) const
    {
      adiar_precondition(this->is_terminal());
      adiar_precondition(o.is_terminal());

      return ptr_uint64(this->_raw & o._raw);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the OR value of both
    ///        pointers. The 'flag' is also ORed.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator| (const ptr_uint64 &o) const
    {
      adiar_precondition(this->is_terminal());
      adiar_precondition(o.is_terminal());

      return ptr_uint64(this->_raw | o._raw);
    }
  };

  /* ============================== ATTRIBUTES ============================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to true.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 flag(const ptr_uint64 &p)
  {
    return p._raw | ptr_uint64::FLAG_BIT;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to false.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 unflag(const ptr_uint64 &p)
  {
    return p._raw & (~ptr_uint64::FLAG_BIT);
  }

  /* ============================ TERMINAL NODES ============================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 negate(ptr_uint64 p)
  {
    return ~p;
  }

  /* ============================== CONVERSION ============================== */
  // TODO: Conversion constructor from node
}

#endif // ADIAR_INTERNAL_DATA_TYPES_PTR_H
