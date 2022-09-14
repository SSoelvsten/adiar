#ifndef ADIAR_INTERNAL_DATA_TYPES_NODE_H
#define ADIAR_INTERNAL_DATA_TYPES_NODE_H

#include<adiar/label.h>

#include<adiar/internal/data_types/id.h>
#include<adiar/internal/data_types/uid.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \brief A Decision Diagram node triple (uid, low, high).
  ///
  /// \remark A node contains a unique identifier for said node in `uid`
  ///         together with pointers to its children in `low` and `high`.
  ///
  /// \remark If a node is a terminal, then `low` and `high` are NIL. Otherwise,
  ///         they are always \em not NIL.
  //////////////////////////////////////////////////////////////////////////////
  struct node
  {
    uid_t uid;
    ptr_t low;
    ptr_t high;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc node
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
  /// \brief Whether a node is the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const node_t &n)
  {
    return is_false(n.uid);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a node is the true terminal.
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
}

#endif // ADIAR_INTERNAL_DATA_TYPES_NODE_H
