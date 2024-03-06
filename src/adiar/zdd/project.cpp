#include <utility>

#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar
{
  class zdd_project_policy : public zdd_policy
  {
  public:
    static inline zdd::pointer_type
    resolve_root(const zdd::node_type& r)
    {
      // TODO: should all but the last case not have a 'suppression taint'?

      // Return the True terminal if any (including its tainting flags).
      if (r.low().is_terminal() && r.high().is_terminal()) {
        adiar_assert(r.low().value() || r.high().value(),
                     "At least one of the terminals should be True in a ZDD");

        return r.low().value() ? r.low() : r.high();
      }

      // Will one of the two options "fall out"?
      if (r.low().is_terminal() && r.low().value() == false) { return r.high(); }
      // if (r.high().is_terminal() && !r.high().value()) { NEVER HAPPENS }

      // Otherwise return as-is
      return r.uid();
    }

  public:
    static inline bool
    keep_terminal(const zdd::pointer_type& p)
    {
      return p.value();
    }

    static constexpr bool
    collapse_to_terminal(const zdd::pointer_type& /*p*/)
    {
      return false;
    }

    static inline zdd::pointer_type
    resolve_terminals(const zdd::pointer_type& a, const zdd::pointer_type& b)
    {
      return or_op(a.value(), b.value());
    }

  public:
    static inline internal::cut
    cut_with_terminals()
    {
      return internal::cut::All;
    }

  public:
    static constexpr bool quantify_onset = false;
  };

  //////////////////////////////////////////////////////////////////////////////
  __zdd
  zdd_project(const exec_policy& ep, const zdd& A, const predicate<zdd::label_type>& dom)
  {
    return internal::quantify<zdd_project_policy>(ep, A, dom);
  }

  __zdd
  zdd_project(const zdd& A, const predicate<zdd::label_type>& dom)
  {
    return zdd_project(exec_policy(), A, dom);
  }

  __zdd
  zdd_project(const exec_policy& ep, zdd&& A, const predicate<zdd::label_type>& dom)
  {
    return internal::quantify<zdd_project_policy>(ep, std::move(A), dom);
  }

  __zdd
  zdd_project(zdd&& A, const predicate<zdd::label_type>& dom)
  {
    return zdd_project(exec_policy(), std::move(A), dom);
  }

  __zdd
  zdd_project(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& dom)
  {
    return internal::quantify<zdd_project_policy>(ep, A, dom);
  }

  __zdd
  zdd_project(const zdd& A, const generator<zdd::label_type>& dom)
  {
    return zdd_project(exec_policy(), A, dom);
  }

  __zdd
  zdd_project(const exec_policy& ep, zdd&& A, const generator<zdd::label_type>& dom)
  {
    return internal::quantify<zdd_project_policy>(ep, std::move(A), dom);
  }

  __zdd
  zdd_project(zdd&& A, const generator<zdd::label_type>& dom)
  {
    return zdd_project(exec_policy(), std::move(A), dom);
  }
}
