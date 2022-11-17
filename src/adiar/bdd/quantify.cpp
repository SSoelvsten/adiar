#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>

#include <adiar/internal/algorithms/quantify.h>

#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  class bdd_quantify_policy : public bdd_policy
  {
  public:
    static __bdd resolve_terminal_root(const node v, const bool_op &op)
    {
      if (v.low().is_terminal() && can_left_shortcut(op, v.low())) {
        return bdd_terminal(v.low().value());
      }

      if (v.high().is_terminal() && can_right_shortcut(op, v.high())) {
        return bdd_terminal(v.high().value());
      }

      return __bdd(); // return nothing
    }

  public:
    static tuple<ptr_uint64, 2, true>
    resolve_request(const bool_op &op, ptr_uint64 r1, ptr_uint64 r2)
    {
      adiar_debug(!r1.is_nil() && !r2.is_nil(), "Resolve request is only used for tuple cases");

      ptr_uint64 r_fst = fst(r1,r2);
      ptr_uint64 r_snd = snd(r1,r2);

      if (r_snd.is_terminal() && can_right_shortcut(op, r_snd)) {
        r_fst = ptr_uint64(false);
      }

      return { r_fst, r_snd };
    }

  public:
    static cut_type cut_with_terminals(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, ptr_uint64(false));
      const bool incl_true = !can_right_shortcut(op, ptr_uint64(true));

      return cut_type_with(incl_false, incl_true);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
# define multi_quantify_macro(bdd_var, labels, op)                  \
  if (labels.size() == 0) { return bdd_var; }                       \
  label_stream<> ls(labels);                                        \
  while(true) {                                                     \
    if (is_terminal(bdd_var)) { return bdd_var; }                   \
                                                                    \
    bdd::label_t label = ls.pull();                                      \
    if (!ls.can_pull()) {                                           \
      return quantify<bdd_quantify_policy>(bdd_var, label, op);     \
    } else {                                                        \
      bdd_var = quantify<bdd_quantify_policy>(bdd_var, label, op);  \
    }                                                               \
  }                                                                 \

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, bdd::label_t label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, or_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, bdd::label_t label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, and_op);
  }
}
