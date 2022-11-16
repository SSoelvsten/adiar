#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/label.h>

#include <adiar/file_stream.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>

namespace adiar
{
  class zdd_project_policy : public zdd_policy
  {
  public:
    static inline __zdd resolve_terminal_root(const node v, const bool_op &/* op */)
    {
      if (v.low().is_terminal() && v.high().is_terminal()) {
        // Only or_op and at least one of the terminals should be true
        return zdd_null();
      }

      return __zdd(); // return nothing
    }

  public:
    static inline tuple resolve_request(const bool_op &/* op */, ptr_uint64 r1, ptr_uint64 r2)
    {
      adiar_debug(!r1.is_nil() && !r2.is_nil(), "Resolve request is only used for tuple cases");

      const ptr_uint64 r_fst = fst(r1,r2);
      ptr_uint64 r_snd = snd(r1,r2);

      // Has the second option fallen out, while the first is still within? Then
      // we can collapse back into the original ZDD.
      if (r_fst.is_node() && r_snd.is_false()) {
        r_snd = ptr_uint64::NIL();
      }

      return { r_fst, r_snd };
    }

  public:
    static cut_type cut_with_terminals(const bool_op &/*op*/)
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
    level_info_stream<node> dd_meta(dd);

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

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Empty domain. Check whether Ø is in dd, i.e. the all-false path is
  // set to true.

# define multi_project_macro(zdd_var, dom)                            \
  if (is_terminal(zdd_var)) { return zdd_var; }                       \
                                                                      \
  if (dom.size() == 0) { return zdd_null(); }                         \
                                                                      \
  label_file dom_inv = extract_non_dom(zdd_var, dom);                 \
                                                                      \
  if (dom_inv.size() == zdd_varcount(zdd_var)) { return zdd_null(); } \
                                                                      \
  label_stream<> ls(dom_inv);                                         \
  while (ls.can_pull()) {                                             \
    if (is_terminal(zdd_var)) { return zdd_var; };                    \
                                                                      \
    zdd_var = quantify<zdd_project_policy>(dd, ls.pull(), or_op);     \
  }                                                                   \
  return zdd_var;                                                     \

  zdd zdd_project(const zdd &dd, const label_file &dom)
  {
    zdd temp = dd;
    multi_project_macro(temp, dom);
  }

  zdd zdd_project(zdd &&dd, const label_file &dom)
  {
    multi_project_macro(dd, dom);
  }
}
