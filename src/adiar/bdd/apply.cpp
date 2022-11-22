#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>

#include <adiar/internal/algorithms/prod2.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>

#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  class apply_prod_policy : public bdd_policy, public internal::prod2_mixed_level_merger
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
    static __bdd resolve_terminal_root(const bdd::node_t &v1, const bdd& bdd_1,
                                       const bdd::node_t &v2, const bdd& bdd_2,
                                       const bool_op &op)
    {
      if (v1.is_terminal() && v2.is_terminal()) {
        bdd::ptr_t p = op(v1.uid(), v2.uid());
        return bdd_terminal(p.value());
      } else if (v1.is_terminal()) {
        if (can_left_shortcut(op, v1.uid())) {
          bdd::ptr_t p =  op(v1.uid(), bdd::ptr_t(false));
          return bdd_terminal(p.value());
        } else if (is_left_irrelevant(op, v1.uid())) {
          return bdd_2;
        } else { // if (is_left_negating(op, v1.uid())) {
          return bdd_not(bdd_2);
        }
      } else { // if (v2.is_terminal()) {
        if (can_right_shortcut(op, v2.uid())) {
          bdd::ptr_t p = op(bdd::ptr_t(false), v2.uid());
          return bdd_terminal(p.value());
        } else if (is_right_irrelevant(op, v2.uid())) {
          return bdd_1;
        } else { // if (is_right_negating(op, v2.uid())) {
          return bdd_not(bdd_1);
        }
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
    __resolve_request(const bool_op &op, bdd::ptr_t r1, bdd::ptr_t r2)
    {
      if (r1.is_terminal() && can_left_shortcut(op, r1)) {
        return { r1, bdd::ptr_t(true) };
      } else if (r2.is_terminal() && can_right_shortcut(op, r2)) {
        return { bdd::ptr_t(true), r2 };
      } else {
        return { r1, r2 };
      }
    }

  public:
    static internal::prod2_rec resolve_request(const bool_op &op,
                                               bdd::ptr_t low1,  bdd::ptr_t low2,
                                               bdd::ptr_t high1, bdd::ptr_t high2)
    {
      return internal::prod2_rec_output {
        __resolve_request(op, low1, low2),
        __resolve_request(op, high1, high2)
      };
    }

    static constexpr bool no_skip = false;
  };

  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op)
  {
    return internal::prod2<apply_prod_policy>(bdd_1, bdd_2, op);
  }
};

