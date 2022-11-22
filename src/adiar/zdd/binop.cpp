#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/file_stream.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  bool can_right_shortcut_zdd(const bool_op &op, const ptr_uint64 terminal)
  {
    ptr_uint64 terminal_F = ptr_uint64(false);
    ptr_uint64 terminal_T = ptr_uint64(true);

    return // Does it shortcut on this level?
         op(terminal_F, terminal) == terminal_F && op(terminal_T,  terminal) == terminal_F
      // Does it shortcut on all other levels below?
      && op(terminal_F, terminal_F) == terminal_F && op(terminal_T,  terminal_F) == terminal_F;
  }

  bool can_left_shortcut_zdd(const bool_op &op, const ptr_uint64 terminal)
  {
    ptr_uint64 terminal_F = ptr_uint64(false);
    ptr_uint64 terminal_T = ptr_uint64(true);

    return // Does it shortcut on this level?
      op(terminal, terminal_F) == terminal_F && op(terminal, terminal_T) == terminal_F
      // Does it shortcut on all other levels below?
      && op(terminal_F, terminal_F) == terminal_F && op(terminal_F,  terminal_T) == terminal_F;
  }

  bool zdd_skippable(const bool_op &op, ptr_uint64 high1, ptr_uint64 high2)
  {
    return (high1.is_terminal() && high2.is_terminal()
            && op(high1, high2) == ptr_uint64(false))
      || (high1.is_terminal() && can_left_shortcut_zdd(op, high1))
      || (high2.is_terminal() && can_right_shortcut_zdd(op, high2));
  }

  //////////////////////////////////////////////////////////////////////////////
  // ZDD product construction policy
  class zdd_prod_policy : public zdd_policy, public prod2_mixed_level_merger
  {
  public:
    static __zdd resolve_same_file(const zdd &zdd_1, const zdd &/* zdd_2 */,
                                   const bool_op &op)
    {
      // Compute the results on all children.
      ptr_uint64 op_F = op(ptr_uint64(false), ptr_uint64(false));
      ptr_uint64 op_T = op(ptr_uint64(true), ptr_uint64(true));

      // Does it collapse to a terminal?
      if (op_F == op_T) {
        return zdd_terminal(op_F.value());
      }

      return zdd_1;
    }

  public:
    static __zdd resolve_terminal_root(const node &v1, const zdd& zdd_1,
                                   const node &v2, const zdd& zdd_2,
                                   const bool_op &op)
    {
      ptr_uint64 terminal_F = ptr_uint64(false);

      if (v1.is_terminal() && v2.is_terminal()) {
        ptr_uint64 p = op(v1.uid(), v2.uid());
        return zdd_terminal(p.value());
      } else if (v1.is_terminal()) {
        if (can_left_shortcut_zdd(op, v1.uid()))  {
          // Shortcuts the left-most path to {Ø} and all others to Ø
          return zdd_terminal(false);
        } else if (is_left_irrelevant(op, v1.uid()) && is_left_irrelevant(op, terminal_F)) {
          // Has no change to left-most path to {Ø} and neither any others
          return zdd_2;
        }
      } else { // if (v2.is_terminal()) {
        if (can_right_shortcut_zdd(op, v2.uid())) {
          // Shortcuts the left-most path to {Ø} and all others to Ø
          return zdd_terminal(false);
        } else if (is_right_irrelevant(op, v2.uid()) && is_right_irrelevant(op, terminal_F)) {
          // Has no change to left-most path to {Ø} and neither any others
          return zdd_1;
        }
      }
      return __zdd(); // return with no_file
    }

  public:
    static cut_type left_cut(const bool_op &op)
    {
      const bool incl_false = !can_left_shortcut_zdd(op, ptr_uint64(false));
      const bool incl_true = !can_left_shortcut_zdd(op, ptr_uint64(true));

      return cut_type_with(incl_false, incl_true);
    }

    static cut_type right_cut(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut_zdd(op, ptr_uint64(false));
      const bool incl_true = !can_right_shortcut_zdd(op, ptr_uint64(true));

      return cut_type_with(incl_false, incl_true);
    }

  private:
    static tuple<ptr_uint64>
    __resolve_request(const bool_op &op, ptr_uint64 r1, ptr_uint64 r2)
    {
      if (r1.is_terminal() && can_left_shortcut_zdd(op, r1)) {
        return { r1, ptr_uint64(true) };
      } else if (r2.is_terminal() && can_right_shortcut_zdd(op, r2)) {
        return { ptr_uint64(true), r2 };
      } else {
        return { r1, r2 };
      }
    }

  public:
    static prod_rec resolve_request(const bool_op &op,
                                    ptr_uint64 low1, ptr_uint64 low2, ptr_uint64 high1, ptr_uint64 high2)
    {
      // Skip node, if it would be removed in the following Reduce
      if (zdd_skippable(op, high1, high2)) {
        return prod_rec_skipto { low1, low2 };
      } else {
        return prod_rec_output {
          __resolve_request(op, low1, low2),
          __resolve_request(op, high1, high2)
        };
      }
    }

    static constexpr bool no_skip = false;
  };

  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_binop(const zdd &A, const zdd &B, const bool_op &op)
  {
    return prod2<zdd_prod_policy>(A, B, op);
  }
}
