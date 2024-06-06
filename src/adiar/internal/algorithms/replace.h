#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include <type_traits>

#include <adiar/exception.h>
#include <adiar/functional.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/assert.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::replace_t stats_replace;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper Functions

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A total mapping function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using replace_func = function<typename T::label_type(typename T::label_type)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level of a single pointer with the one provided by the map `m`.
  ///
  /// \details All other information, e.g. level-identifier, terminal value, and taint flag, are
  ///          preserved as-is.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64
  __replace(const ptr_uint64& p, const replace_func<ptr_uint64>& m)
  {
    return p.is_node() ? replace(p, m(p.level())) : p;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level of a single node and its children pointers.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline node
  __replace(const node& n, const replace_func<node>& m)
  {
    return { __replace(n.uid().as_ptr(), m), __replace(n.low(), m), __replace(n.high(), m) };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  ///
  /// \param ls A stream of `level_info` structs in *descending* order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename LevelInfoStream>
  replace_type
  __replace__infer_type(LevelInfoStream& ls, const replace_func<Policy>& m)
  {
    using signed_label = typename std::make_signed<typename Policy::label_type>::type;

    bool identity = true;
    bool monotone = true;

    signed_label prev_before = -1;
    signed_label prev_after  = -1;
    while (ls.can_pull()) {
      const signed_label next_before = ls.pull().level();
      const signed_label next_after  = m(next_before);

      identity &= next_before == next_after;
      monotone &= prev_before < 0 || prev_after < next_after;

      prev_before = next_before;
      prev_after  = next_after;
    }

    if (identity) { return replace_type::Identity; }
    if (monotone) { return replace_type::Monotone; }
    return replace_type::Non_Monotone;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes in a single linear scan.
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  __replace__monotonic_scan(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    adiar_assert(!dd->is_terminal());

    // Set up outputs
    shared_levelized_file<typename Policy::node_type> out_file;
    node_writer out_writer(out_file);

    out_writer.unsafe_set_sorted(dd->sorted);
    out_writer.unsafe_set_indexable(dd->indexable);

    out_writer.unsafe_set_1level_cut(
      { dd->max_1level_cut[cut::Internal],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_True : cut::Internal_False],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_False : cut::Internal_True],
        dd->max_1level_cut[cut::All] });

    { // Copy over nodes (in "reverse" to still follow the same order on disk)
      node_stream<true> in_nodes(dd);
      while (in_nodes.can_pull()) { out_writer.unsafe_push(__replace(in_nodes.pull(), m)); }
    }
    { // Copy over levels (also in "reverse")
      level_info_stream<true> in_levels(dd);
      while (in_levels.can_pull()) {
        const level_info li = in_levels.pull();
        out_writer.unsafe_push(level_info(m(li.level()), li.width()));
      }
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class replace_reduce_policy : public Policy
  {
  private:
    const replace_func<Policy>& _m;

  public:
    replace_reduce_policy(const replace_func<Policy>& m)
      : _m(m)
    {}

    constexpr inline typename Policy::label_type
    map_level(typename Policy::label_type x) const
    {
      return this->_m(x);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  __replace__monotonic_reduce(const exec_policy& ep,
                              const typename Policy::__dd_type& __dd,
                              const replace_func<Policy>& m)
  {
    replace_reduce_policy<Policy> policy(m);
    return reduce(ep, policy, std::move(__dd));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Nested Sweeping for non-monotonic reorderings.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // "Public" interface

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  replace(const exec_policy& /*ep*/,
          const typename Policy::dd_type& dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Return if nothing needs to be remapped
    if (dd->is_terminal()) {
#ifdef ADIAR_STATS
      stats_replace.terminal_returns += 1u;
#endif
      return dd;
    }

    // Map internal nodes
    switch (m_type) {
    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

    case replace_type::Monotone:
#ifdef ADIAR_STATS
      stats_replace.monotonic_scans += 1u;
#endif
      return __replace__monotonic_scan<Policy>(dd, m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_returns += 1u;
#endif
      return dd;
    }
    adiar_unreachable();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  replace(const exec_policy& ep, const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    level_info_stream<false> ls(dd);
    const replace_type m_type = __replace__infer_type<Policy>(ls, m);

    return replace<Policy>(ep, dd, m, m_type);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, bool check_reduced = true>
  typename Policy::dd_type
  replace(const exec_policy& ep,
          typename Policy::__dd_type&& __dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Is it already reduced?
    if constexpr (check_reduced) {
      if (__dd.template has<typename Policy::shared_node_file_type>()) {
        const typename Policy::dd_type dd(
          __dd.template get<typename Policy::shared_node_file_type>(), __dd.negate);
        return replace<Policy>(ep, dd, m, m_type);
      }
    }

    // Otherwise, map while reducing
    switch (m_type) {
    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

    case replace_type::Monotone:
#ifdef ADIAR_STATS
      stats_replace.monotonic_reduces += 1u;
#endif
      return __replace__monotonic_reduce<Policy>(ep, std::move(__dd), m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_reduces += 1u;
#endif
      return typename Policy::dd_type(std::move(__dd));
    }
    adiar_unreachable();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  replace(typename Policy::__dd_type&& __dd, const replace_func<Policy>& m, replace_type m_type)
  {
    const exec_policy ep = __dd._policy;
    return replace<Policy>(ep, std::move(__dd), m, m_type);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  replace(const exec_policy& ep, typename Policy::__dd_type&& __dd, const replace_func<Policy>& m)
  {
    // Is it already reduced?
    if (__dd.template has<typename Policy::shared_node_file_type>()) {
      const typename Policy::dd_type dd(__dd.template get<typename Policy::shared_node_file_type>(),
                                        __dd.negate);
      return replace<Policy>(ep, dd, m);
    }

    level_info_stream<true> ls(__dd);
    const replace_type m_type = __replace__infer_type<Policy>(ls, m);

    return replace<Policy, false>(ep, std::move(__dd), m, m_type);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  replace(typename Policy::__dd_type&& __dd, const replace_func<Policy>& m)
  {
    const exec_policy ep = __dd._policy;
    return replace<Policy>(ep, std::move(__dd), m);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
