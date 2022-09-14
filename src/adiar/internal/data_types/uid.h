#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include<adiar/label.h>

#include<adiar/internal/assert.h>
#include<adiar/internal/data_types/id.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// To condense almost everything down to mere integer comparisons we reserve
  /// specific parts of a single 64 bit unsigned integer to different variables.
  ///
  ///   | S | ???????????????????????????????????????????????????? | F |
  ///
  /// Where these three parts represent the following variables:
  ///
  ///  - S : the is_terminal flag. If the terminal flag is set, the L and I areas differ
  ///        (see below for the terminal type description).
  ///
  ///  - ? : The layout of these 62 bits change based on whether it describes a
  ///        terminal, an internal node, or NIL.
  ///
  ///  - F : A boolean flag. This is currently only used in arcs to identify
  ///        high and low arcs (see below).
  ///
  /// An important fact is, that the typedef of ptr and uid below merely are
  /// aliases for the unsigned 64 bit integer. They are merely supposed to
  /// support code readability, but the type checker does actually not care.
  ///
  /// We ensure, that the S and ? areas combined uniquely identify all terminals and
  /// nodes. We also notice, that sorting these pointers directly enforce terminal
  /// pointers are sorted after nodes. Finally, two pointers for the same uid
  /// will finally be sorted by the flag.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
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
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// When the terminal flag is set, then we interpret the middle bits as the
  /// value of the terminal.
  ///
  ///     | S | VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV | F |
  ///
  /// Notice, that this means we will never have to actually visit to retrieve
  /// its value. That is, the only time a terminal has to be explicitly represented
  /// as a node is when the BDD only consists of said terminal.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The bit representation of a terminal.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint64_t TERMINAL_BIT = 0x8000000000000000ull;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   A unique identifier (label, id) of an internal node.
  ///
  /// \details An internal node is identified by the tuple (label, id) where
  ///          unique identifiers are first sorted by their label and
  ///          secondarily by their level-identifier.
  ///
  /// \remark  If anything has this type rather than <tt>ptr_t</tt> then it is a
  ///          guarantee that (a) it is \em never NIL, and (b) is \em not
  ///          flagged.
  //////////////////////////////////////////////////////////////////////////////
  typedef uint64_t uid_t;

  /* =============================== POINTERS =============================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A (possibly flagged) identifier of a terminal, an internal node
  ///        (uid_t), or nothing (NIL).
  ///
  /// \remark The layout of a pointer is such, that unique identifiers precede
  ///         terminals which in turn precede NIL. The ordering on unique
  ///         identifiers and terminals are lifted to pointers.
  ///
  /// \remark A pointer may be flagged. For an arc's source this marks the arc
  ///         being a 'high' rather than a 'low' arc.
  //////////////////////////////////////////////////////////////////////////////
  typedef uint64_t ptr_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The bit representation of the high/low flag.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint64_t FLAG_BIT = 0x0000000000000001ull;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Whether the pointer is flagged.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_flagged(ptr_t p)
  {
    return p & FLAG_BIT;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to true.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_t flag(ptr_t p)
  {
    return p | FLAG_BIT;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer with its flag set to false.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_t unflag(ptr_t p)
  {
    return p & (~FLAG_BIT);
  }

  /* ================================== NIL ================================= */
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   A pointer to nothing.
  ///
  /// \details Due to how we create the identifiers for all nodes and terminals, we
  ///          cannot use the common null with value 0. So, instead we provide a
  ///          special value that works with this specific setup.
  ///
  /// \remark  A NIL value always comes after all other types of pointers.
  //////////////////////////////////////////////////////////////////////////////
  constexpr ptr_t NIL = UINT64_MAX - 1; // i.e. unflag(UINT64_MAX);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is NIL.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_nil(ptr_t p)
  {
    // Check for flagged and unflagged NIL
    return p >= NIL;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for an internal node, uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_node(ptr_t p)
  {
    return p <= ~TERMINAL_BIT;
  }

  /* ============================ INTERNAL NODES ============================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline uid_t create_node_uid(label_t label, id_t id)
  {
    adiar_debug(label <= MAX_LABEL, "Cannot represent given label");
    adiar_debug(id <= MAX_ID, "Cannot represent given id");

    return ((uint64_t) label << (ID_BITS + 1)) + (id << 1);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to the uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_t create_node_ptr(label_t label, id_t id)
  {
    return create_node_uid(label, id);
  }

  inline ptr_t create_node_ptr(uid_t uid)
  {
    return uid; //???
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from a uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline label_t label_of(ptr_t p)
  {
    return p >> (ID_BITS + 1);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the level identifier from a uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline id_t id_of(ptr_t p)
  {
    return (p >> 1) & MAX_ID;
  }

  /* ============================ TERMINAL NODES ============================ */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(ptr_t p)
  {
    return !is_nil(p) && p >= TERMINAL_BIT;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the unique identifier for a terminal with the given value.
  //////////////////////////////////////////////////////////////////////////////
  inline uid_t create_terminal_uid(bool v)
  {
    return TERMINAL_BIT + (v << 1);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to a terminal with the given value.
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_t create_terminal_ptr(bool v)
  {
    return create_terminal_uid(v);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(ptr_t p)
  {
    adiar_debug(is_terminal(p), "Cannot extract value of non-terminal");

    return (p & ~TERMINAL_BIT) >> 1;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(ptr_t p)
  {
    return is_terminal(p) && !value_of(p);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(ptr_t p)
  {
    return is_terminal(p) && value_of(p);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of a terminal.
  //////////////////////////////////////////////////////////////////////////////
  uid_t negate(uid_t u);

  inline ptr_t negate(ptr_t n)
  {
    adiar_debug(is_terminal(n), "Cannot negate non-terminal");

    return 2u ^ n;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a node on a given level.
  //////////////////////////////////////////////////////////////////////////////
  inline bool on_level(ptr_t p, label_t level)
  {
    return is_terminal(p) ? false : label_of(p) == level;
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
