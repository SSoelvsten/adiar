#ifndef ADIAR_ZDD_ZDD_H
#define ADIAR_ZDD_ZDD_H

#include <string_view>

#include <adiar/internal/cut.h>
#include <adiar/internal/dd.h>

namespace adiar
{
  // Class declarations to be able to reference it
  class zdd;

  //////////////////////////////////////////////////////////////////////////////
  /// \ingroup module__zdd
  ///
  /// \brief A (possibly unreduced) Zero-suppressed Decision Diagram.
  ///
  /// \relates zdd
  ///
  /// \copydetails adiar::internal::__dd
  //////////////////////////////////////////////////////////////////////////////
  class __zdd : public internal::__dd {
  public:
    ////////////////////////////////////////////////////////////////////////////
    __zdd();

    ////////////////////////////////////////////////////////////////////////////
    __zdd(const internal::__dd::shared_nodes_t &f);

    ////////////////////////////////////////////////////////////////////////////
    __zdd(const internal::__dd::shared_arcs_t &f);

    ////////////////////////////////////////////////////////////////////////////
    __zdd(const zdd &zdd);

    // NOTE:
    //   '__dd' class includes 'max_1level_cut' and 'max_2level_cut' operations.
    //   This does not take the potential extra 'false' arc into account.
    //
    //   For now, we do not care to add this, since we are not using `__zdd` in
    //   a context where it is necessary such as 'zdd_union'.
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \ingroup module__zdd
  ///
  /// \brief Reduced Ordered Zero-suppressed Decision Diagram.
  ///
  /// \copydetails adiar::internal::dd
  //////////////////////////////////////////////////////////////////////////////
  class zdd : public internal::dd
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend __zdd;

    // |- functions
    friend size_t zdd_nodecount(const zdd&);
    friend zdd::label_t zdd_varcount(const zdd&);

    friend bool zdd_subseteq(const zdd&, const zdd&);
    friend bool zdd_disjoint(const zdd &, const zdd &);

  public:
    /// \cond
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view false_print = "Ø";

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Text to pretty-print in '.dot' output.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr std::string_view true_print = "{Ø}";
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    ////////////////////////////////////////////////////////////////////////////
    zdd(const internal::dd::shared_nodes_t &f, bool negate = false);

    ////////////////////////////////////////////////////////////////////////////
    zdd();

    ////////////////////////////////////////////////////////////////////////////
    zdd(bool v);

    ////////////////////////////////////////////////////////////////////////////
    zdd(const zdd &o);

    ////////////////////////////////////////////////////////////////////////////
    zdd(zdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    zdd(__zdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    // Accessors overwrite
  public:
    /// \cond
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 1-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    internal::cut_size_t max_1level_cut(const internal::cut_type ct) const
    {
      return add_false_cofactor(ct, file->max_1level_cut);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 2-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    internal::cut_size_t max_2level_cut(const internal::cut_type ct) const
    {
      return add_false_cofactor(ct, file->max_2level_cut);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Accounts for the false arc added due to using a co-factor.
    ////////////////////////////////////////////////////////////////////////////
    internal::cut_size_t add_false_cofactor(const internal::cut_type ct, const internal::cuts_t &ilevel_cuts) const
    {
      const safe_size_t cut_size = ilevel_cuts[ct];

      // Bit-mask (allowing implicit conversion to size_t with bit-operators) to
      // get the cut-type WITHOUT the false arcs.
      constexpr size_t bit_mask = internal::cut_type::Internal_True;
      const internal::cut_type ct_excl_false = static_cast<internal::cut_type>(ct & bit_mask);

      // In product construction algorithms we need to take into account the
      // (single) suppressed false arc, which may suddenly become visible (e.g.
      // 'zdd_union'). Here, the DAG gets stuck inside of a copy of only one of
      // the input ZDDs. To get there, one followed an arc of the input but
      // afterwards one pairs with an invisible false arc that spans all levels.
      //
      // We do not need to account for this invisible false arc in the following
      // two cases
      //
      // - If the requested cut does not include false arcs.
      //
      // - If the cut size is strictly larger than the corresponding cut_type
      //   excluding false. In this case, we already have a false arc to pair
      //   with.
      const size_t add_suppressed = !includes_terminal(ct, false) && cut_size == ilevel_cuts[ct_excl_false];

      return to_size(cut_size + add_suppressed);
    }
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Assigns new `zdd`.
    ////////////////////////////////////////////////////////////////////////////
    zdd& operator= (const zdd &other);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Assigns new `zdd` to a variable; the content is derefenced before
    ///        the given `__zdd` is reduced into a `zdd`.
    ////////////////////////////////////////////////////////////////////////////
    zdd& operator= (__zdd &&other);

    ////////////////////////////////////////////////////////////////////////////
    /// \see zdd_intsec
    ////////////////////////////////////////////////////////////////////////////
    zdd& operator&= (const zdd &other);

    /// \cond
    zdd& operator&= (zdd &&other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////
    /// \see zdd_union
    ////////////////////////////////////////////////////////////////////////////
    zdd& operator|= (const zdd &other);

    /// \cond
    zdd& operator|= (zdd &&other);
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////
    /// \see zdd_diff
    ////////////////////////////////////////////////////////////////////////////
    zdd& operator-= (const zdd &other);

    /// \cond
    zdd& operator-= (zdd &&other);
    /// \endcond
  };
}

#endif // ADIAR_ZDD_ZDD_H
