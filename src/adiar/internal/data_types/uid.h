#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include <stdexcept>

#include<adiar/internal/assert.h>
#include<adiar/internal/data_types/ptr.h>

namespace adiar {
  /* ========================== UNIQUE IDENTIFIER =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   A unique identifier a decision diagram node.
  ///
  /// \details This essentially is the <tt>ptr</tt> with the guarantee that (a)
  ///          is \em never NIL, and (b) is \em not flagged.
  //////////////////////////////////////////////////////////////////////////////
  class uid : public ptr_uint64
  {
  public:
    typedef ptr_uint64 ptr_t;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    uid() = default;
    uid(const uid &p) = default;
    ~uid() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////
    uid(const ptr_t& p) : ptr_t(unflag(p))
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
    uid(label_t label, id_t id) : ptr_t(label, id)
    { }

    /* ============================== TERMINALS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////
    uid(bool v) : ptr_t(v)
    { }

  public:
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    //////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    {
      // Since uid never is nil, then this is a slightly a faster logic than the
      // one in 'ptr' itself.
      return _raw >= ptr_t::TERMINAL_BIT;
    }
  };

  // TODO: deprecate
  typedef uid uid_t;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
