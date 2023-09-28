#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include <stdexcept>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   A unique identifier a decision diagram node.
  ///
  /// \details This essentially is a *ptr* guaranteed to point to a node, i.e.
  ///          it is \em never nil, and without any associated information,
  ///          e.g. \em without a flag.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Ptr>
  class __uid
    : public Ptr
  {
  public:
    using pointer_type = Ptr;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    __uid() = default;
    __uid(const __uid<pointer_type> &p) = default;
    ~__uid() = default;

  private:
    static pointer_type clean_ptr(const pointer_type &p)
    {
      return p.is_node() ? pointer_type(p.label(), p.id()) : unflag(p);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    __uid(const pointer_type &p) : pointer_type(essential(p))
    {
      adiar_assert(!p.is_nil(), "UID must be created from non-nil value");
    }

    /* ============================= ATTRIBUTES ============================= */
    // Remove anything related to the flag.

    bool is_flagged() = delete;

    /* ================================= nil ================================ */
    // Remove anything related to nil

    static inline constexpr pointer_type nil() = delete;

    bool is_nil() = delete;

    /* ================================ NODES =============================== */
    // Remove anything related to out-index

    typename pointer_type::out_idx_type out_idx() = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the `ptr` for this node uid with the given `out_idx`.
    ////////////////////////////////////////////////////////////////////////////
    inline pointer_type 
    with(const typename pointer_type::out_idx_type out_idx) const
    {
      adiar_assert(pointer_type::is_node());
      return pointer_type(pointer_type::label(), pointer_type::id(), out_idx);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    __uid(const typename pointer_type::label_type label, 
          const typename pointer_type::id_type id)
      : pointer_type(label, id)
    { }

    /* ============================== TERMINALS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    __uid(typename pointer_type::terminal_type v)
      : pointer_type(v)
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    { return pointer_type::is_terminal(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Specialization for the single-integer pointer `ptr_uint64`.

  template<>
  inline ptr_uint64
  __uid<ptr_uint64>::with(const ptr_uint64::out_idx_type out_idx) const
  {
    // Based on the bit-layout, we can do this much faster than the one above.
    constexpr uint64_t out_idx_mask = ~(max_out_idx << pointer_type::flag_bits);
    return ptr_uint64((_raw & out_idx_mask) | pointer_type::encode_out_idx(out_idx));
  }

  template<>
  inline bool
  __uid<ptr_uint64>::is_terminal() const
  {
    // Since uid never is nil, then this is a slightly a faster logic than the
    // one in 'ptr' itself.
    return _raw >= pointer_type::terminal_bit;
  }

  using uid_uint64 = __uid<ptr_uint64>;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
