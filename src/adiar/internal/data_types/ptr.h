#ifndef ADIAR_INTERNAL_DATA_TYPES_PTR_H
#define ADIAR_INTERNAL_DATA_TYPES_PTR_H

#include <limits>
#include <stdint.h>
#include <type_traits>

#include <adiar/internal/assert.h>

namespace adiar::internal
{

  // TODO (ADD (32-bit)):
  //   Template 'ptr_uint64' with 'terminal_t' of how to interpret the bits of a
  //   terminal. To this end, one wants to use 'std::bit_cast' in the internal
  //   logic. Use 'static_assert' to ensure the desired type indeed fits into
  //   62 bits of memory.

  // TODO (ADD (64-bit)):
  // TODO (10+ TiB Decision Diagrams):
  //   Create a new 'ptr_templ' class that does not compress all information
  //   into a single 64-bit unsigned integer. The 'label_type' and 'id_type' should be
  //   provided as template parameters and the 'max_id' and 'max_label' should
  //   be derived based on 'std::numeric_limits<XXXX_t>::max()'.
  //
  //   For ADDs it should furthermore be templated with 'terminal_t'.

  // TODO (LDD):
  // TODO (EVBDD):
  //   Extend 'pointer_type' to a 'weighted_ptr' with a templated `weight_t`.
  //   Specialize for 'bool' to just use the 'flag'.

