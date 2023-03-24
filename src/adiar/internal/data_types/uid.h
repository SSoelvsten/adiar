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
  ///          it is \em never NIL, and without any associated information,
  ///          e.g. \em without a flag.
  //////////////////////////////////////////////////////////////////////////////
  template<typename ptr_type>
  class __uid : public ptr_type
  {
  public:
    typedef ptr_type ptr_t;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    __uid() = default;
    __uid(const __uid<ptr_t> &p) = default;
    ~__uid() = default;

  private:
    static ptr_t clean_ptr(const ptr_t &p)
    {
      return p.is_node() ? ptr_t(p.label(), p.id()) : unflag(p);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    __uid(const ptr_t &p) : ptr_t(essential(p))
    {
#ifndef NDEBUG
      if (p.is_nil()) throw std::invalid_argument("Pointer must be non-NIL");
#endif
    }

    /* ============================= ATTRIBUTES ============================= */
    // Remove anything related to the flag.

    bool is_flagged() = delete;

    /* ================================= NIL ================================ */
    // Remove anything related to NIL

    static inline constexpr ptr_t NIL() = delete;

    bool is_nil() = delete;

    /* ================================ NODES =============================== */
    // Remove anything related to out-index

    typename ptr_t::out_idx_t out_idx() = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the `ptr` for this node uid with the given `out_idx`.
    ////////////////////////////////////////////////////////////////////////////
    inline ptr_t with(const typename ptr_t::out_idx_t out_idx) const
    {
      adiar_precondition(ptr_t::is_node());
      return ptr_t(ptr_t::label(), ptr_t::id(), out_idx);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    __uid(const typename ptr_t::label_t label, const typename ptr_t::id_t id)
      : ptr_t(label, id)
    { }

    /* ============================== TERMINALS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    __uid(typename ptr_t::value_t v)
      : ptr_t(v)
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    { return ptr_t::is_terminal(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Specialization for the single-integer pointer `ptr_uint64`.

  template<>
  inline ptr_uint64
  __uid<ptr_uint64>::with(const ptr_uint64::out_idx_t out_idx) const
  {
    // Based on the bit-layout, we can do this much faster than the one above.
    constexpr uint64_t out_idx_mask = ~(MAX_OUT_IDX << ptr_t::FLAG_BITS);
    return ptr_uint64((_raw & out_idx_mask) | ptr_t::encode_out_idx(out_idx));
  }

  template<>
  inline bool
  __uid<ptr_uint64>::is_terminal() const
  {
    // Since uid never is nil, then this is a slightly a faster logic than the
    // one in 'ptr' itself.
    return _raw >= ptr_t::TERMINAL_BIT;
  }

  using uid_uint64 = __uid<ptr_uint64>;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
