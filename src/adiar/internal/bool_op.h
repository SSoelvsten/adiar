#ifndef ADIAR_INTERNAL_BOOL_OP_H
#define ADIAR_INTERNAL_BOOL_OP_H

#include <array>
#include <type_traits>

#include <adiar/bool_op.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper of a binary operation with which to precomputes and directly interfaces with
  ///        *pointers*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename BinaryOp>
  class binary_op;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Type inference of `binary_op<BinaryOp>` based on the given operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename BinaryOp>
  binary_op<BinaryOp>
  make_binary_op(const BinaryOp& op)
  {
    return binary_op<BinaryOp>(op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the right, i.e. `op(T,t) == op(F,t)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  can_right_shortcut(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == op(true, terminal);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal shortcuts the operator from the left, i.e. `op(t, T) == op(t, F)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  can_left_shortcut(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == op(terminal, true);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal value is idempotent from the right with respect to an operator,
  ///        i.e. `op(X, t) == X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_idempotent(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == false && op(true, terminal) == true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal value is idempotent from the left with respect to an operator,
  ///        i.e. `op(t, X) == X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_idempotent(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == false && op(terminal, true) == true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other from the right,
  ///        i.e. `op(X, t) == ~X`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_right_negating(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(false, terminal) == true && op(true, terminal) == false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the terminal for this operator negates the value of the other from the left,
  ///        i.e. `op(t, X) == ~X`
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_left_negating(const predicate<bool, bool>& op, const bool terminal)
  {
    return op(terminal, false) == true && op(terminal, true) == false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an operator is commutative, `op(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bool
  is_commutative(const predicate<bool, bool>& op)
  {
    return op(true, false) == op(false, true);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Return the flipped operator, i.e. `op'(X, Y) == op(Y, X)`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline predicate<bool, bool>
  flip(const predicate<bool, bool>& op)
  {
    if (is_commutative(op)) return op;
    return [&op](const bool a, const bool b) -> bool { return op(b, a); };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Specialization for boolean operators, i.e. any `predicate<bool, bool>`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<>
  class binary_op<predicate<bool, bool>>
  {
  private:
    /// \brief Evaluations of predicate
    ///
    /// TODO: Replace with Pointer Type?
    std::array<std::array<bool, 2>, 2> _op;

    /// \brief Pre-computed 'left shortcutting' predicate.
    std::array<bool, 2> _left_shortcutting;

    /// \brief Pre-computed 'right shortcutting' predicate.
    std::array<bool, 2> _right_shortcutting;

    /// \brief Pre-computed 'left idempotent' predicate.
    std::array<bool, 2> _left_idempotent;

    /// \brief Pre-computed 'right idempotent' predicate.
    std::array<bool, 2> _right_idempotent;

    /// \brief Pre-computed 'left negating' predicate.
    std::array<bool, 2> _left_negating;

    /// \brief Pre-computed 'right negating' predicate.
    std::array<bool, 2> _right_negating;

    /// \brief Pre-computed 'commutative' predicate.
    bool _commutative;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construction from `predicate<bool, bool>`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    binary_op(const predicate<bool, bool>& op)
      : _op{{ {{op(false,false), op(false, true)}}, {{op(true,false), op(true,true)}} }}
      , _left_shortcutting{ adiar::internal::can_left_shortcut(op, false), adiar::internal::can_left_shortcut(op, true) }
      , _right_shortcutting{ adiar::internal::can_right_shortcut(op, false), adiar::internal::can_right_shortcut(op, true) }
      , _left_idempotent{ adiar::internal::is_left_idempotent(op, false), adiar::internal::is_left_idempotent(op, true) }
      , _right_idempotent{ adiar::internal::is_right_idempotent(op, false), adiar::internal::is_right_idempotent(op, true) }
      , _left_negating{ adiar::internal::is_left_negating(op, false), adiar::internal::is_left_negating(op, true) }
      , _right_negating{ adiar::internal::is_right_negating(op, false), adiar::internal::is_right_negating(op, true) }
      , _commutative(adiar::internal::is_commutative(op))
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    operator ()(const bool lhs, const bool rhs) const
    {
      return this->_op[lhs][rhs];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename Pointer>
    Pointer
    operator ()(const Pointer& lhs, const Pointer& rhs) const
    {
      static_assert(std::is_same<typename Pointer::terminal_type, bool>::value);
      return (*this)(lhs.value(), rhs.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_left_shortcut(const bool p) const
    {
      return this->_left_shortcutting[p];
    }

    template<typename Pointer>
    bool
    can_left_shortcut(const Pointer& p) const
    {
      return this->can_left_shortcut(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_right_shortcut(const bool p) const
    {
      return this->_right_shortcutting[p];
    }

    template<typename Pointer>
    bool
    can_right_shortcut(const Pointer& p) const
    {
      return this->can_right_shortcut(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_left_idempotent(const bool p) const
    {
      return this->_left_idempotent[p];
    }

    template<typename Pointer>
    bool
    is_left_idempotent(const Pointer& p) const
    {
      return this->is_left_idempotent(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_right_idempotent(const bool p) const
    {
      return this->_right_idempotent[p];
    }

    template<typename Pointer>
    bool
    is_right_idempotent(const Pointer& p) const
    {
      return this->is_right_idempotent(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_left_negating(const bool p) const
    {
      return this->_left_negating[p];
    }

    template<typename Pointer>
    bool
    is_left_negating(const Pointer& p) const
    {
      return this->is_left_negating(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_right_negating(const bool p) const
    {
      return this->_right_negating[p];
    }

    template<typename Pointer>
    bool
    is_right_negating(const Pointer& p) const
    {
      return this->is_right_negating(p.value());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_commutative() const
    {
      return this->_commutative;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using flipped_operator = binary_op<predicate<bool, bool>>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    flipped_operator
    flip() const
    {
      // TODO: Reuse current computations
      const predicate<bool, bool> flipped_op = [this](const bool lhs, const bool rhs) -> bool {
        return this->_op[rhs][lhs];
      };
      return flipped_op;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Extends a commutative operator with 'left-' and 'right-' specific member functions.
  ///
  /// \details This is supposed to be used with the 'Curiously Recurring Template Pattern'
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename BinaryOp>
  class commutative_op
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    static bool
    can_left_shortcut(const T& t)
    {
      return BinaryOp::can_shortcut(t);
    }

    template<typename T>
    static bool
    can_right_shortcut(const T& t)
    {
      return BinaryOp::can_shortcut(t);
    }

    template<typename T>
    static bool
    is_left_idempotent(const T& t)
    {
      return BinaryOp::is_idempotent(t);
    }

    template<typename T>
    static bool
    is_right_idempotent(const T& t)
    {
      return BinaryOp::is_idempotent(t);
    }

    template<typename T>
    static bool
    is_left_negating(const T& t)
    {
      return BinaryOp::is_negating(t);
    }

    template<typename T>
    static bool
    is_right_negating(const T& t)
    {
      return BinaryOp::is_negating(t);
    }

    static constexpr bool
    is_commutative()
    { return true; }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using flipped_operator = BinaryOp;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    flipped_operator
    flip() const
    {
      return {};
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Alternative for `binary_op` hardcoded for a logical 'and'.
  ///
  /// \details The benefit of this over `binary_op` is to skip pre-computations and to expose
  ///          optimisations and simplifications to the compiler.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class and_op
    : public commutative_op<and_op>
  {
    ////////////////////////////////////////////////////////////////////////////////////////////////
  public:
    bool
    operator ()(const bool lhs, const bool rhs) const
    {
      return lhs & rhs;
    }

    template<typename Pointer>
    Pointer
    operator ()(const Pointer& lhs, const Pointer& rhs) const
    {
      return lhs & rhs;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
  protected:
    friend commutative_op<and_op>;

    static bool
    can_shortcut(const bool p)
    {
      return !p;
    }

    template<typename Pointer>
    static bool
    can_shortcut(const Pointer& p)
    {
      return can_shortcut(p.value());
    }

    static bool
    is_idempotent(const bool p)
    {
      return p;
    }

    template<typename Pointer>
    static bool
    is_idempotent(const Pointer& p)
    {
      return is_idempotent(p.value());
    }

    template<typename T>
    static constexpr bool
    is_negating(const T&/*t*/)
    {
      return false;
    }
  };
}

#endif // ADIAR_INTERNAL_BOOL_OP_H