  // TODO (QMDD):
  //   Same as for LDD but with the weight specifically being complex values.
  //   Furthermore, template the `outdegree` to use an extra bit for the out
  //   index.

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Compute (at compile-time) the (ceiling) log2 of a number.
  ////////////////////////////////////////////////////////////////////////////
  constexpr uint8_t log2(size_t n)
  {
    return n == 0u ? 0u : log2(n / 2u) + 1u;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A (possibly flagged) unique identifier of a terminal, an internal
  ///        node, or nothing (`nil`).
  ///
  /// \remark The layout of a pointer is such, that unique identifiers precede
  ///         terminals which in turn precede nil. The ordering on unique
  ///         identifiers and terminals are lifted to pointers.
  ///
  /// \remark A pointer may be flagged. For an arc's source this marks the arc
  ///         being a 'high' rather than a 'low' arc.
  ///
  /// \see uid_type
  //////////////////////////////////////////////////////////////////////////////
  class ptr_uint64
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Underlying unsigned integer.
    ////////////////////////////////////////////////////////////////////////////
    // TODO (128 bit integers):
    //   Template with the desired uint type and the number of label bits. Or
    //   better is to use `std::conditional_type` to derive the smallest type
    //   that can fit all the requested number of bits.
    using raw_type = uint64_t;

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// To condense almost everything down to mere integer comparisons we
    /// reserve specific parts of a single 64 bit unsigned integer to different
    /// variables.
    ///
    ///    `LLLLLLLLLLL____________________________________________________F`
    ///
    /// Where these three parts represent the following variables:
    ///
    ///  - `L` : These uppermost 24 bits reflect the level. Here, we assign
    ///          special meaning to the two maximal levels.
    ///
    ///  - `_` : These bits change based on whether it describes a terminal, an
    ///          internal node, or nil.
    ///
    ///  - `F` : A boolean flag. This is currently only used in Reduce on an
    ///          arc's target to mark taints by reduction rules and in Nested
    ///          Sweeping on arc's source to mark the originating sweep of an
    ///          arc's source.
    ///
    /// We ensure, that the ? areas combined uniquely identify all terminals and
    /// nodes. We also ensure, that sorting these pointers directly enforce
    /// terminal pointers are sorted after nodes. Finally, two pointers for the
    /// same uid will finally be sorted by the flag.
    ////////////////////////////////////////////////////////////////////////////
    raw_type _raw;

    ////////////////////////////////////////////////////////////////////////////
    // befriend functions and classes that need access to 'raw'...
    template <typename dd_t>
    friend void __print_dot(const dd_t&, std::ostream &);

    template<typename pointer_type>
    friend class __uid;
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of bits.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t total_bits = sizeof(uint64_t)*8u;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(const ptr_uint64 &p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64(ptr_uint64 &&p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~ptr_uint64() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64& operator =(const ptr_uint64 &p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64& operator =(ptr_uint64 &&p) = default;

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reinterpret an unsigned 64 bit integer as a `ptr`.
    ////////////////////////////////////////////////////////////////////////////
    constexpr ptr_uint64(const raw_type raw)
      : _raw(raw)
    { }

    /* ============================ LEVEL FIELD ============================= */

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for the level.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t level_bits = 24u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits to shift into the level field.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t level_shift = total_bits - level_bits;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type able to hold the node's level.
    ////////////////////////////////////////////////////////////////////////////
    // TODO:
    //   Template with 'label bits' and derive with `std::conditional_type` the
    //   smallest type that can fit all the requested number of bits.
    using level_type = uint32_t;

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a level.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr level_type max_level =
      (static_cast<raw_type>(1) << level_bits) - 1u;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the level of the node in question.
    ////////////////////////////////////////////////////////////////////////////
    inline level_type
    level() const
    {
      return this->_raw >> level_shift;
    }

    /* ============================ BOOLEAN FLAG ============================ */

    ////////////////////////////////////////////////////////////////////////////
    // befriend flag modifying functions that need access to protected values.
    friend ptr_uint64 flag(const ptr_uint64 &p);
    friend ptr_uint64 unflag(const ptr_uint64 &p);
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the Boolean flag to store auxiliary information.
    ////////////////////////////////////////////////////////////////////////////
    using flag_type = bool;

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits used for the Boolean bit flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t flag_bits = 1u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits to shift into the flag field.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t flag_shift = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Bit-mask and placement for bit flag.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr raw_type flag_bit = static_cast<raw_type>(true);

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Retrieve the value of the bit flag within a pointer.
    ////////////////////////////////////////////////////////////////////////////
    inline flag_type is_flagged() const
    {
      return this->_raw & flag_bit;
    }

    /* ================================ DATA ================================ */

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits remaining for data values.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t data_bits = total_bits - (level_bits + flag_bits);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits to shift into the data area.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t data_shift = flag_shift + 1u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the raw data values.
    ////////////////////////////////////////////////////////////////////////////
    inline raw_type
    data() const
    {
      constexpr raw_type data_mask = (static_cast<raw_type>(1) << data_bits) - 1;
      return (this->_raw >> data_shift) & data_mask;
    }

    /* ================================= nil ================================ */

    ////////////////////////////////////////////////////////////////////////////
    /// In some algorithms, we need to store pointers to `null` (e.g. if there
    /// does not exist a source of an edge). Due to how we create the
    /// identifiers for all nodes and terminals, we cannot use the common null
    /// with value 0. So, instead we provide a special value that works with
    /// this specific setup.
    ///
    /// Furthermore, it turns out that it is beneficial if the `nil` value comes
    /// *after* all other values, i.e. it is at level 'infinity'.
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The (maximal) level is reserved for `nil` pointers.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr level_type nil_level = max_level;
    static_assert(nil_level <= max_level);

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Compile-time derived minimal raw value for `nil`.
    ///
    /// \see nil
    ////////////////////////////////////////////////////////////////////////////
    static constexpr raw_type min_nil =
      static_cast<raw_type>(nil_level) << level_shift;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   A pointer to nothing.
    ///
    /// \remark  A nil value always comes after all other types of pointers.
    ////////////////////////////////////////////////////////////////////////////
    static inline constexpr ptr_uint64 nil()
    { return ptr_uint64{ min_nil }; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a pointer is nil.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_nil() const
    {
      // Since `nil_level` is maximal and the level bits are most significant,
      // then we can implement `is_nil()` as a simple comparison to quickly
      // obtain the result without having to deal with flags and other pieces
      // data stored within the pointer.
      return min_nil <= this->_raw;
    }

    /* ================================ NODES =============================== */

    ////////////////////////////////////////////////////////////////////////////
    /// When the `is_node` flag is true, then it is a pointer to a node, which
    /// next to its level (i.e. its variable label) also must be identified by
    /// its 'level identifier'
    ///
    /// If used as part of an arc's source, then we also will store extra
    /// information about the out-degree.
    ///
    /// Hence, a node poitner has its bits laid out as follows (out-degree: 1)
    ///
    ///    `LLLLLLLLLLLLLLLLLLLIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIOF`
    ///
    /// That means that nodes are to be sorted first by their label, then by
    /// their level-identifier, and finally by their .
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type able to hold the label of a variable.
    ////////////////////////////////////////////////////////////////////////////
    using label_type = level_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    // TODO: Find the smallest integer that can fit `(data_bits - out_idx_bits)`
    using id_type = raw_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the out-degree.
    ////////////////////////////////////////////////////////////////////////////
    using out_idx_type = uint8_t;

  private:
    ////////////////////////////////////////////////////////////////////////////
    // befriend flag modifying functions that need access to protected values.
    friend ptr_uint64
    essential(const ptr_uint64 &p);

    friend ptr_uint64
    with_out_idx(const ptr_uint64 &p, const out_idx_type out_idx);
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Number of out-going edges from a node
    ///
    /// \details As this 'ptr' class is used within the `arc` class, we need to
    ///          store somewhere which index the arc is part of the node. To
    ///          save on space, we reserve some of the least significant bits.
    ////////////////////////////////////////////////////////////////////////////
    // TODO (QMDD):
    //   Make into a template parameter
    static constexpr size_t outdegree = 2u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for the out index.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr out_idx_type max_out_idx = outdegree - 1;

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of bits used to store the out-index.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t out_idx_bits = log2(max_out_idx);
    static_assert(out_idx_bits <= 8*sizeof(out_idx_type));

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of bits for a level identifier.
    ///
    /// \details Take up the remaining bits for the ID. This dictates the
    ///          maximum width possible for a single level: a level cannot
    ///          exceed \$2^{id_bits} \cdot 3 \cdot 8\$ bytes.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t id_bits =
      total_bits - level_bits - out_idx_bits - flag_bits;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a level identifier.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr id_type max_id = (1ull << id_bits) - 1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief  The maximal possible value for a unique identifier's label.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr label_type max_label = max_level - 2u;
    static_assert(max_label < max_level);

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id) with
    ///        weight 0.
    ////////////////////////////////////////////////////////////////////////////
    constexpr ptr_uint64(const label_type label, const id_type id)
      : _raw((static_cast<raw_type>(label) << level_shift) |
             (static_cast<raw_type>(id) << (data_shift + out_idx_bits)))
    {
      // TODO: Add Debug checks for non-constexpr context
      // adiar_assert(label <= max_label, "Cannot represent given label");
      // adiar_assert(id    <= max_id,    "Cannot represent given id");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id) with
    ///        given weight.
    ////////////////////////////////////////////////////////////////////////////
    constexpr ptr_uint64(const label_type label,
                         const id_type id,
                         const out_idx_type out_idx)
      : _raw((static_cast<raw_type>(label) << level_shift) |
             (static_cast<raw_type>(id) << (data_shift + out_idx_bits)) |
             (static_cast<raw_type>(out_idx) << data_shift))
    {
      // TODO: Add Debug checks for non-constexpr context
      // adiar_assert(label   <= max_label,   "Cannot represent given label");
      // adiar_assert(id      <= max_id,      "Cannot represent given id");
      // adiar_assert(out_idx <= max_out_idx, "Cannot represent given id");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a pointer is for an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    is_node() const
    {
      // Derive at compile-time the maximal internal node pointer value.
      constexpr raw_type max_node = ptr_uint64(max_label, max_id, max_out_idx)._raw
                                  | ptr_uint64::flag_bit;

      // Since nodes' levels are the smallest ones and the level bits are most
      // significant, then we can implement `is_node()` as a simple comparison
      // with a value where all values are maximal.
      return this->_raw <= max_node;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the label from an internal node (label, id).
    ///
    /// \pre `is_node()` evaluates to `true.`
    ////////////////////////////////////////////////////////////////////////////
    inline label_type
    label() const
    {
      return this->level();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the level identifier from an internal node (label, id).
    ///
    /// \pre `is_node()` evaluates to `true.`
    ////////////////////////////////////////////////////////////////////////////
    inline id_type
    id() const
    {
      return this->data() >> out_idx_bits;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extract the out-index associated with this pointer.
    ///
    /// \pre `is_node()` evaluates to `true.`
    ///
    /// \see arc
    ////////////////////////////////////////////////////////////////////////////
    inline out_idx_type
    out_idx() const
    {
      return this->data() & static_cast<raw_type>(max_out_idx);
    }

    /* ============================== TERMINALS ============================= */

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// When the terminal flag is set, then we interpret the middle bits as the
    /// value of the terminal.
    ///
    ///    `LLLLLLLLLLVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVF`
    ///
    /// Notice, that this means we will never have to actually visit the
    /// terminal node to retrieve its value. That is, the only time a terminal
    /// has to be explicitly represented as a node is when the BDD only consists
    /// of said terminal.
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // befriend terminal functions that need access to protected values.
    friend inline ptr_uint64 negate(ptr_uint64 p);
    ////////////////////////////////////////////////////////////////////////////

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of terminal values.
    ////////////////////////////////////////////////////////////////////////////
    // TODO (32-bit ADD):
    //   Template 'terminal_type'
    using terminal_type = bool;

    static_assert(8*sizeof(terminal_type) <= data_bits,
                  "Type for terminal values may overflow data field");

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Terminal level.
    ///
    /// \remark Since terminals come after all internal nodes, but the maximal
    ///         level is reserved for `nil`, we reserve the second-maximal level
    ///         for terminals.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr level_type terminal_level = max_level - 1u;
    static_assert(terminal_level < max_level);

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Minimal possible raw value for a terminal.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr raw_type min_terminal =
      static_cast<raw_type>(terminal_level) << level_shift;

    // TODO (32-bit ADD):
    //   Add `max_terminal` at compile-time. Note, we are interested in the raw
    //   unsigned encoding (probably by bit-wise oring the minimum and the
    //   maximum).

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    constexpr ptr_uint64(const terminal_type v)
      : _raw((static_cast<raw_type>(terminal_level) << level_shift) |
             (static_cast<raw_type>(v) << data_shift))
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to a terminal node.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    is_terminal() const
    {
      // We could check whether the raw value is in the interval [min_terminal,
      // min_nil), i.e. `min_terminal <= this->_raw && this->_raw < min_nil`.
      // But, doing so would include branching with `&&` and two comparisons.
      //
      // The test below only requires a single right-shift together with a
      // comparison.
      return this->level() == terminal_level;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The value of the terminal this pointer points to.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    ////////////////////////////////////////////////////////////////////////////
    inline terminal_type
    value() const
    {
      adiar_assert(is_terminal());

      // TODO (Attributed Edges):
      //   Negate resulting value based on 'is_flagged()'? It might actually be
      //   better to completely ditch the flag for terminals; this will
      //   simplify quite a lot of the logic.
      return static_cast<terminal_type>(this->data());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to the `false` terminal.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    is_false() const
    {
      return is_terminal() && !value();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this pointer points to the `true` terminal.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    is_true() const
    {
      return is_terminal() && value();
    }

    /* ============================== COMPARATOR ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical ordering on internal nodes (i, id), followed by
    ///        terminals `false`, `true`, and finally `nil`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const ptr_uint64 &o) const
    { return this->_raw < o._raw; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical ordering on internal nodes (i, id), followed by
    ///        terminals `false`, `true`, and finally `nil`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator<= (const ptr_uint64 &o) const
    { return this->_raw <= o._raw; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical ordering on internal nodes (i, id), followed by
    ///        terminals `false`, `true`, and finally `nil`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator> (const ptr_uint64 &o) const
    { return (o < *this); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical ordering on internal nodes (i, id), followed by
    ///        terminals `false`, `true`, and finally `nil`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator>= (const ptr_uint64 &o) const
    { return (o <= *this); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether pointers reference the same node and also share the same
    ///        auxiliary data, i.e. `flag` and `out_idx`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator== (const ptr_uint64 &o) const
    { return this->_raw == o._raw; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether pointers reference the same node and also share the same
    ///        auxiliary data, i.e. `flag` and `out_idx`.
    ////////////////////////////////////////////////////////////////////////////
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

      // TODO: generalise for non-boolean terminals
      constexpr terminal_type max_value = true;

      constexpr raw_type value_mask =
        static_cast<raw_type>(max_value) << data_shift;

      return ptr_uint64(value_mask ^ _raw);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer to the terminal with the XOR value of both
    ///        pointers' value. The 'flag' is also XORed.
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    //////////////////////////////////////////////////////////////////////////////
    ptr_uint64 operator^ (const ptr_uint64 &o) const
    {
      adiar_assert(this->is_terminal());
      adiar_assert(o.is_terminal());

      // Since both are terminals, they have the same level. Hence, when XORing
      // their raw values, the entire level field will be gone. Yet, we have in
      // 'ptr_uint64::min_terminal' the terminal level pre-shifted at
      // compile-time.
      return ptr_uint64(ptr_uint64::min_terminal | (this->_raw ^ o._raw));
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

      // Since both are terminals, they have the same level. Hence, when ANDing
      // their raw values, the entire level field will stay as-is.
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

      // Since both are terminals, they have the same level. Hence, when ORing
      // their raw values, the entire level field will stay as-is.
      return ptr_uint64(this->_raw | o._raw);
    }
  };

  /* ============================= BOOLEAN FLAG ============================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to true.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 flag(const ptr_uint64 &p)
  {
    return p._raw | ptr_uint64::flag_bit;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to false.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 unflag(const ptr_uint64 &p)
  {
    return p._raw & (~ptr_uint64::flag_bit);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The *essential* pointer, i.e. one without any auxiliary data, e.g.
  ///        the Boolean flag and the out-index.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64 essential(const ptr_uint64 &p)
  {
    // Assuming 'max_out_idx' is consecutive 1s.
    constexpr ptr_uint64::raw_type out_idx_mask =
      static_cast<ptr_uint64::raw_type>(ptr_uint64::max_out_idx) << ptr_uint64::data_shift;

    // We can abuse the bit-layout to boil everything down to a bit mask, and a
    // conditional move instruction (see `ptr_uint64::is_node()`). This should
    // be optimisable into very few CPU instructions similar to 'std::min<>'.
    return p.is_node()
      ? (p._raw & ~(out_idx_mask | ptr_uint64::flag_bit))
      : (p._raw & ~ptr_uint64::flag_bit);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with the out-index changed to the given value.
  ///
  /// \pre `p.is_node() == true`
  //////////////////////////////////////////////////////////////////////////////
  // TODO: remove (used only in test/.../algorithms/nested_sweeping)
  inline ptr_uint64 with_out_idx(const ptr_uint64 &p,
                                 const ptr_uint64::out_idx_type out_idx)
  {
    adiar_assert(p.is_node());

    constexpr ptr_uint64::raw_type out_idx_mask =
      static_cast<ptr_uint64::raw_type>(ptr_uint64::max_out_idx) << ptr_uint64::data_shift;

    return (p._raw & ~out_idx_mask) | (static_cast<ptr_uint64::raw_type>(out_idx) << ptr_uint64::data_shift);
  }

  /* ============================ TERMINAL NODES ============================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of a terminal.
  //////////////////////////////////////////////////////////////////////////////
  // TODO: Deprecate in favour of '~'
  inline ptr_uint64 negate(ptr_uint64 p)
  {
    return ~p;
  }

  /* ============================== CONVERSION ============================== */

  // TODO: Conversion constructor from node
}

#endif // ADIAR_INTERNAL_DATA_TYPES_PTR_H
