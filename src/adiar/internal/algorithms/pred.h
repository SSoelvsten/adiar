#ifndef ADIAR_INTERNAL_ALGORITHMS_PRED_H
#define ADIAR_INTERNAL_ALGORITHMS_PRED_H

#include <adiar/exec_policy.h>

#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics for equality checking
  extern statistics::equality_t stats_equality;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Compute whether two shared levelized node files (with associated
  ///        negation flags) are isomorphic.
  ///
  /// \details Checks whether the two files are isomorphic, i.e. whether there is a
  ///          structure-preserving mapping between `f0` and `f1`. This assumes, that both files are
  ///          of a unique reduced form.
  ///
  /// \param fi      The two files of nodes to compare.
  /// \param negatei Whether the nodes of fi should be read in negated form
  ///
  /// \return Whether `f0` and `f1` have isomorphic DAGs when applying the given negation flags.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  is_isomorphic(const exec_policy& ep,
                const shared_levelized_file<node>& f0,
                const shared_levelized_file<node>& f1,
                const bool negate0 = false,
                const bool negate1 = false);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Computes whether two decision diagrams are isomorphic; i.e. whether they are equivalent
  ///        (under the same deletion-rule).
  ///
  /// \details Checks whether the two files are isomorphic, i.e. whether there is a
  ///          structure-preserving mapping between `f0` and `f1`. This assumes, that both files are
  ///          of a unique reduced form.
  ///
  /// \param a   The first decision diagram.
  /// \param b   The second decision diagram.
  ///
  /// \return    Whether `a` and `b` have isomorphic DAGs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  is_isomorphic(const exec_policy& ep, const dd& a, const dd& b);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  template <uint8_t nodes_carried>
  using pred_request = request<2, nodes_carried>;

  template <size_t look_ahead, memory_mode mem_mode>
  using comparison_priority_queue_1_t =
    levelized_node_priority_queue<pred_request<0>,
                                  request_first_lt<pred_request<0>>,
                                  look_ahead,
                                  mem_mode,
                                  2u,
                                  0u>;

  using pred_request_2 = request<2, 1>;

  template <memory_mode mem_mode>
  using comparison_priority_queue_2_t =
    priority_queue<mem_mode, pred_request<1>, request_second_lt<pred_request<1>>>;

  template <typename comp_policy, typename pq_1_t, typename pq_2_t>
  bool
  __comparison_check(const shared_levelized_file<node>& f0,
                     const shared_levelized_file<node>& f1,
                     const bool negate0,
                     const bool negate1,
                     const tpie::memory_size_type pq_1_memory,
                     const tpie::memory_size_type pq_2_memory,
                     const size_t max_pq_size)
  {
    // Set up input
    node_stream<> in_nodes_0(f0, negate0);
    node_stream<> in_nodes_1(f1, negate1);

    node v0 = in_nodes_0.pull();
    node v1 = in_nodes_1.pull();

    // Edge-case for terminals
    if (v0.is_terminal() || v1.is_terminal()) {
      bool ret_value;
      if (comp_policy::resolve_terminals(v0, v1, ret_value)) { return ret_value; }
    }

    if (v0.low().is_terminal() && v0.high().is_terminal() && v1.low().is_terminal()
        && v1.high().is_terminal()) {
      return comp_policy::resolve_singletons(v0, v1);
    }

    // Set up cross-level priority queue
    pq_1_t comparison_pq_1({ f0, f1 }, pq_1_memory, max_pq_size, stats_equality.lpq);
    comparison_pq_1.push({ { v0.uid(), v1.uid() }, {} });

    // Set up per-level priority queue
    pq_2_t comparison_pq_2(pq_2_memory, max_pq_size);

    // Initialise level checking
    typename comp_policy::level_check_t level_checker(f0, f1);

    while (!comparison_pq_1.empty()) {
      // Set up next level
      comparison_pq_1.setup_next_level();

      level_checker.next_level(comparison_pq_1.current_level());

      while (!comparison_pq_1.empty_level() || !comparison_pq_2.empty()) {
        pred_request_2 req;

        // Merge requests from comparison_pq_1 and comparison_pq_2
        if (comparison_pq_1.can_pull()
            && (comparison_pq_2.empty()
                || comparison_pq_1.top().target.first() < comparison_pq_2.top().target.second())) {
          req = { comparison_pq_1.top().target,
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } } };
          comparison_pq_1.pop();
        } else {
          req = comparison_pq_2.top();
          comparison_pq_2.pop();
        }

        // Seek request partially in stream
        const typename comp_policy::pointer_type t_seek =
          req.empty_carry() ? req.target.first() : req.target.second();

        while (v0.uid() < t_seek && in_nodes_0.can_pull()) { v0 = in_nodes_0.pull(); }
        while (v1.uid() < t_seek && in_nodes_1.can_pull()) { v1 = in_nodes_1.pull(); }

        // Skip all remaining requests to the same node
        while (comparison_pq_1.can_pull() && (comparison_pq_1.top().target == req.target)) {
          comparison_pq_1.pull();
        }

        // Forward information across the level
        if (req.empty_carry() && req.target[0].is_node() && req.target[1].is_node()
            && req.target[0].label() == req.target[1].label()
            && (v0.uid() != req.target[0] || v1.uid() != req.target[1])) {
          const typename comp_policy::children_type children =
            (req.target[0] == v0.uid() ? v0 : v1).children();

          comparison_pq_2.push({ req.target, { children } });

          continue;
        }

        if (level_checker.on_step()) { return level_checker.termination_value; }

        // Obtain children or root for both nodes (depending on level)
        const tuple<typename comp_policy::children_type, 2> children =
          comp_policy::merge(req, t_seek, v0, v1);

        // Create pairing of product children and obtain new recursion targets
        const tuple<typename comp_policy::pointer_type> rec_pair_0 = { children[0][false],
                                                                       children[1][false] };

        const tuple<typename comp_policy::pointer_type> rec_pair_1 = { children[0][true],
                                                                       children[1][true] };

        // Forward pairing and return early if possible
        if (comp_policy::resolve_request(comparison_pq_1, rec_pair_0)
            || comp_policy::resolve_request(comparison_pq_1, rec_pair_1)) {
          return comp_policy::early_return_value;
        }
      }
    }

    return comp_policy::no_early_return_value;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Behaviour can be changed with the 'comp_policy'.
  ///
  /// - The 'resolve_terminals' function resolves the case of being given two terminals.
  ///
  /// - The 'resolve_request' function checks for early termination and places new recursion
  ///   requests in the priority queue if more recursions are needed.
  ///
  /// - If the constexpr 'request_capped_by_level_size' variable is set to true, then the algorithm
  ///   is guaranteed to only run in O(sort(N_1)) number of I/Os.
  ///
  /// - The constexpr 'early_return_value' and 'no_early_return_value' change the return value on
  ///   early returns.
  ///
  /// This 'comp_policy' also should inherit (or provide) the general policy for the
  /// decision_diagram used (i.e. bdd_policy in bdd/bdd.h, zdd_policy in zdd/zdd.h and so on). This
  /// provides the following functions
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename comp_policy>
  bool
  comparison_check(const exec_policy& ep,
                   const shared_levelized_file<node>& f0,
                   const shared_levelized_file<node>& f1,
                   const bool negate0,
                   const bool negate1)
  {
    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t aux_available_memory = memory_available()
      // Input
      - 2 * node_stream<>::memory_usage()
      // Level checker policy
      - comp_policy::level_check_t::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      comparison_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      comparison_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
      * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_1_memory_fits =
      comparison_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
        pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      comparison_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_1_bound = comp_policy::level_check_t::pq1_upper_bound(f0, f1);

    const size_t max_pq_1_size =
      internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = comp_policy::level_check_t::pq2_upper_bound(f0, f1);

    const size_t max_pq_2_size =
      internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    // TODO: Only one element per node in pq_2, so maximum is width (or their product)!
    if (!external_only && max_pq_1_size <= no_lookahead_bound(comp_policy::lookahead_bound())) {
#ifdef ADIAR_STATS
      stats_equality.lpq.unbucketed += 1u;
#endif
      return __comparison_check<comp_policy,
                                comparison_priority_queue_1_t<0, memory_mode::Internal>,
                                comparison_priority_queue_2_t<memory_mode::Internal>>(
        f0, f1, negate0, negate1, pq_1_internal_memory, pq_2_internal_memory, max_pq_1_size);
    } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
               && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_equality.lpq.internal += 1u;
#endif
      return __comparison_check<
        comp_policy,
        comparison_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>,
        comparison_priority_queue_2_t<memory_mode::Internal>>(
        f0, f1, negate0, negate1, pq_1_internal_memory, pq_2_internal_memory, max_pq_1_size);
    } else {
#ifdef ADIAR_STATS
      stats_equality.lpq.external += 1u;
#endif
      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __comparison_check<
        comp_policy,
        comparison_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>,
        comparison_priority_queue_2_t<memory_mode::External>>(
        f0, f1, negate0, negate1, pq_1_memory, pq_2_memory, max_pq_1_size);
    }
  }

  template <typename comp_policy>
  bool
  comparison_check(const exec_policy& ep, const dd& a, const dd& b)
  {
    return comparison_check<comp_policy>(
      ep, a.file_ptr(), b.file_ptr(), a.is_negated(), b.is_negated());
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_PRED_H
