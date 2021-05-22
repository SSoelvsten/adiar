#include "apply.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>
#include <adiar/tuple.h>

#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/product_construction.h>

#include <adiar/bdd/build.h>
#include <adiar/bdd/negate.h>

#include <adiar/assert.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Apply policy for product construction.
  class apply_prod_policy
  {
  public:
    static __bdd resolve_same_file(const bdd &bdd_1, const bdd &bdd_2,
                                   const bool_op &op)
    {
      ptr_t sink_1_F = create_sink_ptr(bdd_1.negate);
      ptr_t sink_2_F = create_sink_ptr(bdd_2.negate);

      // Compute the results on all children.
      ptr_t op_F = op(sink_1_F, sink_2_F);
      ptr_t op_T = op(negate(sink_1_F), negate(sink_2_F));

      // Does it collapse to a sink?
      if (op_F == op_T) {
        return bdd_sink(value_of(op_F));
      }

      return op_F == sink_1_F ? bdd_1 : ~bdd_1;
    }

    static __bdd resolve_sink_root(const node_t &v1, const bdd& bdd_1,
                                   const node_t &v2, const bdd& bdd_2,
                                   const bool_op &op)
    {
      if (is_sink(v1) && is_sink(v2)) {
        ptr_t p = op(v1.uid, v2.uid);
        return bdd_sink(value_of(p));
      } else if (is_sink(v1)) {
        if (can_left_shortcut(op, v1.uid)) {
          ptr_t p =  op(v1.uid, create_sink_ptr(false));
          return bdd_sink(value_of(p));
        } else if (is_left_irrelevant(op, v1.uid)) {
          return bdd_2;
        } else { // if (is_left_negating(op, v1.uid)) {
          return bdd_not(bdd_2);
        }
      } else { // if (is_sink(v2)) {
        if (can_right_shortcut(op, v2.uid)) {
          ptr_t p = op(create_sink_ptr(false), v2.uid);
          return bdd_sink(value_of(p));
        } else if (is_right_irrelevant(op, v2.uid)) {
          return bdd_1;
        } else { // if (is_right_negating(op, v2.uid)) {
          return bdd_not(bdd_1);
        }
      }
      adiar_unreachable();
    }

    static void resolve_request(prod_priority_queue_1_t &prod_pq_1, arc_writer &aw,
                                ptr_t /* t1 */, ptr_t /* t2 */, const bool_op &op,
                                ptr_t source, ptr_t r1, ptr_t r2)
    {
      if (is_sink(r1) && is_sink(r2)) {
        arc_t out_arc = { source, op(r1, r2) };
        aw.unsafe_push_sink(out_arc);
      } else if (is_sink(r1) && can_left_shortcut(op, r1)) {
        arc_t out_arc = { source, op(r1, create_sink_ptr(true)) };
        aw.unsafe_push_sink(out_arc);
      } else if (is_sink(r2) && can_right_shortcut(op, r2)) {
        arc_t out_arc = { source, op(create_sink_ptr(true), r2) };
        aw.unsafe_push_sink(out_arc);
      } else {
        prod_pq_1.push({ r1, r2, source });
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op)
  {
    return product_construction<apply_prod_policy, __bdd>(bdd_1, bdd_2, op);
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_and(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, and_op);
  }

  __bdd bdd_nand(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, nand_op);
  }

  __bdd bdd_or(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, or_op);
  }

  __bdd bdd_nor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, nor_op);
  }

  __bdd bdd_xor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, xor_op);
  }

  __bdd bdd_xnor(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, xnor_op);
  }

  __bdd bdd_imp(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, imp_op);
  }

  __bdd bdd_invimp(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, invimp_op);
  }

  __bdd bdd_equiv(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, equiv_op);
  }

  __bdd bdd_diff(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, diff_op);
  }

  __bdd bdd_less(const bdd &bdd_1, const bdd &bdd_2)
  {
    return bdd_apply(bdd_1, bdd_2, less_op);
  }
}
