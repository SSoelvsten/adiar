#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/quantify.h>
#include <adiar/internal/tuple.h>

namespace adiar
{
  class bdd_quantify_policy : public bdd_policy
  {
  public:
    static __bdd resolve_sink_root(const node_t v, const bool_op &op)
    {
      if (is_sink(v.low) && can_left_shortcut(op, v.low)) {
        return bdd_sink(value_of(v.low));
      }

      if (is_sink(v.high) && can_right_shortcut(op, v.high)) {
        return bdd_sink(value_of(v.high));
      }

      return __bdd(); // return nothing
    }

  public:
    static tuple resolve_request(const bool_op &op, ptr_t r1, ptr_t r2)
    {
      adiar_debug(!is_nil(r1) && !is_nil(r2), "Resolve request is only used for tuple cases");

      ptr_t r_fst = fst(r1,r2);
      ptr_t r_snd = snd(r1,r2);

      if (is_sink(r_snd) && can_right_shortcut(op, r_snd)) {
        r_fst = create_sink_ptr(false);
      }

      return { r_fst, r_snd };
    }

  public:
    static cut_type cut_with_sinks(const bool_op &op)
    {
      const bool incl_false = !can_right_shortcut(op, create_sink_ptr(false));
      const bool incl_true = !can_right_shortcut(op, create_sink_ptr(true));

      return cut_type_with(incl_false, incl_true);
    }
  };

  class bdd_quantify_label_file
  {
    label_stream<> ls;
    bool empty;

  public:
    bdd_quantify_label_file(const label_file &lf) : ls(lf)
    {
      empty = !ls.can_pull();
    }

  public:
    bool has_next_label() {
      return empty ? false : ls.can_pull();
    }

    label_t get_next_label() {
      return ls.pull();
    }

    bool use_label(label_t /*label*/) {
      return true;
    }
  };

  class bdd_quantify_label_pred
  {
    label_stream<> ls;
    label_predicate pred;
    bool empty;

  public:
    bdd_quantify_label_pred(const bdd &bdd_var, const label_predicate &lp) : ls(varprofile(bdd_var)), pred(lp)
    {
      empty = !ls.can_pull();
    }

  public:
    bool has_next_label() {
      return empty ? false : ls.can_pull();
    }

    label_t get_next_label() {
      return ls.pull();
    }

    bool use_label(label_t label) {
      return pred(label);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, label_t label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    bdd_quantify_label_file lf(labels);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_file>(out, lf, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const label_file &labels)
  {
    bdd_quantify_label_file lf(labels);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_file>(in_bdd, lf, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const label_predicate &label_pred)
  {
    bdd out = in_bdd;
    bdd_quantify_label_pred lp(out, label_pred);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_pred>(out, lp, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const label_predicate &label_pred)
  {
    bdd_quantify_label_pred lp(in_bdd, label_pred);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_pred>(in_bdd, lp, or_op);
  }

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &in_bdd, label_t label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    bdd_quantify_label_file lf(labels);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_file>(out, lf, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const label_file &labels)
  {
    bdd_quantify_label_file lf(labels);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_file>(in_bdd, lf, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_predicate &label_pred)
  {
    bdd out = in_bdd;
    bdd_quantify_label_pred lp(out, label_pred);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_pred>(out, lp, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const label_predicate &label_pred)
  {
    bdd_quantify_label_pred lp(in_bdd, label_pred);
    return multi_quantify<bdd_quantify_policy, bdd_quantify_label_pred>(in_bdd, lp, and_op);
  }
}
