#ifndef ADIAR_ZDD_ZDD_H
#define ADIAR_ZDD_ZDD_H

#include <string_view>

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/util.h>

namespace adiar {
  // Class declarations to be able to reference it
  class zdd;

  //////////////////////////////////////////////////////////////////////////////
  /// \internal \brief A (possibly non-reduced) Ordered Zero-suppressed Decision
  /// Diagram.
  ///
  /// \relates zdd
  ///
  /// \extends __decision_diagram
  ///
  /// \copydoc __decision_diagram
  //////////////////////////////////////////////////////////////////////////////
  class __zdd : public __decision_diagram {
  public:
    __zdd();
    __zdd(const node_file &f);
    __zdd(const arc_file &f);
    __zdd(const zdd &zdd);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Reduced Ordered Zero-suppressed Decision Diagram.
  ///
  /// \extends decision_diagram
  ///
  /// \copydoc decision_diagram
  //////////////////////////////////////////////////////////////////////////////
  class zdd : public decision_diagram
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend __zdd;

    // |- functions
    friend size_t zdd_nodecount(const zdd&);
    friend label_t zdd_varcount(const zdd&);

    friend bool zdd_subseteq(const zdd&, const zdd&);
    friend bool zdd_disjoint(const zdd &, const zdd &);

  public:
    static constexpr std::string_view false_print = "Ø";
    static constexpr std::string_view true_print = "{Ø}";

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    zdd(const node_file &f, bool negate = false);

    zdd();
    zdd(bool v);

    zdd(const zdd &o);
    zdd(zdd &&o);

    zdd(__zdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    // Accessors overwrite

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 1-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_1level_cut(const cut_type ct) const
    {
      return add_false_cofactor(ct, file._file_ptr->max_1level_cut);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 2-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_2level_cut(const cut_type ct) const
    {
      return add_false_cofactor(ct, file._file_ptr->max_2level_cut);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    zdd& operator= (const zdd &other);
    zdd& operator= (__zdd &&other);

    zdd& operator&= (const zdd &other);
    zdd& operator&= (zdd &&other);

    zdd& operator|= (const zdd &other);
    zdd& operator|= (zdd &&other);

    zdd& operator-= (const zdd &other);
    zdd& operator-= (zdd &&other);

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Accounts for the false arc added due to using a co-factor.
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t add_false_cofactor(const cut_type ct, const cuts_t &ilevel_cuts) const
    {
      const safe_size_t cut_size = ilevel_cuts[ct];

      // Bit-mask (allowing implicit conversion to size_t with bit-operators) to
      // get the cut-type WITHOUT the false arcs.
      constexpr size_t bit_mask = cut_type::INTERNAL_TRUE;
      const cut_type ct_excl_false = static_cast<cut_type>(ct & bit_mask);

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
  };
}

#endif // ADIAR_ZDD_ZDD_H
