#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <utility>

#include <adiar/internal/assert.h>
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
    static inline zdd::pointer_type
    resolve_root(const zdd::node_type &r, const bool_op &/*op*/)
    {
      // TODO: should all but the last case not have a 'suppression taint'?

      // Return the True terminal if any (including its tainting flags).
      if (r.low().is_terminal() && r.high().is_terminal()) {
        adiar_assert(r.low().value() || r.high().value(),
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
    static inline bool
    keep_terminal(const bool_op &/*op*/, const zdd::pointer_type &p)
    {
      return p.value();
    }

    static constexpr bool
    collapse_to_terminal(const bool_op &/*op*/, const zdd::pointer_type &/*p*/)
    {
      return false;
    }

  public:
    static inline internal::cut
    cut_with_terminals(const bool_op &/*op*/)
    {
      return internal::cut::All;
    }

  public:
    static constexpr bool quantify_onset = false;
  };

  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_project(const zdd &A, const predicate<zdd::label_type> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }

  __zdd zdd_project(zdd &&A, const predicate<zdd::label_type> &dom)
  {
    return internal::quantify<zdd_project_policy>(std::forward<zdd>(A), dom, or_op);
  }

  __zdd zdd_project(const zdd &A, const generator<zdd::label_type> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }

  __zdd zdd_project(zdd &&A, const generator<zdd::label_type> &dom)
  {
    return internal::quantify<zdd_project_policy>(A, dom, or_op);
  }
}
