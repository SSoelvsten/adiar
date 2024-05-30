#ifndef ADIAR_BDD_BDD_H
#define ADIAR_BDD_BDD_H

#include <string_view>

#include <adiar/internal/dd.h>

namespace adiar
{
  class bdd;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \ingroup module__bdd
  ///
  /// \brief A (possibly) unreduced Binary Decision Diagram.
  ///
  /// \relates bdd
  ///
  /// \copydetails adiar::internal::__dd
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class __bdd : public internal::__dd
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor with an empty result.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __bdd();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Wrapper for an algorithm's already reduced output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __bdd(const shared_node_file_type& f);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Wrapper for an algorithm's unreduced output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __bdd(const shared_arc_file_type& f, const exec_policy& ep);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion constructor from a `bdd` to pass along a prior value.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __bdd(const bdd& bdd);
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \ingroup module__bdd
  ///
  /// \brief A reduced Binary Decision Diagram.
  ///
  /// \copydetails adiar::internal::dd
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class bdd : public internal::dd
  {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes [public]
    friend __bdd;

    // |- classes [internal]
    template <typename BinaryOp>
    friend class apply_prod2_policy;

    // |- functions
    friend bdd
    bdd_not(const bdd&);
    friend bdd
    bdd_not(bdd&&);

    friend size_t
    bdd_nodecount(const bdd&);

    friend typename bdd::label_type
    bdd_varcount(const bdd&);

    friend __bdd
    bdd_ite(const exec_policy& ep, const bdd& f, const bdd& g, const bdd& h);

  public:
    /// \cond
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view false_print = "&perp;";

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view true_print = "&#x22A4;"; // &top;
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Default construction, creating the false terminal.
    ///
    /// \see bdd_false
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Implicit conversion from a terminal value to construct the
    ///        `false` and `true` terminals.
    ///
    /// \see bdd_terminal, bdd_false, bdd_true
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd(bdd::terminal_type t);

    /// \cond
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor to wrap the node-based result of an algorithm.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd(const bdd::shared_node_file_type& f, bool negate = false);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy construction, incrementing the reference count on the file underneath.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd(const bdd& f);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move construction, taking over ownership of the files underneath.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd(bdd&& f);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Implicit move conversion from a possibly to-be reduced result from an algorithm to
    ///          a `bdd`.
    ///
    /// \details Since the `adiar::internal::reduce` algorithm is run as part of this constructor,
    ///          the scoping rules ensure we garbage collect irrelevant files as early as possible.
    ///
    /// \remark  Since the value `o` is forced to be moved, we force the content of `o` to be
    ///          destructed after finishing the *Reduce* algorithm.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd(__bdd&& f);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Assigns new `bdd`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator=(const bdd& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Assigns new `bdd` to a variable; the content is derefenced before the given `__bdd`
    ///        is reduced into a `bdd`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator=(__bdd&& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \see bdd_and
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator&=(const bdd& other);

    /// \cond
    bdd&
    operator&=(bdd&& other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \see bdd_or
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator|=(const bdd& other);

    /// \cond
    bdd&
    operator|=(bdd&& other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \see bdd_xor
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator^=(const bdd& other);

    /// \cond
    bdd&
    operator^=(bdd&& other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \see bdd_or
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator+=(const bdd& other);

    /// \cond
    bdd&
    operator+=(bdd&& other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \see bdd_diff
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bdd&
    operator-=(const bdd& other);

    /// \cond
    bdd&
    operator-=(bdd&& other);
    /// \endcond
  };
}

#endif // ADIAR_BDD_BDD_H
