#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/data.h>

#include <adiar/file_stream.h>

#include <adiar/internal/quantify.h>

namespace adiar
{
  class zdd_project_policy : public zdd_policy
  {
  public:
    static inline __zdd resolve_sink_root(const node_t v, const bool_op &/* op */)
    {
      if (is_sink(v.low) && is_sink(v.high)) {
        // Only or_op and at least one of the sinks should be true
        return zdd_null();
      }

      return __zdd(); // return nothing
    }

  public:
    static inline tuple resolve_request(const bool_op &/* op */, ptr_t r1, ptr_t r2)
    {
      adiar_debug(!is_nil(r1) && !is_nil(r2), "Resolve request is only used for tuple cases");

      const ptr_t r_fst = fst(r1,r2);
      ptr_t r_snd = snd(r1,r2);

      // Has the second option fallen out, while the first is still within? Then
      // we can collapse back into the original ZDD.
      if (is_node(r_fst) && is_false(r_snd)) {
        r_snd = NIL;
      }

      return { r_fst, r_snd };
    }

  public:
    static cut_type cut_with_sinks(const bool_op &/*op*/)
    {
      return cut_type::ALL;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  label_file extract_non_dom(const zdd &dd, const label_file &dom)
  {
    label_file dom_inv;
    label_writer diw(dom_inv);

    label_stream<> ls(dom);
    level_info_stream<node_t> dd_meta(dd);

    while (dd_meta.can_pull()) {
      label_t dd_label = label_of(dd_meta.pull());
      bool found_dd_label = false;

      while (ls.can_pull()) {
        label_t dom_label = ls.pull();

        if (dd_label == dom_label) {
          found_dd_label = true;
          break;
        }
      }
      if (!found_dd_label) { diw << dd_label; }
      ls.reset();
    }

    return dom_inv;
  }

  class zdd_project_label_file
  {
    label_stream<> ls;
    bool empty;

  public:
    zdd_project_label_file(const zdd &zdd_var, const label_file &dom) : ls(extract_non_dom(zdd_var, dom))
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

  class zdd_project_label_pred
  {
    label_stream<> ls;
    label_predicate pred;
    bool empty;

  public:
    zdd_project_label_pred(const zdd &zdd_var, const label_predicate &lp) : ls(varprofile(zdd_var)), pred(lp)
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
      return !pred(label);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Empty domain. Check whether Ø is in dd, i.e. the all-false path is
  // set to true.

  zdd zdd_project(const zdd &dd, const label_file &dom)
  {
    zdd temp = dd;
    zdd_project_label_file lf(temp, dom);
    return multi_quantify<zdd_project_policy, zdd_project_label_file>(temp, lf, or_op);
  }

  zdd zdd_project(zdd &&dd, const label_file &dom)
  {
    zdd_project_label_file lf(dd, dom);
    return multi_quantify<zdd_project_policy, zdd_project_label_file>(dd, lf, or_op);
  }

  zdd zdd_project(const zdd &dd, const label_predicate &label_pred)
  {
    zdd temp = dd;
    zdd_project_label_pred lf(temp, label_pred);
    return multi_quantify<zdd_project_policy, zdd_project_label_pred>(temp, lf, or_op);
  }

  zdd zdd_project(zdd &&dd, const label_predicate &label_pred)
  {
    zdd_project_label_pred lf(dd, label_pred);
    return multi_quantify<zdd_project_policy, zdd_project_label_pred>(dd, lf, or_op);
  }
}
