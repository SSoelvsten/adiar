#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar
{
  class zdd_project_policy : public zdd_policy
  {
  public:
    static inline __zdd
    resolve_terminal_root(const zdd::node_t v, const bool_op &/* op */)
    {
      if (v.low().is_terminal() && v.high().is_terminal()) {
        //adiar_debug(op == or_op, "Projection is only designed for the 'or_op'");
        adiar_debug(v.low().value() || v.high().value(),
                    "At least one of the terminals should be True in a ZDD");
        return zdd_null();
      }

      return __zdd(); // return nothing
    }

  public:
    static inline internal::quantify_request<0>::target_t
    resolve_request(const bool_op &/* op */,
                    const internal::quantify_request<0>::target_t &target)
    {
      adiar_debug(!target[0].is_nil() && !target[1].is_nil(),
                  "Resolve request is only used for tuple cases");

      const zdd::ptr_t tgt_fst = target.fst();
      const zdd::ptr_t tgt_snd = target.snd();

      // Has the second option fallen out, while the first is still within? Then
      // we can collapse back into the original ZDD.
      if (tgt_fst.is_node() && tgt_snd.is_false()) {
        return { tgt_fst, zdd::ptr_t::NIL() };
      }
      return target;
    }

  public:
    static inline internal::cut_type
    cut_with_terminals(const bool_op &/*op*/)
    {
      return internal::cut_type::ALL;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  shared_file<zdd::label_t>
  extract_non_dom(const zdd &dd, const shared_file<zdd::label_t> &dom)
  {
    shared_file<zdd::label_t> dom_inv;
    internal::label_writer diw(dom_inv);

    internal::file_stream<zdd::label_t> ls(dom);
    internal::level_info_stream<> dd_meta(dd);

    while (dd_meta.can_pull()) {
      zdd::label_t dd_label = dd_meta.pull().label();
      bool found_dd_label = false;

      while (ls.can_pull()) {
        zdd::label_t dom_label = ls.pull();

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

# define multi_project_macro(zdd_var, dom)                                  \
  if (is_terminal(zdd_var)) { return zdd_var; }                             \
                                                                            \
  if (dom->size() == 0) { return zdd_null(); }                               \
                                                                            \
  shared_file<zdd::label_t> dom_inv = extract_non_dom(zdd_var, dom);   \
                                                                            \
  if (dom_inv->size() == zdd_varcount(zdd_var)) { return zdd_null(); }       \
                                                                            \
  internal::file_stream<zdd::label_t> ls(dom_inv);                                     \
  while (ls.can_pull()) {                                                   \
    if (is_terminal(zdd_var)) { return zdd_var; };                          \
                                                                            \
    zdd_var = internal::quantify<zdd_project_policy>(dd, ls.pull(), or_op); \
  }                                                                         \
  return zdd_var;                                                           \

  zdd zdd_project(const zdd &dd, const shared_file<zdd::label_t> &dom)
  {
    zdd temp = dd;
    multi_project_macro(temp, dom);
  }

  zdd zdd_project(zdd &&dd, const shared_file<zdd::label_t> &dom)
  {
    multi_project_macro(dd, dom);
  }
}
