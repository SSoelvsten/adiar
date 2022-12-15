#ifndef ADIAR_DATA_H
#define ADIAR_DATA_H

#include <stdint.h>
#include <functional>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <adiar/internal/assert.h>

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
  /// \brief The number of bits for a label.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint8_t  LABEL_BITS = 24;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal possible value for a unique identifier's label.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint64_t max_var  = (1ull << LABEL_BITS) - 1;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of bits for a unique identifier's id.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint8_t  ID_BITS = 64 - 2 - LABEL_BITS;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal possible value for a unique identifier's id.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint64_t MAX_ID  = (1ull << ID_BITS) - 1;

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
  /// \brief The bit representation of the high/low flag.
  //////////////////////////////////////////////////////////////////////////////
  constexpr uint64_t FLAG_BIT = 0x0000000000000001ull;

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
  constexpr ptr_t NIL = UINT64_MAX - 1; // e.g. unflag(UINT64_MAX);

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

  /* ================ UNIQUE IDENTIFIERS : INTERNAL NODES =================== */

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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Variable label.
  //////////////////////////////////////////////////////////////////////////////
  typedef uint32_t label_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Level identifier.
  //////////////////////////////////////////////////////////////////////////////
  typedef uint64_t id_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  inline uid_t create_node_uid(label_t label, id_t id)
  {
    adiar_debug(label <= max_var, "Cannot represent given label");
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

  /* ================ UNIQUE IDENTIFIERS : TERMINAL NODES==================== */

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
  /// \brief   Computes the unique identifier of the resulting terminal based on the
  ///          pointers to two terminals.
  ///
  /// \details By abusing our knowledge of the bit-layout, we can implement all
  ///          common operations merely as a few fast bit operations.
  ///
  /// \remark  For each operator, we provide the truth table
  ///          [(1,1), (1,0), (0,1), (0,0)].
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<uid_t(ptr_t,ptr_t)> bool_op;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'and' operator, i.e. the truth table: [1,0,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op and_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return unflag(terminal1 & terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not and' operator, i.e. the truth table: [0,1,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nand_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return negate(and_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'or' operator, i.e. the truth table: [1,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op or_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return unflag(terminal1 | terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'not or' operator, i.e. the truth table: [0,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op nor_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return negate(or_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [0,1,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xor_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return TERMINAL_BIT | unflag(terminal1 ^ terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator, i.e. the truth table: [1,0,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op xnor_op  = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return negate(xor_op(terminal1, terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,0,1,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op imp_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return create_terminal_ptr(unflag(terminal1) <= unflag(terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator, i.e. the truth table: [1,1,0,1].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op invimp_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return create_terminal_ptr(unflag(terminal2) <= unflag(terminal1));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'equivalence' operator, i.e. the 'xnor' operator
  //////////////////////////////////////////////////////////////////////////////
  const bool_op equiv_op = xnor_op;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'set difference' operator, i.e. the truth table [0,1,0,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op diff_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return and_op(terminal1, negate(terminal2));
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less' operator, i.e. the truth table [0,0,1,0].
  //////////////////////////////////////////////////////////////////////////////
  const bool_op less_op = [](ptr_t terminal1, ptr_t terminal2) -> ptr_t
  {
    return and_op(negate(terminal1), terminal2);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right,
  ///        i.e. op(T,terminal) = op(F,terminal).
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_right_shortcut(const bool_op &op, const ptr_t terminal)
  {
    return op(create_terminal_ptr(false), terminal) == op(create_terminal_ptr(true), terminal);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left,
  ///        i.e. op(terminal, T) = op(terminal, F).
  //////////////////////////////////////////////////////////////////////////////
  inline bool can_left_shortcut(const bool_op &op, const ptr_t terminal)
  {
    return op(terminal, create_terminal_ptr(false)) == op(terminal, create_terminal_ptr(true));
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the right,
  ///        i.e. op(X, terminal) = X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_irrelevant(const bool_op &op, const ptr_t terminal)
  {
    return op(create_terminal_ptr(false), terminal) == create_terminal_ptr(false)
      && op(create_terminal_ptr(true), terminal) == create_terminal_ptr(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal is irrelevant for the operator from the left,
  ///        i.e. op(terminal, X) = X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_irrelevant(const bool_op &op, const ptr_t terminal)
  {
    return op(terminal, create_terminal_ptr(false)) == create_terminal_ptr(false)
      && op(terminal, create_terminal_ptr(true)) == create_terminal_ptr(true);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the right, i.e. op(X, terminal) = ~X.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_right_negating(const bool_op &op, const ptr_t terminal)
  {
    return op(terminal, create_terminal_ptr(false)) == create_terminal_ptr(true)
      && op(terminal, create_terminal_ptr(true)) == create_terminal_ptr(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other
  ///        from the left, i.e. op(terminal, X) = ~X
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_left_negating(const bool_op &op, const ptr_t terminal)
  {
    return op(terminal, create_terminal_ptr(false)) == create_terminal_ptr(true)
      && op(terminal, create_terminal_ptr(true)) == create_terminal_ptr(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, op(X, Y) = op(Y, X).
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_commutative(const bool_op &op)
  {
    ptr_t terminal_T = create_terminal_ptr(true);
    ptr_t terminal_F = create_terminal_ptr(false);

    return op(terminal_T, terminal_F) == op(terminal_F, terminal_T);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a node on a given level.
  //////////////////////////////////////////////////////////////////////////////
  inline bool on_level(ptr_t p, label_t level)
  {
    return is_terminal(p) ? false : label_of(p) == level;
  }

  /* ================================ NODES ================================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A Decision Diagram node triple (uid, low, high).
  ///
  /// \remark A node contains a unique identifier for said node in <tt>uid</tt>
  ///         together with pointers to its children in <tt>low</tt> and
  ///         <tt>high</tt>.
  ///
  /// \remark If a node is a terminal, then <tt>low</tt> and <tt>high</tt> are NIL.
  ///         Otherwise, they are always \em not NIL.
  //////////////////////////////////////////////////////////////////////////////
  struct node
  {
    uid_t uid;
    ptr_t low;
    ptr_t high;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc
  //////////////////////////////////////////////////////////////////////////////
  typedef node node_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a terminal node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  inline node_t create_terminal(bool value)
  {
    return { create_terminal_ptr(value) , NIL, NIL };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given node represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(const node_t& n)
  {
    return n.uid >= TERMINAL_BIT;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const node_t &n)
  {
    adiar_debug(is_terminal(n), "Cannot extract value from non-terminal");

    return value_of(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const node_t &n)
  {
    return is_false(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(const node_t &n)
  {
    return is_true(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given unique identifier (label, id) and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  inline node_t create_node(uid_t uid, ptr_t low, ptr_t high)
  {
    return { uid, low, high };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given its label, its level identifier, and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  inline node_t create_node(label_t label, id_t id, ptr_t low, ptr_t high)
  {
    // TODO: Should these be adiar_assert instead to check validity of user input?
    adiar_debug(!is_nil(low), "Cannot create a node with NIL child");
    adiar_debug(is_terminal(low) || label < label_of(low),
                "Node is not prior to given low child");

    adiar_debug(!is_nil(high), "Cannot create a node with NIL child");
    adiar_debug(is_terminal(high) || label < label_of(high),
                "Node is not prior to given high child");

    return create_node(create_node_uid(label, id), low, high);
  }

  inline node_t create_node(label_t label, id_t id, ptr_t low, node_t high)
  {
    return create_node(label, id, low, high.uid);
  }

  inline node_t create_node(label_t label, id_t id, node_t low, ptr_t high)
  {
    return create_node(label, id, low.uid, high);
  }

  inline node_t create_node(label_t label, id_t id, node_t low, node_t high)
  {
    return create_node(label, id, low.uid, high.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from a node.
  //////////////////////////////////////////////////////////////////////////////
  inline label_t label_of(const node_t &n)
  {
    adiar_debug(!is_terminal(n), "Cannot extract label of a terminal");

    return label_of(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the level identifier of a node.
  //////////////////////////////////////////////////////////////////////////////
  inline id_t id_of(const node_t &n)
  {
    adiar_debug(!is_terminal(n), "Cannot extract id of a terminal");

    return id_of(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a node is on a given level, i.e. has the given label.
  //////////////////////////////////////////////////////////////////////////////
  inline bool on_level(const node_t &n, label_t level)
  {
    return on_level(n.uid, level);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the node representing the (locally) negated function:
  ///        pointers to terminal children are negated while pointers to other nodes
  ///        are left unchanged.
  //////////////////////////////////////////////////////////////////////////////
  inline node_t negate(const node_t &n)
  {
    if (is_terminal(n)) {
      return { negate(n.uid), NIL, NIL };
    }

    uint64_t low =  is_terminal(n.low)  ? negate(n.low)  : n.low;
    uint64_t high = is_terminal(n.high) ? negate(n.high) : n.high;
    return { n.uid, low, high };
  }

  inline node operator! (const node &n)
  {
    return negate(n);
  }

  inline bool operator< (const node &a, const node &b)
  {
    return a.uid < b.uid;
  }

  inline bool operator> (const node &a, const node &b)
  {
    return a.uid > b.uid;
  }

  inline bool operator== (const node &a, const node &b)
  {
    return a.uid == b.uid && a.low == b.low && a.high == b.high;
  }

  inline bool operator!= (const node &a, const node &b)
  {
    return !(a==b);
  }

  /* ================================= ARCS ================================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief An arc from some source to a target.
  ///
  /// \details An arc contains a value for the source and one for the target.
  ///          Notice, that we don't specifically have a member for is_high. The
  ///          reason for this choice is that the C++ compiler makes everything
  ///          word-aligned. That means with an explicit is_high member it would
  ///          take up 3 x 64 bits rather than only 2 x 64 bits.
  ///
  /// \remark  If <tt>source</tt> is flagged, then this is a high arc rather than
  ///          a low arc.
  ///
  /// \remark  <tt>source</tt> may be NIL
  //////////////////////////////////////////////////////////////////////////////
  struct arc
  {
    ptr_t source;
    ptr_t target;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc
  //////////////////////////////////////////////////////////////////////////////
  typedef arc arc_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an arc is marked as the high from its source.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_high(const arc_t &a)
  {
    return is_flagged(a.source);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negates the target value, if it is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t negate(const arc_t &a)
  {
    uint64_t target = is_terminal(a.target) ? negate(a.target) : a.target;
    return { a.source, target };
  }

  inline arc_t operator! (const arc &a)
  {
    return negate(a);
  }

  inline bool operator== (const arc &a, const arc &b)
  {
    return a.source == b.source && a.target == b.target;
  }

  inline bool operator!= (const arc &a, const arc &b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate: First on unique identifier of the source, and
  ///        secondly on whether it is the high arc.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_source_lt
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.source < b.source;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate on the target.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_lt
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.target < b.target
#ifndef NDEBUG
        || (a.target == b.target && a.source < b.source)
#endif
        ;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extracts the label from the target of an arc
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_label
  {
    static inline label_t label_of(const arc_t& a)
    {
      return adiar::label_of(a.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t low_arc_of(const node_t &n)
  {
    return { n.uid, n.low };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the high arc from a node
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t high_arc_of(const node_t &n)
  {
    return { flag(n.uid), n.high };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief      Combine two arcs with the same source into a node.
  ///
  /// \param low  The low arc
  ///
  /// \param high The high arc
  //////////////////////////////////////////////////////////////////////////////
  inline node_t node_of(const arc_t &low, const arc_t &high)
  {
    adiar_debug(unflag(low.source) == unflag(high.source), "Arcs are not of the same node");
    adiar_debug(!is_high(low), "High flag set on low child");
    adiar_debug(is_high(high), "High flag not set on high child");

    return { low.source, low.target, high.target };
  }

  /* ============================== ASSIGNMENTS ============================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief An assignment [label -> value] to a variable with the given label to
  ///        which is assigned the given value.
  //////////////////////////////////////////////////////////////////////////////
  struct assignment {
    label_t label;
    bool value;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc assignment
  //////////////////////////////////////////////////////////////////////////////
  typedef assignment assignment_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline assignment_t create_assignment(label_t label, bool value)
  {
    adiar_debug(label <= max_var, "Cannot represent that large a label");

    return { label, value };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline label_t label_of(const assignment_t &a)
  {
    return a.label;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const assignment_t &a)
  {
    return a.value;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline assignment operator! (const assignment &a)
  {
    return { a.label, !a.value };
  }

  inline bool operator< (const assignment &a, const assignment &b)
  {
    return a.label < b.label;
  }

  inline bool operator> (const assignment &a, const assignment &b)
  {
    return a.label > b.label;
  }

  inline bool operator== (const assignment &a, const assignment &b)
  {
    return a.label == b.label && a.value == b.value;
  }

  inline bool operator!= (const assignment &a, const assignment &b)
  {
    return !(a==b);
  }

  /* ======================== LEVEL META INFORMATION ======================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Meta information on a single level in a decision diagram.
  ///
  /// \details Several of our algorithms and data structures exploit some meta
  ///          information to improve their performance.
  //////////////////////////////////////////////////////////////////////////////
  struct level_info
  {
    label_t label;
    size_t width;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc
  //////////////////////////////////////////////////////////////////////////////
  typedef level_info level_info_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Combine all the meta information for a single level
  //////////////////////////////////////////////////////////////////////////////
  inline level_info_t create_level_info(label_t label, size_t level_width)
  {
    return { label, level_width };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the variable label of a specific level.
  //////////////////////////////////////////////////////////////////////////////
  inline label_t label_of(const level_info_t &m)
  {
    return m.label;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the number of nodes on a specific level.
  //////////////////////////////////////////////////////////////////////////////
  inline size_t width_of(const level_info_t &m)
  {
    return m.width;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \internal This is only due to some file_stream requires the existence of a
  /// ! operator.
  //////////////////////////////////////////////////////////////////////////////
  inline level_info operator! (const level_info &m)
  {
    return m;
  }

  inline bool operator== (const level_info &a, const level_info &b)
  {
    return a.label == b.label && a.width == b.width;
  }

  inline bool operator!= (const level_info &a, const level_info &b)
  {
    return !(a==b);
  }
}

#endif // ADIAR_DATA_H
