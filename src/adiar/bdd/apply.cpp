#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/unreachable.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // BDD product construction policy
  class apply_prod2_policy
    : public bdd_policy
    , public internal::prod2_mixed_level_merger<bdd_policy>
  {
  public:
    static __bdd
    resolve_same_file(const bdd& bdd_1, const bdd& bdd_2, const bool_op& op)
    {
      // Compute the results on all children.
      const bool op_F = op(bdd_1.negate, bdd_2.negate);
      const bool op_T = op(!bdd_1.negate, !bdd_2.negate);

      // Does it collapse to a terminal?
      if (op_F == op_T) { return bdd_terminal(op_F); }

      return op_F == bdd_1.negate ? bdd_1 : ~bdd_1;
    }

  public:
    static __bdd
    resolve_terminal_root(const bdd& bdd_1, const bdd& bdd_2, const bool_op& op)
    {
      adiar_assert(bdd_isterminal(bdd_1) || bdd_isterminal(bdd_2));

      if (bdd_isterminal(bdd_1) && bdd_isterminal(bdd_2)) {
        const bool p1 = dd_valueof(bdd_1);
        const bool p2 = dd_valueof(bdd_2);

        return bdd_terminal(op(p1, p2));
      } else if (bdd_isterminal(bdd_1)) {
        const bool p1 = dd_valueof(bdd_1);

        if (internal::can_left_shortcut(op, p1)) { return bdd_terminal(op(p1, false)); }
        if (internal::is_left_irrelevant(op, p1)) { return bdd_2; }
        // if (is_left_negating(op, p1))
        return bdd_not(bdd_2);
      } else { // if (bdd_isterminal(bdd_2)) {
        const bool p2 = dd_valueof(bdd_2);

        if (internal::can_right_shortcut(op, p2)) { return bdd_terminal(op(false, p2)); }
        if (internal::is_right_irrelevant(op, p2)) { return bdd_1; }
        // if (is_right_negating(op, p2))
        return bdd_not(bdd_1);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

  public:
    static internal::cut
    left_cut(const bool_op& op)
    {
      const bool incl_false = !internal::can_left_shortcut(op, false);
      const bool incl_true  = !internal::can_left_shortcut(op, true);

      return internal::cut(incl_false, incl_true);
    }

    static internal::cut
    right_cut(const bool_op& op)
    {
      const bool incl_false = !internal::can_right_shortcut(op, false);
      const bool incl_true  = !internal::can_right_shortcut(op, true);

      return internal::cut(incl_false, incl_true);
    }

  private:
    static internal::tuple<bdd::pointer_type>
    __resolve_request(const bool_op& op, const internal::tuple<bdd::pointer_type>& r)
    {
      if (r[0].is_terminal() && internal::can_left_shortcut(op, r[0].value())) {
        return { r[0], bdd::pointer_type(true) };
      }
      if (r[1].is_terminal() && internal::can_right_shortcut(op, r[1].value())) {
        return { bdd::pointer_type(true), r[1] };
      }
      return r;
    }

  public:
    static internal::prod2_rec
    resolve_request(const bool_op& op,
                    const internal::tuple<bdd::pointer_type>& r_low,
                    const internal::tuple<bdd::pointer_type>& r_high)
    {
      return internal::prod2_rec_output{ __resolve_request(op, r_low),
                                         __resolve_request(op, r_high) };
    }

    static constexpr bool no_skip = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_apply(const exec_policy& ep, const bdd& f, const bdd& g, const bool_op& op)
  {
    return internal::prod2<apply_prod2_policy>(ep, f, g, op);
  }

  __bdd
  bdd_apply(const bdd& f, const bdd& g, const bool_op& op)
  {
    return bdd_apply(exec_policy(), f, g, op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_and(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, and_op);
  }

  __bdd
  bdd_and(const bdd& f, const bdd& g)
  {
    return bdd_and(exec_policy(), f, g);
  }

  __bdd
  bdd_nand(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, nand_op);
  }

  __bdd
  bdd_nand(const bdd& f, const bdd& g)
  {
    return bdd_nand(exec_policy(), f, g);
  }

  __bdd
  bdd_or(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, or_op);
  }

  __bdd
  bdd_or(const bdd& f, const bdd& g)
  {
    return bdd_or(exec_policy(), f, g);
  }

  __bdd
  bdd_nor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, nor_op);
  }

  __bdd
  bdd_nor(const bdd& f, const bdd& g)
  {
    return bdd_nor(exec_policy(), f, g);
  }

  __bdd
  bdd_xor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, xor_op);
  }

  __bdd
  bdd_xor(const bdd& f, const bdd& g)
  {
    return bdd_xor(exec_policy(), f, g);
  }

  __bdd
  bdd_xnor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, xnor_op);
  }

  __bdd
  bdd_xnor(const bdd& f, const bdd& g)
  {
    return bdd_xnor(exec_policy(), f, g);
  }

  __bdd
  bdd_imp(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, imp_op);
  }

  __bdd
  bdd_imp(const bdd& f, const bdd& g)
  {
    return bdd_imp(exec_policy(), f, g);
  }

  __bdd
  bdd_invimp(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, invimp_op);
  }

  __bdd
  bdd_invimp(const bdd& f, const bdd& g)
  {
    return bdd_invimp(exec_policy(), f, g);
  }

  __bdd
  bdd_equiv(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, equiv_op);
  }

  __bdd
  bdd_equiv(const bdd& f, const bdd& g)
  {
    return bdd_equiv(exec_policy(), f, g);
  }

  __bdd
  bdd_diff(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, diff_op);
  }

  __bdd
  bdd_diff(const bdd& f, const bdd& g)
  {
    return bdd_diff(exec_policy(), f, g);
  }

  __bdd
  bdd_less(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, less_op);
  }

  __bdd
  bdd_less(const bdd& f, const bdd& g)
  {
    return bdd_less(exec_policy(), f, g);
  }
};
