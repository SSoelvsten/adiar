#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/label.h>

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
    static __bdd resolve_terminal_root(const node_t v, const bool_op &op)
    {
      if (is_terminal(v.low()) && can_left_shortcut(op, v.low())) {
        return bdd_terminal(value_of(v.low()));
      }

      if (is_terminal(v.high()) && can_right_shortcut(op, v.high())) {
        return bdd_terminal(value_of(v.high()));
      }

      return __bdd(); // return nothing
    }

  public:
    static tuple resolve_request(const bool_op &op, ptr_t r1, ptr_t r2)
    {
      adiar_debug(!is_nil(r1) && !is_nil(r2), "Resolve request is only used for tuple cases");

      ptr_t r_fst = fst(r1,r2);
      ptr_t r_snd = snd(r1,r2);

      if (is_terminal(r_snd) && can_right_shortcut(op, r_snd)) {
        r_fst = create_terminal_ptr(false);
      }

      return { r_fst, r_snd };
    }

  public:
    static cut_type cut_with_terminals(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, create_terminal_ptr(false));
      const bool incl_true = !can_right_shortcut(op, create_terminal_ptr(true));

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
    label_t label = ls.pull();                                      \
    if (!ls.can_pull()) {                                           \
      return quantify<bdd_quantify_policy>(bdd_var, label, op);     \
    } else {                                                        \
      bdd_var = quantify<bdd_quantify_policy>(bdd_var, label, op);  \
    }                                                               \
  }                                                                 \

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, label_t label)
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

  __bdd bdd_forall(const bdd &in_bdd, label_t label)
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
