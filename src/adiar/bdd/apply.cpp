#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  class apply_prod2_policy
    : public bdd_policy
    , public internal::prod2_mixed_level_merger<bdd_policy>
  {
  public:
    static __bdd resolve_same_file(const bdd &bdd_1, const bdd &bdd_2,
                                   const bool_op &op)
    {
      bdd::ptr_t terminal_1_F = bdd::ptr_t(bdd_1.negate);
      bdd::ptr_t terminal_2_F = bdd::ptr_t(bdd_2.negate);

      // Compute the results on all children.
      bdd::ptr_t op_F = op(terminal_1_F, terminal_2_F);
      bdd::ptr_t op_T = op(negate(terminal_1_F), negate(terminal_2_F));

      // Does it collapse to a terminal?
      if (op_F == op_T) {
        return bdd_terminal(op_F.value());
      }

      return op_F == terminal_1_F ? bdd_1 : ~bdd_1;
    }

  public:
    static __bdd resolve_terminal_root(const bdd& bdd_1,
                                       const bdd& bdd_2,
                                       const bool_op &op)
    {
      adiar_precondition(is_terminal(bdd_1) || is_terminal(bdd_2));

      if (is_terminal(bdd_1) && is_terminal(bdd_2)) {
        const bdd::ptr_t p1 = bdd::ptr_t(value_of(bdd_1));
        const bdd::ptr_t p2 = bdd::ptr_t(value_of(bdd_2));

        return bdd_terminal(op(p1, p2).value());
      } else if (is_terminal(bdd_1)) {
        const bdd::ptr_t p1 = bdd::ptr_t(value_of(bdd_1));

        if (can_left_shortcut(op, p1)) {
          return bdd_terminal(op(p1, bdd::ptr_t(false)).value());
        }
        if (is_left_irrelevant(op, p1)) {
          return bdd_2;
        }
        // if (is_left_negating(op, p1))
        return bdd_not(bdd_2);
      } else { // if (is_terminal(bdd_2)) {
        const bdd::ptr_t p2 = bdd::ptr_t(value_of(bdd_2));

        if (can_right_shortcut(op, p2)) {
          return bdd_terminal(op(bdd::ptr_t(false), p2).value());
        }
        if (is_right_irrelevant(op, p2)) {
          return bdd_1;
        }
        // if (is_right_negating(op, p2))
        return bdd_not(bdd_1);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

  public:
    static internal::cut_type left_cut(const bool_op &op)
    {
      const bool incl_false = !can_left_shortcut(op, bdd::ptr_t(false));
      const bool incl_true = !can_left_shortcut(op, bdd::ptr_t(true));

      return internal::cut_type_with(incl_false, incl_true);
    }

    static internal::cut_type right_cut(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, bdd::ptr_t(false));
      const bool incl_true = !can_right_shortcut(op, bdd::ptr_t(true));

      return internal::cut_type_with(incl_false, incl_true);
    }

  private:
    static internal::tuple<bdd::ptr_t>
    __resolve_request(const bool_op &op,
                      const internal::tuple<bdd::ptr_t> &r)
    {
      if (r[0].is_terminal() && can_left_shortcut(op, r[0])) {
        return { r[0], bdd::ptr_t(true) };
      }
      if (r[1].is_terminal() && can_right_shortcut(op, r[1])) {
        return { bdd::ptr_t(true), r[1] };
      }
      return r;
    }

  public:
    static internal::prod2_rec
    resolve_request(const bool_op &op,
                    const internal::tuple<bdd::ptr_t> &r_low,
                    const internal::tuple<bdd::ptr_t> &r_high)
    {
      return internal::prod2_rec_output {
        __resolve_request(op, r_low),
        __resolve_request(op, r_high)
      };
    }

    static constexpr bool no_skip = false;
  };

  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op)
  {
    return internal::prod2<apply_prod2_policy>(bdd_1, bdd_2, op);
  }
};
