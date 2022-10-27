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

  private:
    // TODO (ADD (64-bit)):
    //   template with 'uid_t' reexpose it (and its related 'ptr') with typedefs.

    // TODO (Attributed Edges):
    //   Add logic related to flag on children.

    uid_t _uid;
    ptr_uint64 _children[OUTDEGREE];

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    node() = default;
    node(const node &) = default;
    ~node() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct node `(uid, low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const uid_t u, const ptr_uint64 &l, const ptr_uint64 &h)
      : _uid(u), _children{l, h}
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const ptr_uint64 &l, const ptr_uint64 &h)
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
    node(const label_t label, const id_t id, const node &l, const ptr_uint64 &h)
      : node(label, id, l.uid(), h)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const ptr_uint64 &l, const node &h)
      : node(label, id, l, h.uid())
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *internal* node `((label, id), low, high)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const label_t label, const id_t id, const node &l, const node &h)
      : node(label, id, l.uid(), h.uid())
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct *terminal* node `(value, NIL, NIL)`.
    ////////////////////////////////////////////////////////////////////////////
    node(const bool value)
      : _uid(ptr_uint64(value)), _children{ptr_uint64::NIL(), ptr_uint64::NIL()}
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The unique identifier of this node
    ////////////////////////////////////////////////////////////////////////////
    inline uid_t uid() const
    { return _uid; }

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

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the label of a node.
    ///
    /// \todo Rename to `level()` when introducing variable ordering
    ///
    /// \pre `is_terminal()` evaluates to `false`.
    //////////////////////////////////////////////////////////////////////////////
    inline label_t label() const
    {
      adiar_precondition(!is_terminal());
      return uid().label();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level-specific identifier of a node.
    ///
    /// \pre `is_terminal()` evaluates to `false`.
    //////////////////////////////////////////////////////////////////////////////
    inline id_t id() const
    {
      adiar_precondition(!is_terminal());
      return uid().id();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a node is on a given level, i.e. has the given label.
    //////////////////////////////////////////////////////////////////////////////
    inline bool on_level(label_t level) const
    { return uid().on_level(level); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The 'low' child (also known as the 'else' child), i.e. reflecting
    ///        assigning `false` to variable with the 'label'.
    ////////////////////////////////////////////////////////////////////////////
    inline ptr_uint64 low() const
    { return _children[false]; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The 'high' child (also known as the 'then' child), i.e.
    ///        reflecting assigning `true` to variable with the 'label'.
    ////////////////////////////////////////////////////////////////////////////
    inline ptr_uint64 high() const
    { return _children[true]; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc node
  //////////////////////////////////////////////////////////////////////////////
  typedef node node_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the node representing the (locally) negated function:
  ///        pointers to terminal children are negated while pointers to other nodes
  ///        are left unchanged.
  //////////////////////////////////////////////////////////////////////////////
  inline node_t negate(const node_t &n)
  {
    if (n.is_terminal()) {
      return node(negate(n.uid()), ptr_uint64::NIL(), ptr_uint64::NIL());
    }

    const ptr_uint64 low =  n.low().is_terminal()  ? negate(n.low())  : n.low();
    const ptr_uint64 high = n.high().is_terminal() ? negate(n.high()) : n.high();
    return node(n.uid(), low, high);
  }

  inline node operator! (const node &n)
  {
    return negate(n);
  }

  inline bool operator< (const node &a, const node &b)
  {
    return a.uid() < b.uid();
  }

  inline bool operator> (const node &a, const node &b)
  {
    return a.uid() > b.uid();
  }

  inline bool operator== (const node &a, const node &b)
  {
    return a.uid() == b.uid() && a.low() == b.low() && a.high() == b.high();
  }

  inline bool operator!= (const node &a, const node &b)
  {
    return !(a==b);
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_NODE_H
