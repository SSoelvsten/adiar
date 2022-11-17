#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>

#include <adiar/internal/algorithms/product_construction.h>

#include <adiar/internal/data_structures/levelized_priority_queue.h>

#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  class apply_prod_policy : public bdd_policy, public prod_mixed_level_merger
  {
  public:
    static __bdd resolve_same_file(const bdd &bdd_1, const bdd &bdd_2,
                                   const bool_op &op)
    {
      ptr_uint64 terminal_1_F = ptr_uint64(bdd_1.negate);
      ptr_uint64 terminal_2_F = ptr_uint64(bdd_2.negate);

      // Compute the results on all children.
      ptr_uint64 op_F = op(terminal_1_F, terminal_2_F);
      ptr_uint64 op_T = op(negate(terminal_1_F), negate(terminal_2_F));

      // Does it collapse to a terminal?
      if (op_F == op_T) {
        return bdd_terminal(op_F.value());
      }

      return op_F == terminal_1_F ? bdd_1 : ~bdd_1;
    }

  public:
    static __bdd resolve_terminal_root(const node &v1, const bdd& bdd_1,
                                   const node &v2, const bdd& bdd_2,
                                   const bool_op &op)
    {
      if (v1.is_terminal() && v2.is_terminal()) {
        ptr_uint64 p = op(v1.uid(), v2.uid());
        return bdd_terminal(p.value());
      } else if (v1.is_terminal()) {
        if (can_left_shortcut(op, v1.uid())) {
          ptr_uint64 p =  op(v1.uid(), ptr_uint64(false));
          return bdd_terminal(p.value());
        } else if (is_left_irrelevant(op, v1.uid())) {
          return bdd_2;
        } else { // if (is_left_negating(op, v1.uid())) {
          return bdd_not(bdd_2);
        }
      } else { // if (v2.is_terminal()) {
        if (can_right_shortcut(op, v2.uid())) {
          ptr_uint64 p = op(ptr_uint64(false), v2.uid());
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
    static cut_type left_cut(const bool_op &op)
    {
      const bool incl_false = !can_left_shortcut(op, ptr_uint64(false));
      const bool incl_true = !can_left_shortcut(op, ptr_uint64(true));

      return cut_type_with(incl_false, incl_true);
    }

    static cut_type right_cut(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, ptr_uint64(false));
      const bool incl_true = !can_right_shortcut(op, ptr_uint64(true));

      return cut_type_with(incl_false, incl_true);
    }

  private:
    static tuple<ptr_uint64>
    __resolve_request(const bool_op &op, ptr_uint64 r1, ptr_uint64 r2)
    {
      if (r1.is_terminal() && can_left_shortcut(op, r1)) {
        return { r1, ptr_uint64(true) };
      } else if (r2.is_terminal() && can_right_shortcut(op, r2)) {
        return { ptr_uint64(true), r2 };
      } else {
        return { r1, r2 };
      }
    }

  public:
    static prod_rec resolve_request(const bool_op &op,
                                    ptr_uint64 low1, ptr_uint64 low2,
                                    ptr_uint64 high1, ptr_uint64 high2)
    {
      return prod_rec_output {
        __resolve_request(op, low1, low2),
        __resolve_request(op, high1, high2)
      };
    }

    static constexpr bool no_skip = false;
  };

  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op)
  {
    return product_construction<apply_prod_policy>(bdd_1, bdd_2, op);
  }
};

