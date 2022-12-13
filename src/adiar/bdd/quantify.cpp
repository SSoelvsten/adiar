#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file_writer.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar
{
  class bdd_quantify_policy : public bdd_policy
  {
  public:
    static inline __bdd
    resolve_terminal_root(const bdd::node_t v, const bool_op &op)
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
    static inline internal::quantify_request<0>::target_t
    resolve_request(const bool_op &op,
                    const internal::quantify_request<0>::target_t &target)
    {
      adiar_debug(!target[0].is_nil() && !target[1].is_nil(),
                  "Resolve request is only used for tuple cases");

      const bdd::ptr_t tgt_snd = target.snd();

      if (tgt_snd.is_terminal() && can_right_shortcut(op, tgt_snd)) {
        return { bdd::ptr_t(false), tgt_snd };
      }
      return target;
    }

  public:
    static inline internal::cut_type
    cut_with_terminals(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, bdd::ptr_t(false));
      const bool incl_true  = !can_right_shortcut(op, bdd::ptr_t(true));

      return internal::cut_type_with(incl_false, incl_true);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
# define multi_quantify_macro(bdd_var, labels, op)                           \
  if (labels->size() == 0) { return bdd_var; }                               \
  internal::file_stream<bdd::label_t> ls(labels);                            \
  while(true) {                                                              \
    if (is_terminal(bdd_var)) { return bdd_var; }                            \
                                                                             \
    bdd::label_t label = ls.pull();                                          \
    if (!ls.can_pull()) {                                                    \
      return internal::quantify<bdd_quantify_policy>(bdd_var, label, op);    \
    } else {                                                                 \
      bdd_var = internal::quantify<bdd_quantify_policy>(bdd_var, label, op); \
    }                                                                        \
  }                                                                          \

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, bdd::label_t label)
  {
    return internal::quantify<bdd_quantify_policy>(in_bdd, label, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const shared_file<bdd::label_t> &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const shared_file<bdd::label_t> &labels)
  {
    multi_quantify_macro(in_bdd, labels, or_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, bdd::label_t label)
  {
    return internal::quantify<bdd_quantify_policy>(in_bdd, label, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const shared_file<bdd::label_t> &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const shared_file<bdd::label_t> &labels)
  {
    multi_quantify_macro(in_bdd, labels, and_op);
  }
}
