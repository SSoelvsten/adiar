#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/replace.h>

namespace adiar
{
  bdd
  bdd_relprod(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const predicate<bdd::label_type>& pred)
  {
    return bdd_exists(ep, bdd_and(ep, states, relation), pred);
  }

  bdd
  bdd_relprod(const bdd& states, const bdd& relation, const predicate<bdd::label_type>& m)
  {
    return bdd_relprod(exec_policy(), states, relation, m);
  }

  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    __bdd tmp_1 = bdd_and(ep, states, relation);

    const bdd tmp_2 =
      bdd_exists(ep, std::move(tmp_1), [&m](bdd::label_type x) { return !m(x).has_value(); });

    const bdd tmp_3 = bdd_replace(
      ep, std::move(tmp_2), [&m](bdd::label_type x) { return m(x).value(); }, m_type);

    return tmp_3;
  }

  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    return bdd_relnext(exec_policy(), states, relation, m, m_type);
  }

  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    replace_type m_type;
    {
      internal::level_info_stream<false> ls(relation);
      m_type = internal::__replace__infer_type<bdd_policy>(ls, m);
    }
    return bdd_relnext(ep, states, relation, m, m_type);
  }

  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    return bdd_relnext(exec_policy(), states, relation, m);
  }

  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    const bdd tmp_1 = bdd_replace(
      ep, states, [&m](bdd::label_type x) { return m(x).value(); }, m_type);

    __bdd tmp_2 = bdd_and(ep, std::move(tmp_1), relation);

    const bdd tmp_3 =
      bdd_exists(ep, std::move(tmp_2), [&m](bdd::label_type x) { return !m(x).has_value(); });

    return tmp_3;
  }

  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    return bdd_relprev(exec_policy(), states, relation, m, m_type);
  }

  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    replace_type m_type;
    {
      internal::level_info_stream<false> ls(states);
      m_type = internal::__replace__infer_type<bdd_policy>(ls, m);
    }

    return bdd_relprev(ep, states, relation, m, m_type);
  }

  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    return bdd_relprev(exec_policy(), states, relation, m);
  }
}
