#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <utility>

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
    static inline zdd::ptr_t
    resolve_root(const zdd::node_t &r, const bool_op &/*op*/)
    {
      // TODO: should all but the last case not have a 'suppression taint'?

      // Return the True terminal if any (including its tainting flags).
      if (r.low().is_terminal() && r.high().is_terminal()) {
        adiar_debug(r.low().value() || r.high().value(),
                    "At least one of the terminals should be True in a ZDD");

        return r.low().value() ? r.low() : r.high();
      }

      // Will one of the two options "fall out"?
      if (r.low().is_terminal() && !r.low().value()) {
        return r.high();
      }
      // if (r.high().is_terminal() && !r.high().value()) { NEVER HAPPENS }

      // Otherwise return as-is
      return r.uid();
    }

  public:
    static inline internal::quantify_request<0>::target_t
    resolve_request(const bool_op &/* op */,
                    const internal::quantify_request<0>::target_t &target)
    {
      adiar_debug(!target[0].is_nil() && !target[1].is_nil(),
                  "Resolve request is only used for tuple cases");

      const zdd::ptr_t tgt_first = target.first();
      const zdd::ptr_t tgt_second = target.second();

      // Has the second option fallen out, while the first is still within? Then
      // we can collapse back into the original ZDD.
      if (tgt_first.is_node() && tgt_second.is_false()) {
        return { tgt_first, zdd::ptr_t::NIL() };
      }
      return target;
    }

  public:
    static inline internal::cut_type
    cut_with_terminals(const bool_op &/*op*/)
    {
      return internal::cut_type::ALL;
    }

  public:
    static constexpr bool quantify_onset = false;
  };

  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_project(const zdd &A, const std::function<bool(zdd::label_t)> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }

  __zdd zdd_project(zdd &&A, const std::function<bool(zdd::label_t)> &dom)
  {
    return internal::quantify<zdd_project_policy>(std::forward<zdd>(A), dom, or_op);
  }

  __zdd zdd_project(const zdd &A, const std::function<zdd::label_t()> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }

  __zdd zdd_project(zdd &&A, const std::function<zdd::label_t()> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }
}
