#ifndef ADIAR_INTERNAL_DATA_TYPES_PTR_H
#define ADIAR_INTERNAL_DATA_TYPES_PTR_H

#include <stdint.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Compute (at compile-time) the (ceiling) log2 of a number.
  ////////////////////////////////////////////////////////////////////////////
  constexpr uint8_t log2(size_t n)
  {
    return n == 0u ? 0u : log2(n / 2u) + 1u;
  }

  // TODO (ADD (32-bit)):
  //   Template 'ptr_uint64' with 'value_t' of how to interpret the bits of a
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
  //   Extend 'ptr_t' to a 'weighted_ptr' with a templated `weight_t`.

  // TODO (QMDD):
  //   Same as for LDD but with the weight specifically being complex values.
  //   Furthermore, template the `OUTDEGREE` to use an extra bit for the out
  //   index.

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
    ///    `S______________________________________________________________F`
    ///
    /// Where these three parts represent the following variables:
    ///
    ///  - `S` : the is_terminal flag. If the terminal flag is set, the contents
    ///          of the middle areas differ (see below).
    ///
    ///  - `_` : The layout of these 62 bits change based on whether it
    ///          describes a terminal, an internal node, or NIL.
    ///
    ///  - `F` : A boolean flag. This is currently only used in arcs to identify
    ///          high and low arcs (see below).
    ///
    /// We ensure, that the S and ? areas combined uniquely identify all
    /// terminals and nodes. We also notice, that sorting these pointers
    /// directly enforce terminal pointers are sorted after nodes. Finally, two
    /// pointers for the same uid will finally be sorted by the flag.
    ////////////////////////////////////////////////////////////////////////////
    // TODO (128 bit integers):
    //   Template with the desired uint type and the number of label bits.
    uint64_t _raw;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of bits.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t TOTAL_BITS = sizeof(uint64_t)*8u;

    // --------------------------------------------------
    // befriend other functions that need access to 'raw'
    template <typename T>
    friend void output_dot(const T& nodes, std::ostream &out);

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    ptr_uint64() = default;
    ptr_uint64(const ptr_uint64 &p) = default;
    ~ptr_uint64() = default;

  protected:
    constexpr ptr_uint64(const uint64_t raw) : _raw(raw)
    { }

    template<typename ptr_t>
    friend class __uid;

    /* ============================= ATTRIBUTES ============================= */
  public:
    friend ptr_uint64 flag(const ptr_uint64 &p);
    friend ptr_uint64 unflag(const ptr_uint64 &p);

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of flags
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t FLAG_BITS = 1u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generic bit-flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t FLAG_MASK = 0x0000000000000001ull;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Generic bit-flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t FLAG_BIT = FLAG_MASK;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Retrieve the value of the bit-flag within a pointer.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_flagged() const
    {
      return _raw & ptr_uint64::FLAG_MASK;
    }

    /* ================================= NIL ================================ */
  protected:
    static constexpr uint64_t NIL_VAL =
      std::numeric_limits<uint64_t>::max() ^ FLAG_MASK;

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
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the out-degree.
    ////////////////////////////////////////////////////////////////////////////
    using out_idx_t = uint64_t;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of out-going edges from a node
    ///
    /// \details As this 'ptr' class is used within the `arc` class, we need to
    ///          store somewhere which index the arc is part of the node. To
    ///          save on space, we reserve some of the bits.
    ////////////////////////////////////////////////////////////////////////////
    // TODO (QMDD):
    //   Make into a template parameter
    static constexpr size_t OUTDEGREE = 2u;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for the out index.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr out_idx_t MAX_OUT_IDX = OUTDEGREE - 1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits used to store the out0index.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t OUT_IDX_BITS = log2(MAX_OUT_IDX);

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type able to hold the label of a variable.
    ////////////////////////////////////////////////////////////////////////////
    typedef uint32_t label_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for a label.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t LABEL_BITS = 24u;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr label_t MAX_LABEL = (1ull << LABEL_BITS) - 1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    using id_t = uint64_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for a level identifier.
    ///
    /// \details Take up the remaining bits for the ID. This dictates the
    ///          maximum width possible for a single level: a level cannot
    ///          exceed \$2^{ID_BITS} \cdot 3 \cdot 8\$ bytes.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t ID_BITS =
      TOTAL_BITS - 1u - LABEL_BITS - OUT_IDX_BITS - FLAG_BITS;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr id_t MAX_ID = (1ull << ID_BITS) - 1;

  private:
    friend ptr_uint64 essential(const ptr_uint64 &p);
    friend ptr_uint64 with_out_idx(const ptr_uint64 &p, const out_idx_t out_idx);

    ////////////////////////////////////////////////////////////////////////////
    /// When the <tt>is_node</tt> flag is true, then it is a pointer to a node,
    /// which is identifiable by two variables:
    ///
    ///  - `L` : the variable label. For nodes n1 and n2 with n1.label <
    ///          n2.label, we guarantee that n1 comes before n2 in the stream
    ///          reading order.
    ///
    ///  - `I` : a unique identifier for the nodes on the same level. For nodes
    ///          n1 and n2 with n1.label == n2.label but n1.id < n2.id, we
    ///          guarantee that n1 comes before n2 in the stream reading order.
    ///
    ///  - `O` : bit with the out-index (used in `arc`).
    ///
    /// These are spaced out in the middle area as follows (out-degree: 1)
    ///
    ///    `SLLLLLLLLLLLLLLLLLLLLLLLLIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOF`
    ///
    /// That means that nodes are to be sorted first by their label, then by
    /// their level-identifier, and finally by their .
    ////////////////////////////////////////////////////////////////////////////
  protected:
    static uint64_t encode_label(const label_t label)
    {
      adiar_assert(label <= MAX_LABEL, "Cannot represent given label");
      return (uint64_t) label << (ID_BITS + OUT_IDX_BITS + FLAG_BITS);
    }

    static uint64_t encode_id(const id_t id)
    {
      adiar_assert(id <= MAX_ID, "Cannot represent given id");
      return (uint64_t) id << (OUT_IDX_BITS + FLAG_BITS);
    }

    static uint64_t encode_out_idx(const out_idx_t out_idx)
    { return (uint64_t) out_idx << (FLAG_BITS); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id) with
    ///        weight 0.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const label_t label, const id_t id)
      : _raw(encode_label(label) | encode_id(id))
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id) with
    ///        given weight.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const label_t label, const id_t id, const out_idx_t out_idx)
      : _raw(encode_label(label) | encode_id(id) | encode_out_idx(out_idx))
    { }

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
      return _raw >> (ID_BITS + OUT_IDX_BITS + FLAG_BITS);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the level identifier from an internal node (label, id).
    ///
    /// \pre `is_node()` evaluates to `true.`
    ////////////////////////////////////////////////////////////////////////////
    inline id_t id() const
    {
      return (_raw >> (OUT_IDX_BITS + FLAG_BITS)) & MAX_ID;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the out-index associated with this pointer.
    ///
    /// \pre `is_node()` evaluates to `true.`
    ///
    /// \sa arc
    ////////////////////////////////////////////////////////////////////////////
    inline out_idx_t out_idx() const
    {
      return (_raw >> FLAG_BITS) & MAX_OUT_IDX;
    }

    /* ============================== TERMINALS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of terminal values.
    ////////////////////////////////////////////////////////////////////////////
    typedef bool value_t;

  public:
    friend inline ptr_uint64 negate(ptr_uint64 p);

    ////////////////////////////////////////////////////////////////////////////
    /// When the terminal flag is set, then we interpret the middle bits as the
    /// value of the terminal.
    ///
    ///    `SVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVF`
    ///
    /// Notice, that this means we will never have to actually visit the
    /// terminal node to retrieve its value. That is, the only time a terminal
    /// has to be explicitly represented as a node is when the BDD only consists
    /// of said terminal.
    ////////////////////////////////////////////////////////////////////////////

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Terminal bit-flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t TERMINAL_BIT = 0x8000000000000000ull;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Terminal bit-flag mask.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint64_t VALUE_MASK = 0x0000000000000002ull;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const value_t v) : _raw(TERMINAL_BIT | (v << FLAG_BITS))
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
      adiar_assert(is_terminal());

      // TODO (Attributed Edges):
      //   Negate resulting value based on 'is_flagged()'? It might actually be
      //   better to completely ditch the flag for terminals; this will
      //   simplify quite a lot of the logic.
      return (_raw & ~TERMINAL_BIT) >> FLAG_BITS;
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
    inline bool on_level(const label_t level) const
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
      adiar_assert(this->is_terminal());
      return ptr_uint64(VALUE_MASK ^ _raw);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the XOR value of both
    ///        pointers. The 'flag' is also XORed.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator^ (const ptr_uint64 &o) const
    {
      adiar_assert(this->is_terminal());
      adiar_assert(o.is_terminal());

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
      adiar_assert(this->is_terminal());
      adiar_assert(o.is_terminal());

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
      adiar_assert(this->is_terminal());
      adiar_assert(o.is_terminal());

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
    return p._raw & (~ptr_uint64::FLAG_MASK);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to false and out-index set to 0.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 essential(const ptr_uint64 &p)
  {
    // We can abuse the bit-layout to boil everything down to a bit mask, and a
    // conditional move instruction. This should be optimisable to
    // 'std::min<size_t>' into very few cpu instructions.
    const uint64_t _raw = p._raw;

    constexpr uint64_t main_mask =
      ~((1ull << ptr_uint64::FLAG_BITS) - 1u);

    constexpr uint64_t node_mask =
      ~(((1ull << ptr_uint64::OUT_IDX_BITS) - 1u) << ptr_uint64::FLAG_BITS) & main_mask;

    return _raw > ptr_uint64::TERMINAL_BIT
      ? (_raw & main_mask)
      : (_raw & node_mask);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with the out-index changed to the given value.
  ///
  /// \pre `p.is_node() == true`
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 with_out_idx(const ptr_uint64 &p,
                                 const ptr_uint64::out_idx_t out_idx)
  {
    adiar_assert(p.is_node());

    constexpr uint64_t out_idx_mask =
      ~(((1ull << ptr_uint64::OUT_IDX_BITS) - 1u) << ptr_uint64::FLAG_BITS);

    return (p._raw & out_idx_mask) | ptr_uint64::encode_out_idx(out_idx);
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
