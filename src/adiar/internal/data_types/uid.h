#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include <stdexcept>

#include<adiar/internal/assert.h>
#include<adiar/internal/data_types/ptr.h>

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

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    __uid(const ptr_t &p) : ptr_t(unflag(p))
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
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    //////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    { return ptr_t::is_terminal(); }
  };

  template<>
  inline bool __uid<ptr_uint64>::is_terminal() const
  {
    // Since uid never is nil, then this is a slightly a faster logic than the
    // one in 'ptr' itself.
    return _raw >= ptr_t::TERMINAL_BIT;
  }

  typedef __uid<ptr_uint64> uid_uint64;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
