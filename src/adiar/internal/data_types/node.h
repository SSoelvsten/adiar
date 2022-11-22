#ifndef ADIAR_INTERNAL_DATA_TYPES_NODE_H
#define ADIAR_INTERNAL_DATA_TYPES_NODE_H

#include<adiar/internal/data_types/ptr.h>
#include<adiar/internal/data_types/uid.h>
#include<adiar/internal/data_types/tuple.h>

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
  class node
  {
  public:
    // TODO (MDD):
    //   change (with templating) the _children array below to be a std::vector.
    //   To get it to work with TPIE, use the serialization stream instead.

    // TODO (QMDD):
    //   template with 'outdegree' and use it with the value of '4u'. Reexpose
    //   the templated variable in the constexpr below, such that algorithms
    //   also can retrieve it.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of children of this node type.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t OUTDEGREE = 2u;

    // TODO (ADD (64-bit)):
    //   template with 'uid_t' reexpose it (and its related 'ptr') with typedefs.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the pointer of this node's children.
    ////////////////////////////////////////////////////////////////////////////
    typedef ptr_uint64 ptr_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's unique identifier.
    ////////////////////////////////////////////////////////////////////////////
    typedef __uid<ptr_t> uid_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    typedef ptr_t::label_t label_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr label_t MAX_LABEL = ptr_t::MAX_LABEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    typedef ptr_t::id_t id_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr id_t MAX_ID = ptr_t::MAX_ID;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the children tuple.
    ////////////////////////////////////////////////////////////////////////////
    typedef tuple<ptr_t, OUTDEGREE> children_t;

  private:
    // TODO (Attributed Edges):
    //   Add logic related to flag on children.

    uid_t _uid;
    children_t _children;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    node() = default;
    node(const node &) = default;
    ~node() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The unique identifier of this node
    ////////////////////////////////////////////////////////////////////////////
    inline uid_t uid() const
    { return _uid; }

    /* ============================== TERMINAL NODE ========================= */
  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *terminal* node `(value, NIL, NIL)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const bool value)
      : _uid(ptr_t(value)), _children(ptr_uint64::NIL(), ptr_uint64::NIL())
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this node represents a terminal value.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_terminal() const
    { return _uid.is_terminal(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The value of this terminal node (assuming it is one).
    ///
    /// \pre `is_terminal()` evaluates to `true`.
    ////////////////////////////////////////////////////////////////////////////
    inline bool value() const
    {
      adiar_precondition(is_terminal());
      return _uid.value();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this node is the false terminal.
    ///
    /// \details This is equivalent to `n.is_terminal() && !n.value()`.
    //////////////////////////////////////////////////////////////////////////////
    inline bool is_false() const
    { return uid().is_false(); }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this node is the true terminal.
    ///
    /// \details This is equivalent to `n.is_terminal() && n.value()`.
    //////////////////////////////////////////////////////////////////////////////
    inline bool is_true() const
    { return uid().is_true(); }

    /* ============================== INTERNAL NODE ========================= */
  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.

    // TODO (QMDD):
    //   Add generic version of the two constructors below that takes a
    //   'children_t' as argument.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct node `(uid, low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const uid_t u, const ptr_t &l, const ptr_t &h)
      : _uid(u), _children{l, h}
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const ptr_t &l, const ptr_t &h)
      : _uid(label, id), _children{l, h}
    {
      adiar_debug(!l.is_nil(), "Cannot create a node with NIL child");
      adiar_debug(l.is_terminal() || label < l.label(),
                  "Node is not prior to given low child");

      adiar_debug(!h.is_nil(), "Cannot create a node with NIL child");
      adiar_debug(h.is_terminal() || label < h.label(),
                  "Node is not prior to given high child");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const node &l, const ptr_t &h)
      : node(label, id, l.uid(), h)
    {
      adiar_debug(OUTDEGREE == 2, "Constructor is for binary node only.");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const ptr_t &l, const node &h)
      : node(label, id, l, h.uid())
    {
      adiar_debug(OUTDEGREE == 2, "Constructor is for binary node only.");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const node &l, const node &h)
      : node(label, id, l.uid(), h.uid())
    {
      adiar_debug(OUTDEGREE == 2, "Constructor is for binary node only.");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the label of a node.
    ///
    /// \todo Rename to `level()` when introducing variable ordering
    ///
    /// \pre `is_terminal()` evaluates to `false`.
    ////////////////////////////////////////////////////////////////////////////
    inline label_t label() const
    {
      adiar_precondition(!is_terminal());
      return uid().label();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level-specific identifier of a node.
    ///
    /// \pre `is_terminal()` evaluates to `false`.
    ////////////////////////////////////////////////////////////////////////////
    inline id_t id() const
    {
      adiar_precondition(!is_terminal());
      return uid().id();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a node is on a given level, i.e. has the given label.
    ////////////////////////////////////////////////////////////////////////////
    inline bool on_level(label_t level) const
    { return uid().on_level(level); }

    /* ================================= CHILDREN =========================== */
  public:
    // TODO: indexable retrieval of children.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The node's children sorted based on the semantics of this node.
    ////////////////////////////////////////////////////////////////////////////
    inline const children_t& children() const
    { return _children; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The 'low' child (also known as the 'else' child), i.e. reflecting
    ///        assigning `false` to variable with the 'label'.
    ////////////////////////////////////////////////////////////////////////////
    inline ptr_uint64 low() const
    {
      adiar_debug(OUTDEGREE == 2,
                  "Semantics of 'low' is only defined for binary nodes.");

      return _children[false];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The 'high' child (also known as the 'then' child), i.e.
    ///        reflecting assigning `true` to variable with the 'label'.
    ////////////////////////////////////////////////////////////////////////////
    inline ptr_uint64 high() const
    {
      adiar_debug(OUTDEGREE == 2,
                  "Semantics of 'high' is only defined for binary node.");

      return _children[true];
    }

    /* =============================== COMPARATORS ========================== */
  public:
    inline bool operator< (const node &o) const
    { return this->_uid < o._uid; }

    inline bool operator> (const node &o) const
    { return (o < *this); }

    inline bool operator== (const node &o) const
    {
      return this->_uid == o._uid
        && this->_children[0] == o._children[0]
        && this->_children[1] == o._children[1];
    }

    inline bool operator!= (const node &o) const
    { return !(*this == o); }

    /* ================================ OPERATORS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create the node representing the (locally) negated function:
    ///        pointers to terminal children are negated while pointers to other
    ///        nodes are left unchanged.
    ////////////////////////////////////////////////////////////////////////////
    node operator~ () const
    {
      if (this->is_terminal()) {
        return node(~this->_uid, ptr_t::NIL(), ptr_t::NIL());
      }

      const ptr_t low  = this->_children[0].is_terminal()
        ? ~this->_children[0]
        :  this->_children[0];

      const ptr_t high = this->_children[1].is_terminal()
        ? ~this->_children[1]
        :  this->_children[1];

      return node(this->_uid, low, high);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc node::operator~
  //////////////////////////////////////////////////////////////////////////////
  inline node negate(const node &n)
  { return ~n; }

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc node::operator~
  //////////////////////////////////////////////////////////////////////////////
  inline node operator! (const node &n)
  { return ~n; }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_NODE_H
